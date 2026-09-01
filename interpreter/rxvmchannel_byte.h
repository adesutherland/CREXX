/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXVMCHANNEL_BYTE_H
#define CREXX_RXVMCHANNEL_BYTE_H

#include "rxvmbyteendpoint.h"
#include "rxvmchannel.h"

#define RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT INT64_C(4)

typedef struct rxvm_channel_byte_registry rxvm_channel_byte_registry;

rxvm_channel_byte_registry *rxvm_channel_byte_registry_create(void);
void rxvm_channel_byte_registry_destroy(rxvm_channel_byte_registry *registry);

rxvm_channel_status rxvm_channel_byte_reference_retain(
        rxvm_channel_byte_registry *registry,
        const unsigned char *reference_node,
        size_t reference_node_length,
        uint32_t required_rights,
        rxvm_byte_endpoint **endpoint_out);

void rxvm_channel_byte_provider_descriptor(
        rxvm_channel_byte_registry *registry,
        rxvm_channel_provider_descriptor *descriptor);

#endif
