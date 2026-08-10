//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

#ifdef _WIN32
#include <windows.h>
static SRWLOCK getpi_seed_lock = SRWLOCK_INIT;
#define GETPI_SEED_LOCK() AcquireSRWLockExclusive(&getpi_seed_lock)
#define GETPI_SEED_UNLOCK() ReleaseSRWLockExclusive(&getpi_seed_lock)
#else
#include <pthread.h>
static pthread_mutex_t getpi_seed_lock = PTHREAD_MUTEX_INITIALIZER;
#define GETPI_SEED_LOCK() ((void)pthread_mutex_lock(&getpi_seed_lock))
#define GETPI_SEED_UNLOCK() ((void)pthread_mutex_unlock(&getpi_seed_lock))
#endif

RXPA_PLUGIN_PROCESS_REENTRANT

static uint64_t getpi_seed_counter;

static uint64_t getpi_next_random(uint64_t *state) {
    uint64_t value = (*state += UINT64_C(0x9e3779b97f4a7c15));
    value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
    return value ^ (value >> 31);
}

static uint64_t getpi_new_seed(void) {
    uint64_t sequence;
    uint64_t state;

    GETPI_SEED_LOCK();
    sequence = ++getpi_seed_counter;
    GETPI_SEED_UNLOCK();
    state = ((uint64_t)time(NULL) << 32) ^
            (sequence * UINT64_C(0xd1342543de82ef95)) ^
            (uint64_t)(uintptr_t)&sequence;
    return getpi_next_random(&state);
}

static double getpi_random_unit(uint64_t *state) {
    return (double)(getpi_next_random(state) >> 11) *
           (1.0 / 9007199254740992.0);
}

// Function to get an environment variable
PROCEDURE(getpi) {
    // Should never happen as the compiler checks arguments; best practice is to check this anyway
    if( NUM_ARGS != 1) RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "PI calculation type expected") // sets the signal and returns

    char *varName = GETSTRING(ARG(0)); // Get the pi type variable name
    double pi = 0.0;
    int terms, i;

    if (strncmp(&varName[0], "L",1)     == 0) {     // Calculate PI according to Leibnitz method
        terms = 10000000; // Number of terms to sum
        for (i = 0; i < terms; i++) {
            if (i % 2 == 0) pi += 1.0 / (2.0 * i + 1.0);
            else pi -= 1.0 / (2.0 * i + 1.0);
        }
        pi=pi*4.0;
     }  else if (strncmp(&varName[0], "M",1)     == 0) { // Monte Carlo method
        terms = 1000000; // Number of points to be generated
        long points_in_circle = 0;
        double x, y;
        uint64_t random_state = getpi_new_seed();
         for (i = 0; i < terms; i++) {
         // Generate random point (x, y) in the range [-1, 1]
            x = getpi_random_unit(&random_state) * 2.0 - 1.0;
            y = getpi_random_unit(&random_state) * 2.0 - 1.0;
         // Check if the point is inside the unit circle
            if (x * x + y * y <= 1) points_in_circle++;
        }
        pi = 4.0 * (double) points_in_circle / (double) terms;         // Estimate the value of Pi
    } else {     //    if (varName == NULL) {  // If the PI type variable is not found
         pi=3.14;   /* Set a constant as return value */
    }
    SETFLOAT(RETURN, pi);
 // Make sure the signal is reset to ok - best practice
    RESETSIGNAL
}

// Functions to be provided to rexx
LOADFUNCS
//      C Function, REXX namespace & name, Option, Return Type, Arguments
    ADDPROC(getpi,     "getpi.getpi",      "b",    ".float",   "pi_arg=.string");
ENDLOADFUNCS
