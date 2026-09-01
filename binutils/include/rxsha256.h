/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXSHA256_H
#define CREXX_RXSHA256_H

#include <stddef.h>
#include <stdint.h>

#define RX_SHA256_DIGEST_SIZE 32u
#define RX_SHA256_BLOCK_SIZE 64u
#define RX_SHA256_SERIALIZED_STATE_SIZE 152u
#define RX_SHA256_FILE_CHUNK_SIZE 32768u

typedef struct rx_sha256_context {
    uint32_t h[8];
    uint64_t bytes;
    unsigned char block[RX_SHA256_BLOCK_SIZE];
    size_t used;
} rx_sha256_context;

typedef enum rx_sha256_state_status {
    RX_SHA256_STATE_OK = 0,
    RX_SHA256_STATE_INVALID_LENGTH,
    RX_SHA256_STATE_INVALID_MAGIC,
    RX_SHA256_STATE_UNSUPPORTED_VERSION,
    RX_SHA256_STATE_INVALID_FORMAT,
    RX_SHA256_STATE_INCONSISTENT,
    RX_SHA256_STATE_INTEGRITY_FAILURE
} rx_sha256_state_status;

typedef size_t (*rx_sha256_stream_read_fn)(void *context,
                                           unsigned char *buffer,
                                           size_t capacity);
typedef int (*rx_sha256_stream_error_fn)(void *context);
typedef int (*rx_sha256_stream_close_fn)(void *context);

typedef struct rx_sha256_stream {
    void *context;
    rx_sha256_stream_read_fn read;
    rx_sha256_stream_error_fn error;
    rx_sha256_stream_close_fn close;
} rx_sha256_stream;

typedef enum rx_sha256_file_status {
    RX_SHA256_FILE_OK = 0,
    RX_SHA256_FILE_OPEN_FAILURE,
    RX_SHA256_FILE_READ_FAILURE,
    RX_SHA256_FILE_CLOSE_FAILURE,
    RX_SHA256_FILE_LENGTH_OVERFLOW
} rx_sha256_file_status;

void rx_sha256_init(rx_sha256_context *context);
int rx_sha256_update(rx_sha256_context *context,
                     const void *data, size_t length);
void rx_sha256_final(const rx_sha256_context *context,
                     unsigned char digest[RX_SHA256_DIGEST_SIZE]);

rx_sha256_state_status rx_sha256_export_state(
        const rx_sha256_context *context,
        unsigned char state[RX_SHA256_SERIALIZED_STATE_SIZE]);
rx_sha256_state_status rx_sha256_import_state(
        rx_sha256_context *context, const void *state, size_t length);

rx_sha256_file_status rx_sha256_stream_digest(
        rx_sha256_stream *stream,
        unsigned char digest[RX_SHA256_DIGEST_SIZE]);
rx_sha256_file_status rx_sha256_file(
        const char *path, unsigned char digest[RX_SHA256_DIGEST_SIZE]);

void rx_sha256(const void *data, size_t length,
               unsigned char digest[RX_SHA256_DIGEST_SIZE]);

#endif
