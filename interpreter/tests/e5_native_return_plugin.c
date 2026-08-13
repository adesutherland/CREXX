/*
 * Private PERF3-13 E5 native-return progress fixture.
 *
 * The call publishes entry, remains in native code briefly, then returns to
 * bytecode. The executor test publishes cancellation only after entry so the
 * sparse owner must observe the mailbox at native/plugin return.
 */

#include "crexxpa.h"

#if defined(_WIN32)
#include <windows.h>
static volatile LONG e5_native_entered;
#else
#include <time.h>
static volatile int e5_native_entered;
#endif

void e5_native_return_reset(void) {
#if defined(_WIN32)
    InterlockedExchange(&e5_native_entered, 0);
#else
    __atomic_store_n(&e5_native_entered, 0, __ATOMIC_RELEASE);
#endif
}

int e5_native_return_entered(void) {
#if defined(_WIN32)
    return InterlockedCompareExchange(&e5_native_entered, 0, 0) != 0;
#else
    return __atomic_load_n(&e5_native_entered, __ATOMIC_ACQUIRE) != 0;
#endif
}

PROCEDURE(pause)
{
    (void)_numargs;
    (void)_arg;
    (void)_signal;
#if defined(_WIN32)
    InterlockedExchange(&e5_native_entered, 1);
    Sleep(50);
#else
    struct timespec delay = {0, 50000000L};
    __atomic_store_n(&e5_native_entered, 1, __ATOMIC_RELEASE);
    (void)nanosleep(&delay, 0);
#endif
    RETURNINT(1);
}

LOADFUNCS
ADDPROC(pause, "e5native.pause", "b", ".int", "");
ENDLOADFUNCS
