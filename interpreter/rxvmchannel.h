/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXVMCHANNEL_H
#define CREXX_RXVMCHANNEL_H

#include <stddef.h>
#include <stdint.h>

struct rxvm_context;

typedef enum rxvm_channel_status {
    RXVM_CHANNEL_OK = 0,
    RXVM_CHANNEL_INVALID_ARGUMENT = 1,
    RXVM_CHANNEL_INVALID_VALUE_TYPE = 2,
    RXVM_CHANNEL_INVALID_PROVIDER = 3,
    RXVM_CHANNEL_PROVIDER_UNAVAILABLE = 4,
    RXVM_CHANNEL_UNSUPPORTED_CAPABILITY = 5,
    RXVM_CHANNEL_INVALID_CONFIGURATION = 6,
    RXVM_CHANNEL_INCOMPATIBLE_VERSION = 7,
    RXVM_CHANNEL_RESOURCE_EXHAUSTED = 8,
    RXVM_CHANNEL_BACKPRESSURE = 9,
    RXVM_CHANNEL_WOULD_BLOCK = 10,
    RXVM_CHANNEL_TIMEOUT = 11,
    RXVM_CHANNEL_CLOSED = 12,
    RXVM_CHANNEL_STALE_CAPABILITY = 13,
    RXVM_CHANNEL_WRONG_OWNER = 14,
    RXVM_CHANNEL_UNKNOWN_TICKET = 15,
    RXVM_CHANNEL_ALREADY_TERMINAL = 16,
    RXVM_CHANNEL_PROVIDER_FAILURE = 17,
    RXVM_CHANNEL_SHUTTING_DOWN = 18,
    RXVM_CHANNEL_UNSUPPORTED_OPERATION = 19,
    RXVM_CHANNEL_INTERNAL_ERROR = 20
} rxvm_channel_status;

typedef struct rxvm_channel_binary {
    unsigned char *data;
    size_t length;
    size_t capacity;
} rxvm_channel_binary;

rxvm_channel_status rxvm_channel_open(
        struct rxvm_context *context,
        int64_t provider_type,
        int64_t required_capabilities,
        const void *configuration,
        size_t configuration_length,
        int64_t *channel_out);

rxvm_channel_status rxvm_channel_start(
        struct rxvm_context *context,
        int64_t channel,
        const void *envelope,
        size_t envelope_length,
        int64_t wait_microseconds,
        int64_t *ticket_out);

rxvm_channel_status rxvm_channel_wait(
        struct rxvm_context *context,
        int64_t channel,
        int64_t wait_microseconds,
        rxvm_channel_binary *completion_out);

rxvm_channel_status rxvm_channel_cancel(
        struct rxvm_context *context,
        int64_t channel,
        int64_t ticket,
        const void *reason,
        size_t reason_length);

rxvm_channel_status rxvm_channel_close(
        struct rxvm_context *context,
        int64_t channel,
        int64_t mode);

void rxvm_channel_binary_free(rxvm_channel_binary *binary);

/* Context teardown hook. It cancels, joins and releases every still-open
 * execution-local channel before the controller generation is unpinned. */
void rxvm_channel_context_destroy(struct rxvm_context *context);

/* Focused private test counters; no public RXVML ABI is exposed. */
size_t rxvm_channel_context_live_channels(const struct rxvm_context *context);
size_t rxvm_channel_context_live_tickets(const struct rxvm_context *context);

#endif
