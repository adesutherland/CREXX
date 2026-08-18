/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXVMCHANNEL_CHILD_H
#define CREXX_RXVMCHANNEL_CHILD_H

#include "rxvmchannel.h"
#include "rxvmchannel_byte.h"

#define RXVM_CHANNEL_PROVIDER_CHILD_PROCESS INT64_C(5)

void rxvm_channel_child_provider_descriptor(
        rxvm_channel_byte_registry *byte_registry,
        rxvm_channel_provider_descriptor *descriptor);

#endif
