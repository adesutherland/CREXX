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
struct rxvm_runtime;

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

/* Private F1 provider seam.  This header is not installed and the operation
 * table is deliberately rebuild-together until the F2 provider ABI review. */
#define RXVM_CHANNEL_PROVIDER_ABI_VERSION 1u
#define RXVM_CHANNEL_EXTENSION_PROVIDER_MIN INT64_C(65536)

typedef struct rxvm_channel_provider_completion {
    int64_t state;
    int64_t error_code;
    const char *message;
    const unsigned char *result_node;
    size_t result_node_length;
    const unsigned char *details_node;
    size_t details_node_length;
    unsigned char inline_result_node[20];
} rxvm_channel_provider_completion;

typedef struct rxvm_channel_provider_operations {
    rxvm_channel_status (*open)(
            void *module_state,
            struct rxvm_context *context,
            const void *configuration,
            size_t configuration_length,
            void **channel_state_out);
    rxvm_channel_status (*start)(
            void *channel_state,
            const void *envelope,
            size_t envelope_length,
            int64_t wait_microseconds,
            void **request_state_out);
    int (*terminal_snapshot)(
            void *channel_state,
            void *request_state,
            rxvm_channel_provider_completion *completion_out,
            uint64_t *completion_order_out);
    uint64_t (*completion_generation)(void *channel_state);
    int (*completion_wait)(
            void *channel_state,
            uint64_t observed_generation,
            int64_t wait_microseconds);
    rxvm_channel_status (*cancel)(
            void *channel_state,
            void *request_state,
            const void *reason,
            size_t reason_length);
    rxvm_channel_status (*close)(void *channel_state, int64_t mode);
    rxvm_channel_status (*request_destroy)(
            void *channel_state,
            void *request_state);
    void (*channel_destroy)(void *channel_state);
} rxvm_channel_provider_operations;

typedef struct rxvm_channel_provider_descriptor {
    int64_t type;
    const char *name;
    uint32_t abi_version;
    uint32_t configuration_version_min;
    uint32_t configuration_version_max;
    uint64_t capabilities;
    rxvm_channel_provider_operations operations;
    void *module_state;
    void (*module_retain)(void *module_state);
    void (*module_release)(void *module_state);
} rxvm_channel_provider_descriptor;

typedef enum rxvm_channel_provider_registration_result {
    RXVM_CHANNEL_PROVIDER_REGISTRATION_OK = 0,
    RXVM_CHANNEL_PROVIDER_REGISTRATION_INVALID = 1,
    RXVM_CHANNEL_PROVIDER_REGISTRATION_DUPLICATE_TYPE = 2,
    RXVM_CHANNEL_PROVIDER_REGISTRATION_DUPLICATE_NAME = 3,
    RXVM_CHANNEL_PROVIDER_REGISTRATION_OUT_OF_MEMORY = 4,
    RXVM_CHANNEL_PROVIDER_REGISTRATION_PINNED = 5,
    RXVM_CHANNEL_PROVIDER_REGISTRATION_NOT_FOUND = 6
} rxvm_channel_provider_registration_result;

rxvm_channel_provider_registration_result rxvm_channel_provider_register(
        struct rxvm_runtime *runtime,
        const rxvm_channel_provider_descriptor *descriptor);
rxvm_channel_provider_registration_result rxvm_channel_provider_unregister(
        struct rxvm_runtime *runtime,
        int64_t provider_type);

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
