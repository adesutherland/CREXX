/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXVMCHANNEL_PROCESS_H
#define CREXX_RXVMCHANNEL_PROCESS_H

#include "rxvmchannel.h"

#define RXVM_CHANNEL_PROVIDER_PROCESS INT64_C(2)
#define RXVM_CHANNEL_PROCESS_CAPABILITIES UINT64_C(0x010f)

void rxvm_channel_process_provider_descriptor(
        rxvm_channel_provider_descriptor *descriptor);

/* Private conformance hook. PHASE 1 severs transport before INVOKE; phase 2
 * severs it after STARTED. It is not installed or reachable from RXAS. */
int rxvm_channel_process_test_crash_next(
        struct rxvm_context *context,
        int64_t channel_capability,
        int phase);

#endif
