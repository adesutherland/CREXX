/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXVMCHANNEL_INTERNAL_H
#define CREXX_RXVMCHANNEL_INTERNAL_H

#include "rxvmchannel.h"
#include "rxvmexecutor.h"

typedef struct rxvm_channel_task_invoke {
    uint64_t callable_id;
    rxvm_executor_register_image *arguments;
    size_t argument_count;
    int64_t target_kind;
} rxvm_channel_task_invoke;

rxvm_channel_status rxvm_channel_parse_task_pool_configuration(
        const void *data,
        size_t length,
        int64_t provider_type,
        size_t *worker_count_out,
        size_t *admission_capacity_out);

rxvm_channel_status rxvm_channel_parse_task_scope_configuration(
        const void *data,
        size_t length,
        int64_t provider_type,
        int64_t *pool_capability_out,
        int64_t *failure_policy_out,
        int64_t *timeout_microseconds_out);

rxvm_channel_status rxvm_channel_parse_task_invoke(
        const void *data,
        size_t length,
        rxvm_channel_task_invoke *invoke);
void rxvm_channel_task_invoke_free(rxvm_channel_task_invoke *invoke);

rxvm_channel_status rxvm_channel_resolve_provider_state(
        struct rxvm_context *context,
        int64_t capability,
        int64_t provider_type,
        void **provider_state_out);

int rxvm_channel_validate_node_frame(
        const void *data,
        size_t length);

void rxvm_channel_completion_from_executor(
        const rxvm_executor_completion *executor_completion,
        rxvm_channel_provider_completion *completion_out);

rxvm_channel_status rxvm_channel_encode_process_completion(
        const rxvm_executor_completion *executor_completion,
        unsigned char **document_out,
        size_t *document_length_out);
rxvm_channel_status rxvm_channel_decode_process_completion(
        const void *document,
        size_t document_length,
        rxvm_channel_provider_completion *completion_out);

#endif
