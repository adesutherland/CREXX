//
// uuid4.c — UUIDv4 (random-based) generator implementation
//

#include "uuid4.h"
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

static inline uint64_t uuid4_splitmix64(uint64_t* state)
{
uint64_t z = (*state += 0x9E3779B97F4A7C15u);
z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9u;
z = (z ^ (z >> 27)) * 0x94D049BB133111EBu;
return z ^ (z >> 31);
}

static inline uint32_t uuid4_hash(uint32_t value)
{
static uint32_t multiplier = 0x43b0d7e5u;
value ^= multiplier;
multiplier *= 0x931e8875u;
value *= multiplier;
value ^= value >> 16;
return value;
}

static inline uint32_t uuid4_mix(uint32_t x, uint32_t y)
{
uint32_t result = 0xca01f9ddu * x - 0x4973f715u * y;
result ^= result >> 16;
return result;
}

#if defined(_WIN32)
#include <windows.h>
void uuid4_seed(UUID4_STATE_T* state)
{
    static uint64_t state0 = 0;
    LARGE_INTEGER time;
    BOOL ok = QueryPerformanceCounter(&time);
    assert(ok);

    *state = state0++ + ((uintptr_t)&time ^ (uint64_t)time.QuadPart);

    uint32_t pid = (uint32_t)GetCurrentProcessId();
    uint32_t tid = (uint32_t)GetCurrentThreadId();

    *state = *state * 6364136223846793005u +
        ((uint64_t)(uuid4_mix(uuid4_hash(pid), uuid4_hash(tid))) << 32);
    *state = *state * 6364136223846793005u + (uintptr_t)GetCurrentProcessId;
    *state = *state * 6364136223846793005u + (uintptr_t)uuid4_gen;
}

#elif defined(__linux__)
#include <unistd.h>
#include <sys/syscall.h>
void uuid4_seed(UUID4_STATE_T* state)
{
    static uint64_t state0 = 0;
    struct timespec ts;
    bool ok = clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0;
    assert(ok);

    *state = state0++ + ((uintptr_t)&ts ^ (uint64_t)(ts.tv_sec * 1000000000 + ts.tv_nsec));

    uint32_t pid = (uint32_t)getpid();
    uint32_t tid = (uint32_t)syscall(SYS_gettid);

    *state = *state * 6364136223846793005u +
        ((uint64_t)(uuid4_mix(uuid4_hash(pid), uuid4_hash(tid))) << 32);
    *state = *state * 6364136223846793005u + (uintptr_t)getpid;
    *state = *state * 6364136223846793005u + (uintptr_t)uuid4_gen;
}

#elif defined(__APPLE__)
#include <mach/mach_time.h>
#include <unistd.h>
#include <pthread.h>
void uuid4_seed(UUID4_STATE_T* state)
{
    static uint64_t state0 = 0;
    uint64_t time = mach_absolute_time();
    *state = state0++ + time;

    uint32_t pid = (uint32_t)getpid();
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);

    *state = *state * 6364136223846793005u +
        ((uint64_t)(uuid4_mix(uuid4_hash(pid), uuid4_hash((uint32_t)tid))) << 32);
    *state = *state * 6364136223846793005u + (uintptr_t)getpid;
    *state = *state * 6364136223846793005u + (uintptr_t)uuid4_gen;
}
#else
#error Unsupported platform
#endif

static void uuid4_randomize(UUID4_STATE_T* state, UUID4_T* out)
{
    out->qwords[0] = uuid4_splitmix64(state);
    out->qwords[1] = uuid4_splitmix64(state);
}

void uuid4_gen(UUID4_STATE_T* state, UUID4_T* out)
{
    uuid4_randomize(state, out);
    out->bytes[6] = (out->bytes[6] & 0x0F) | 0x40; /* version 4 */
    out->bytes[8] = (out->bytes[8] & 0x3F) | 0x80; /* variant 10 */
}

bool uuid4_to_s(const UUID4_T uuid, char* out, int capacity)
{
    static const char hex[] = "0123456789abcdef";
    static const int groups[] = { 8, 4, 4, 4, 12 };
    int b = 0;
    if (capacity < UUID4_STR_BUFFER_SIZE)
        return false;

    for (int i = 0; i < (int)(sizeof(groups)/sizeof(groups[0])); ++i) {
        for (int j = 0; j < groups[i]; j += 2) {
            uint8_t byte = uuid.bytes[b++];
            *out++ = hex[byte >> 4];
            *out++ = hex[byte & 0x0F];
        }
        *out++ = '-';
    }
    *--out = '\0';
    return true;
}
