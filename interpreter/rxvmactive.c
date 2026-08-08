/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include <stdio.h>
#include <stdlib.h>

#include "rxvmintp.h"

#if defined(_MSC_VER)
#define RXVM_THREAD_LOCAL __declspec(thread)
#else
#define RXVM_THREAD_LOCAL __thread
#endif

static RXVM_THREAD_LOCAL rxvm_context *rxvm_thread_active_context;

rxvm_context *rxvm_active_context_current(void) {
    return rxvm_thread_active_context;
}

rxvm_context *rxvm_active_context_enter(rxvm_context *context) {
    rxvm_context *previous = rxvm_thread_active_context;

    if (context && !rxvm_worker_is_current_thread_owner(&context->worker)) {
        fprintf(stderr,
                "RXVM active-context entry rejected: wrong worker owner thread\n");
        abort();
    }
    rxvm_thread_active_context = context;
    return previous;
}

void rxvm_active_context_leave(rxvm_context *previous_context) {
    if (previous_context &&
        !rxvm_worker_is_current_thread_owner(&previous_context->worker)) {
        fprintf(stderr,
                "RXVM active-context restore rejected: wrong worker owner thread\n");
        abort();
    }
    rxvm_thread_active_context = previous_context;
}
