/* cREXX License (MIT)
 * Copyright (c) 2026 the cREXX contributors
 * Synchronous VM signals for the explicit single-thread build.
 * No OS signal handlers, timer callbacks or worker doorbells are installed.
 */
#include "rxvmintp.h"

static rxvm_context *process_main;
static size_t signal_users;

int rxvm_signal_bind_process_main(rxvm_context *context) {
    if (!context || (process_main && process_main != context)) return -1;
    process_main = context;
    return 0;
}

int rxvm_signal_enter_execution(rxvm_context *context,
        volatile sig_atomic_t *pending,
        volatile sig_atomic_t **previous_out) {
    volatile sig_atomic_t *previous;
    if (!context || !pending || !previous_out) return -1;
    previous = context->active.pending_interrupts;
    *pending = previous ? *previous : 0;
    if (previous) *previous = 0;
    context->active.pending_interrupts = pending;
    *previous_out = previous;
    return 0;
}

int rxvm_signal_leave_execution(rxvm_context *context,
        volatile sig_atomic_t *pending, volatile sig_atomic_t *previous) {
    if (!context || !pending || context->active.pending_interrupts != pending)
        return -1;
    context->active.pending_interrupts = previous;
    if (previous) *previous |= *pending;
    *pending = 0;
    return 0;
}

void rxvm_signal_pending_or(volatile sig_atomic_t *pending, sig_atomic_t mask) {
    *pending |= mask;
}
void rxvm_signal_pending_and(volatile sig_atomic_t *pending, sig_atomic_t mask) {
    *pending &= mask;
}
void rxvm_signal_raise_process_main(unsigned char signal) {
    if (process_main && process_main->active.pending_interrupts)
        *process_main->active.pending_interrupts |= rxsignal_mask(signal);
}
void rxvm_signal_clear_process_main(unsigned char signal) {
    if (process_main && process_main->active.pending_interrupts)
        *process_main->active.pending_interrupts &= ~rxsignal_mask(signal);
}

/* Language handlers are configured in the VM's own interrupt tables. These
 * hooks cannot install an OS handler in this build. */
int enable_interrupt(int signal) { (void)signal; return -1; }
int ignore_interrupt(int signal) { (void)signal; return -1; }
int restore_interrupt(int signal) { (void)signal; return -1; }
int initialize_vm_signals(void) { ++signal_users; return 0; }
void cleanup_vm_signals(void) {
    if (signal_users && --signal_users == 0) process_main = 0;
}
