/*
 * Verify that every valid VM signal has one distinct, positive sig_atomic_t
 * mask bit and that sentinel/out-of-range codes cannot create a mask.
 */

#include "rxsignal.h"

int main(void) {
    sig_atomic_t seen = 0;
    int signal_code;

    if (rxsignal_mask(-1) != 0 ||
        rxsignal_mask(RXSIGNAL_NONE) != 0 ||
        rxsignal_mask(RXSIGNAL_MAX) != 0 ||
        rxsignal_mask(RXSIGNAL_MAX + 1) != 0) return 1;

    for (signal_code = 1; signal_code < RXSIGNAL_MAX; ++signal_code) {
        sig_atomic_t mask = rxsignal_mask(signal_code);
        if (mask <= 0 || (seen & mask) != 0) return 1;
        seen |= mask;
    }

    return 0;
}
