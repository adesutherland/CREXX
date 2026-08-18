/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXVMBYTEENDPOINT_H
#define CREXX_RXVMBYTEENDPOINT_H

#include "rxvmchannel.h"

#include <stddef.h>
#include <stdatomic.h>
#include <stdint.h>

typedef struct rxvm_byte_endpoint rxvm_byte_endpoint;

enum {
    RXVM_BYTE_ENDPOINT_READ = 1,
    RXVM_BYTE_ENDPOINT_WRITE = 2,
    RXVM_BYTE_ENDPOINT_DUPLEX = 3
};

rxvm_byte_endpoint *rxvm_byte_endpoint_create(
        int direction,
        size_t capacity,
        const void *initial_bytes,
        size_t initial_length,
        int null_endpoint);

void rxvm_byte_endpoint_retain(rxvm_byte_endpoint *endpoint);
void rxvm_byte_endpoint_release(rxvm_byte_endpoint *endpoint);

rxvm_channel_status rxvm_byte_endpoint_read(
        rxvm_byte_endpoint *endpoint,
        void *bytes,
        size_t maximum_bytes,
        int64_t wait_microseconds,
        const atomic_uchar *operation_cancelled,
        size_t *length_out,
        int *eof_out);

rxvm_channel_status rxvm_byte_endpoint_write(
        rxvm_byte_endpoint *endpoint,
        const void *bytes,
        size_t length,
        int64_t wait_microseconds,
        const atomic_uchar *operation_cancelled,
        size_t *accepted_out);

rxvm_channel_status rxvm_byte_endpoint_half_close(
        rxvm_byte_endpoint *endpoint,
        int direction);

void rxvm_byte_endpoint_cancel(rxvm_byte_endpoint *endpoint);
void rxvm_byte_endpoint_wake(rxvm_byte_endpoint *endpoint);
int rxvm_byte_endpoint_direction(rxvm_byte_endpoint *endpoint);
size_t rxvm_byte_endpoint_capacity(rxvm_byte_endpoint *endpoint);
size_t rxvm_byte_endpoint_buffered(rxvm_byte_endpoint *endpoint);

#endif
