/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmbyteendpoint.h"
#include "rxvmintp.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
typedef HANDLE endpoint_test_thread;
#define ENDPOINT_TEST_RETURN DWORD WINAPI
#else
#include <pthread.h>
typedef pthread_t endpoint_test_thread;
#define ENDPOINT_TEST_RETURN void *
#endif

typedef struct blocked_read {
    rxvm_byte_endpoint *endpoint;
    rxvm_channel_status status;
} blocked_read;

static int failures;

static void check(int condition, const char *message) {
    if (condition) return;
    fprintf(stderr, "FAIL: %s\n", message);
    failures++;
}

static ENDPOINT_TEST_RETURN blocked_reader(void *opaque) {
    blocked_read *state = (blocked_read *)opaque;
    unsigned char byte = 0u;
    size_t length = 0u;
    int eof = 0;
    state->status = rxvm_byte_endpoint_read(
            state->endpoint, &byte, 1u, -1, 0, &length, &eof);
#if defined(_WIN32)
    return 0u;
#else
    return 0;
#endif
}

static int start_thread(endpoint_test_thread *thread, blocked_read *state) {
#if defined(_WIN32)
    *thread = CreateThread(0, 0, blocked_reader, state, 0, 0);
    return *thread != 0;
#else
    return pthread_create(thread, 0, blocked_reader, state) == 0;
#endif
}

static void join_thread(endpoint_test_thread thread) {
#if defined(_WIN32)
    check(WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0,
          "blocked reader thread joins");
    CloseHandle(thread);
#else
    check(pthread_join(thread, 0) == 0, "blocked reader thread joins");
#endif
}

int main(void) {
    rxvm_byte_endpoint *endpoint;
    rxvm_byte_endpoint *null_endpoint;
    REDIRECT *redirect;
    unsigned char buffer[16];
    size_t amount;
    int eof;
    blocked_read blocked;
    endpoint_test_thread thread;
    int index;

    check(!rxvm_byte_endpoint_create(0, 4u, 0, 0u, 0),
          "zero direction rejected");
    check(!rxvm_byte_endpoint_create(
                  RXVM_BYTE_ENDPOINT_DUPLEX, 2u, "abc", 3u, 0),
          "oversized initial snapshot rejected");

    endpoint = rxvm_byte_endpoint_create(
            RXVM_BYTE_ENDPOINT_READ, 8u, "abc", 3u, 0);
    check(endpoint != 0, "initial snapshot endpoint created");
    amount = 0u;
    eof = 0;
    check(rxvm_byte_endpoint_read(endpoint, buffer, sizeof(buffer), 0, 0,
                                  &amount, &eof) == RXVM_CHANNEL_OK &&
          amount == 3u && !memcmp(buffer, "abc", 3u) && !eof,
          "initial snapshot read exactly");
    amount = 9u;
    eof = 0;
    check(rxvm_byte_endpoint_read(endpoint, buffer, sizeof(buffer), 0, 0,
                                  &amount, &eof) == RXVM_CHANNEL_OK &&
          amount == 0u && eof,
          "initial snapshot reaches EOF");
    rxvm_byte_endpoint_release(endpoint);

    endpoint = rxvm_byte_endpoint_create(
            RXVM_BYTE_ENDPOINT_DUPLEX, 8u, 0, 0u, 0);
    redirect = rxspawn_redirect_to_byte_endpoint(endpoint, 0);
    check(redirect != 0, "standalone redirect attaches to byte endpoint");
    check(rxspawn_redirect_write_close(redirect, "reuse", 5u) == 0,
          "standalone redirect streams without spawning a process");
    amount = 0u;
    eof = 0;
    check(rxvm_byte_endpoint_read(endpoint, buffer, sizeof(buffer), 0, 0,
                                  &amount, &eof) == RXVM_CHANNEL_OK &&
          amount == 5u && !memcmp(buffer, "reuse", 5u),
          "standalone redirect preserves exact bytes");
    amount = 1u;
    check(rxvm_byte_endpoint_read(endpoint, buffer, sizeof(buffer), 0, 0,
                                  &amount, &eof) == RXVM_CHANNEL_OK &&
          amount == 0u && eof,
          "standalone redirect publishes EOF");
    check(rxspawn_redirect_byte_endpoint_destroy(redirect) == 0,
          "standalone redirect releases after joined transfer");
    rxvm_byte_endpoint_release(endpoint);

    endpoint = rxvm_byte_endpoint_create(
            RXVM_BYTE_ENDPOINT_DUPLEX, 4u, 0, 0u, 0);
    check(endpoint != 0, "duplex endpoint created");
    amount = 0u;
    check(rxvm_byte_endpoint_write(endpoint, "abcd", 4u, 0, 0, &amount) ==
                  RXVM_CHANNEL_OK && amount == 4u,
          "bounded buffer fills");
    amount = 99u;
    check(rxvm_byte_endpoint_write(endpoint, "e", 1u, 0, 0, &amount) ==
                  RXVM_CHANNEL_WOULD_BLOCK && amount == 0u,
          "full buffer reports backpressure");
    amount = 0u;
    eof = 0;
    check(rxvm_byte_endpoint_read(endpoint, buffer, 2u, 0, 0, &amount, &eof) ==
                  RXVM_CHANNEL_OK && amount == 2u &&
          !memcmp(buffer, "ab", 2u),
          "partial read frees bounded capacity");
    amount = 0u;
    check(rxvm_byte_endpoint_write(endpoint, "ef", 2u, 0, 0, &amount) ==
                  RXVM_CHANNEL_OK && amount == 2u,
          "ring buffer wraps");
    amount = 0u;
    check(rxvm_byte_endpoint_read(endpoint, buffer, 4u, 0, 0, &amount, &eof) ==
                  RXVM_CHANNEL_OK && amount == 4u &&
          !memcmp(buffer, "cdef", 4u),
          "wrapped bytes preserve order");
    check(rxvm_byte_endpoint_half_close(
                  endpoint, RXVM_BYTE_ENDPOINT_WRITE) == RXVM_CHANNEL_OK,
          "write half closes");
    amount = 3u;
    eof = 0;
    check(rxvm_byte_endpoint_read(endpoint, buffer, sizeof(buffer), 0, 0,
                                  &amount, &eof) == RXVM_CHANNEL_OK &&
          amount == 0u && eof,
          "drained half-close reports EOF");
    rxvm_byte_endpoint_release(endpoint);

    null_endpoint = rxvm_byte_endpoint_create(
            RXVM_BYTE_ENDPOINT_DUPLEX, 0u, 0, 0u, 1);
    check(null_endpoint != 0, "null endpoint created");
    amount = 0u;
    check(rxvm_byte_endpoint_write(null_endpoint, "discard", 7u, 0, 0,
                                   &amount) == RXVM_CHANNEL_OK && amount == 7u,
          "null endpoint accepts bounded write logically");
    amount = 1u;
    eof = 0;
    check(rxvm_byte_endpoint_read(null_endpoint, buffer, sizeof(buffer), 0, 0,
                                  &amount, &eof) == RXVM_CHANNEL_OK &&
          amount == 0u && eof,
          "null endpoint reads EOF");
    rxvm_byte_endpoint_release(null_endpoint);

    endpoint = rxvm_byte_endpoint_create(
            RXVM_BYTE_ENDPOINT_DUPLEX, 4u, 0, 0u, 0);
    blocked.endpoint = endpoint;
    blocked.status = RXVM_CHANNEL_INTERNAL_ERROR;
    check(start_thread(&thread, &blocked), "blocked reader starts");
    rxvm_byte_endpoint_cancel(endpoint);
    join_thread(thread);
    check(blocked.status == RXVM_CHANNEL_CLOSED,
          "cancellation wakes blocked reader");
    rxvm_byte_endpoint_release(endpoint);

    for (index = 0; index < 1000; index++) {
        endpoint = rxvm_byte_endpoint_create(
                RXVM_BYTE_ENDPOINT_DUPLEX, 8u, 0, 0u, 0);
        check(endpoint != 0, "repeated endpoint allocation");
        if (!endpoint) break;
        rxvm_byte_endpoint_retain(endpoint);
        rxvm_byte_endpoint_release(endpoint);
        rxvm_byte_endpoint_release(endpoint);
    }

    if (failures) {
        fprintf(stderr, "Gate F byte endpoint failures: %d\n", failures);
        return 1;
    }
    puts("PASS: Gate F bounded byte endpoint substrate");
    return 0;
}
