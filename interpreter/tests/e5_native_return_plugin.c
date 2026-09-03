/*
 * Private PERF3-13 E5 native-return progress fixture.
 *
 * The call publishes entry and waits for an explicit release from the
 * controller. The controller publishes cancellation before that release so
 * the sparse owner must observe the mailbox at native/plugin return.
 */

#include "crexxpa.h"

#if defined(_WIN32)
#include <windows.h>
static volatile LONG e5_native_entered;
static volatile LONG e5_native_release_requested;
#else
#include <time.h>
static volatile int e5_native_entered;
static volatile int e5_native_release_requested;
#endif

void e5_native_return_reset(void) {
#if defined(_WIN32)
    InterlockedExchange(&e5_native_entered, 0);
    InterlockedExchange(&e5_native_release_requested, 0);
#else
    __atomic_store_n(&e5_native_entered, 0, __ATOMIC_RELEASE);
    __atomic_store_n(&e5_native_release_requested, 0, __ATOMIC_RELEASE);
#endif
}

int e5_native_return_entered(void) {
#if defined(_WIN32)
    return InterlockedCompareExchange(&e5_native_entered, 0, 0) != 0;
#else
    return __atomic_load_n(&e5_native_entered, __ATOMIC_ACQUIRE) != 0;
#endif
}

void e5_native_return_release(void) {
#if defined(_WIN32)
    InterlockedExchange(&e5_native_release_requested, 1);
#else
    __atomic_store_n(&e5_native_release_requested, 1, __ATOMIC_RELEASE);
#endif
}

static int e5_native_return_released(void) {
#if defined(_WIN32)
    return InterlockedCompareExchange(
            &e5_native_release_requested, 0, 0) != 0;
#else
    return __atomic_load_n(
            &e5_native_release_requested, __ATOMIC_ACQUIRE) != 0;
#endif
}

PROCEDURE(pause)
{
    (void)_numargs;
    (void)_arg;
    (void)_signal;
#if defined(_WIN32)
    InterlockedExchange(&e5_native_entered, 1);
#else
    __atomic_store_n(&e5_native_entered, 1, __ATOMIC_RELEASE);
#endif
    /* The CTest timeout bounds a broken handshake; elapsed time establishes
     * no ordering between native return and mailbox publication. */
    while (!e5_native_return_released()) {
#if defined(_WIN32)
        Sleep(1);
#else
        struct timespec delay = {0, 1000000L};
        (void)nanosleep(&delay, 0);
#endif
    }
    RETURNINT(1);
}

LOADFUNCS
ADDPROC(pause, "e5native.pause", "b", ".int", "");
ENDLOADFUNCS
