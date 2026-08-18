/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmbyteendpoint.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
typedef CRITICAL_SECTION rxvm_endpoint_mutex;
typedef CONDITION_VARIABLE rxvm_endpoint_condition;
static int endpoint_mutex_init(rxvm_endpoint_mutex *mutex) {
    InitializeCriticalSection(mutex);
    return 1;
}
static void endpoint_mutex_destroy(rxvm_endpoint_mutex *mutex) {
    DeleteCriticalSection(mutex);
}
static void endpoint_mutex_lock(rxvm_endpoint_mutex *mutex) {
    EnterCriticalSection(mutex);
}
static void endpoint_mutex_unlock(rxvm_endpoint_mutex *mutex) {
    LeaveCriticalSection(mutex);
}
static int endpoint_condition_init(rxvm_endpoint_condition *condition) {
    InitializeConditionVariable(condition);
    return 1;
}
static void endpoint_condition_destroy(rxvm_endpoint_condition *condition) {
    (void)condition;
}
static void endpoint_condition_broadcast(rxvm_endpoint_condition *condition) {
    WakeAllConditionVariable(condition);
}
static int endpoint_condition_wait(rxvm_endpoint_condition *condition,
                                   rxvm_endpoint_mutex *mutex,
                                   int64_t wait_microseconds) {
    DWORD milliseconds;
    BOOL result;
    if (wait_microseconds < 0) milliseconds = INFINITE;
    else {
        uint64_t rounded = ((uint64_t)wait_microseconds + 999u) / 1000u;
        milliseconds = rounded >= (uint64_t)INFINITE
                ? INFINITE - 1u : (DWORD)rounded;
    }
    result = SleepConditionVariableCS(condition, mutex, milliseconds);
    if (result) return 1;
    return GetLastError() == ERROR_TIMEOUT ? 0 : -1;
}
#else
#include <errno.h>
#include <pthread.h>
typedef pthread_mutex_t rxvm_endpoint_mutex;
typedef pthread_cond_t rxvm_endpoint_condition;
static int endpoint_mutex_init(rxvm_endpoint_mutex *mutex) {
    return pthread_mutex_init(mutex, 0) == 0;
}
static void endpoint_mutex_destroy(rxvm_endpoint_mutex *mutex) {
    if (pthread_mutex_destroy(mutex) != 0) abort();
}
static void endpoint_mutex_lock(rxvm_endpoint_mutex *mutex) {
    if (pthread_mutex_lock(mutex) != 0) abort();
}
static void endpoint_mutex_unlock(rxvm_endpoint_mutex *mutex) {
    if (pthread_mutex_unlock(mutex) != 0) abort();
}
static int endpoint_condition_init(rxvm_endpoint_condition *condition) {
    return pthread_cond_init(condition, 0) == 0;
}
static void endpoint_condition_destroy(rxvm_endpoint_condition *condition) {
    if (pthread_cond_destroy(condition) != 0) abort();
}
static void endpoint_condition_broadcast(rxvm_endpoint_condition *condition) {
    if (pthread_cond_broadcast(condition) != 0) abort();
}
static int endpoint_condition_wait(rxvm_endpoint_condition *condition,
                                   rxvm_endpoint_mutex *mutex,
                                   int64_t wait_microseconds) {
    int result;
    if (wait_microseconds < 0) {
        result = pthread_cond_wait(condition, mutex);
        return result == 0 ? 1 : -1;
    }
#if defined(__APPLE__)
    {
        struct timespec relative;
        relative.tv_sec = (time_t)(wait_microseconds / INT64_C(1000000));
        relative.tv_nsec = (long)((wait_microseconds % INT64_C(1000000)) *
                                  INT64_C(1000));
        result = pthread_cond_timedwait_relative_np(
                condition, mutex, &relative);
    }
#else
    {
        struct timespec absolute;
        uint64_t nanoseconds;
        if (clock_gettime(CLOCK_REALTIME, &absolute) != 0) return -1;
        nanoseconds = (uint64_t)absolute.tv_nsec +
                      (uint64_t)wait_microseconds * UINT64_C(1000);
        absolute.tv_sec += (time_t)(nanoseconds / UINT64_C(1000000000));
        absolute.tv_nsec = (long)(nanoseconds % UINT64_C(1000000000));
        result = pthread_cond_timedwait(condition, mutex, &absolute);
    }
#endif
    if (result == 0) return 1;
    return result == ETIMEDOUT ? 0 : -1;
}
#endif

struct rxvm_byte_endpoint {
    rxvm_endpoint_mutex mutex;
    rxvm_endpoint_condition changed;
    unsigned char *bytes;
    size_t capacity;
    size_t head;
    size_t length;
    size_t references;
    unsigned char direction;
    unsigned char null_endpoint;
    unsigned char read_closed;
    unsigned char write_closed;
    unsigned char cancelled;
};

static uint64_t endpoint_monotonic_microseconds(void) {
#if defined(_WIN32)
    return (uint64_t)GetTickCount64() * UINT64_C(1000);
#else
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return 0u;
    return (uint64_t)now.tv_sec * UINT64_C(1000000) +
           (uint64_t)now.tv_nsec / UINT64_C(1000);
#endif
}

static int64_t endpoint_remaining(uint64_t deadline) {
    uint64_t now;
    uint64_t remaining;
    if (!deadline) return -1;
    now = endpoint_monotonic_microseconds();
    if (!now || now >= deadline) return 0;
    remaining = deadline - now;
    return remaining > (uint64_t)INT64_MAX
            ? INT64_MAX : (int64_t)remaining;
}

rxvm_byte_endpoint *rxvm_byte_endpoint_create(
        int direction,
        size_t capacity,
        const void *initial_bytes,
        size_t initial_length,
        int null_endpoint) {
    rxvm_byte_endpoint *endpoint;
    if ((direction & ~RXVM_BYTE_ENDPOINT_DUPLEX) != 0 || direction == 0 ||
        (!null_endpoint && (!capacity || initial_length > capacity)) ||
        (!initial_bytes && initial_length)) return 0;
    endpoint = (rxvm_byte_endpoint *)calloc(1u, sizeof(*endpoint));
    if (!endpoint) return 0;
    if (!endpoint_mutex_init(&endpoint->mutex)) {
        free(endpoint);
        return 0;
    }
    if (!endpoint_condition_init(&endpoint->changed)) {
        endpoint_mutex_destroy(&endpoint->mutex);
        free(endpoint);
        return 0;
    }
    if (!null_endpoint) {
        endpoint->bytes = (unsigned char *)malloc(capacity);
        if (!endpoint->bytes) {
            endpoint_condition_destroy(&endpoint->changed);
            endpoint_mutex_destroy(&endpoint->mutex);
            free(endpoint);
            return 0;
        }
        if (initial_length) {
            memcpy(endpoint->bytes, initial_bytes, initial_length);
        }
    }
    endpoint->capacity = capacity;
    endpoint->length = initial_length;
    endpoint->references = 1u;
    endpoint->direction = (unsigned char)direction;
    endpoint->null_endpoint = null_endpoint ? 1u : 0u;
    if (initial_length) endpoint->write_closed = 1u;
    return endpoint;
}

void rxvm_byte_endpoint_retain(rxvm_byte_endpoint *endpoint) {
    if (!endpoint) return;
    endpoint_mutex_lock(&endpoint->mutex);
    if (!endpoint->references || endpoint->references == SIZE_MAX) abort();
    endpoint->references++;
    endpoint_mutex_unlock(&endpoint->mutex);
}

void rxvm_byte_endpoint_release(rxvm_byte_endpoint *endpoint) {
    int destroy = 0;
    if (!endpoint) return;
    endpoint_mutex_lock(&endpoint->mutex);
    if (!endpoint->references) abort();
    endpoint->references--;
    destroy = endpoint->references == 0u;
    endpoint_mutex_unlock(&endpoint->mutex);
    if (!destroy) return;
    endpoint_condition_destroy(&endpoint->changed);
    endpoint_mutex_destroy(&endpoint->mutex);
    free(endpoint->bytes);
    free(endpoint);
}

rxvm_channel_status rxvm_byte_endpoint_read(
        rxvm_byte_endpoint *endpoint,
        void *bytes,
        size_t maximum_bytes,
        int64_t wait_microseconds,
        const atomic_uchar *operation_cancelled,
        size_t *length_out,
        int *eof_out) {
    uint64_t deadline = 0u;
    rxvm_channel_status status = RXVM_CHANNEL_OK;
    if (length_out) *length_out = 0u;
    if (eof_out) *eof_out = 0;
    if (!endpoint || !bytes || !maximum_bytes || wait_microseconds < -1 ||
        !(endpoint->direction & RXVM_BYTE_ENDPOINT_READ)) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    if (wait_microseconds > 0) {
        uint64_t now = endpoint_monotonic_microseconds();
        uint64_t duration = (uint64_t)wait_microseconds;
        deadline = duration > UINT64_MAX - now ? UINT64_MAX : now + duration;
    }
    endpoint_mutex_lock(&endpoint->mutex);
    for (;;) {
        if (operation_cancelled && atomic_load_explicit(
                    operation_cancelled, memory_order_acquire)) {
            status = RXVM_CHANNEL_ALREADY_TERMINAL;
            break;
        }
        if (endpoint->cancelled) {
            status = RXVM_CHANNEL_CLOSED;
            break;
        }
        if (endpoint->null_endpoint || endpoint->read_closed) {
            if (eof_out) *eof_out = 1;
            break;
        }
        if (endpoint->length) {
            size_t first;
            size_t amount = endpoint->length < maximum_bytes
                    ? endpoint->length : maximum_bytes;
            first = endpoint->capacity - endpoint->head;
            if (first > amount) first = amount;
            memcpy(bytes, endpoint->bytes + endpoint->head, first);
            if (amount > first) {
                memcpy((unsigned char *)bytes + first, endpoint->bytes,
                       amount - first);
            }
            endpoint->head = (endpoint->head + amount) % endpoint->capacity;
            endpoint->length -= amount;
            if (length_out) *length_out = amount;
            endpoint_condition_broadcast(&endpoint->changed);
            break;
        }
        if (endpoint->write_closed) {
            if (eof_out) *eof_out = 1;
            break;
        }
        if (wait_microseconds == 0) {
            status = RXVM_CHANNEL_WOULD_BLOCK;
            break;
        }
        {
            int64_t remaining = deadline
                    ? endpoint_remaining(deadline) : -1;
            int waited;
            if (deadline && remaining == 0) {
                status = RXVM_CHANNEL_TIMEOUT;
                break;
            }
            waited = endpoint_condition_wait(
                    &endpoint->changed, &endpoint->mutex, remaining);
            if (waited < 0) {
                status = RXVM_CHANNEL_INTERNAL_ERROR;
                break;
            }
            if (!waited) {
                status = RXVM_CHANNEL_TIMEOUT;
                break;
            }
        }
    }
    endpoint_mutex_unlock(&endpoint->mutex);
    return status;
}

rxvm_channel_status rxvm_byte_endpoint_write(
        rxvm_byte_endpoint *endpoint,
        const void *bytes,
        size_t length,
        int64_t wait_microseconds,
        const atomic_uchar *operation_cancelled,
        size_t *accepted_out) {
    uint64_t deadline = 0u;
    size_t accepted = 0u;
    rxvm_channel_status status = RXVM_CHANNEL_OK;
    if (accepted_out) *accepted_out = 0u;
    if (!endpoint || (!bytes && length) || wait_microseconds < -1 ||
        !(endpoint->direction & RXVM_BYTE_ENDPOINT_WRITE)) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    if (wait_microseconds > 0) {
        uint64_t now = endpoint_monotonic_microseconds();
        uint64_t duration = (uint64_t)wait_microseconds;
        deadline = duration > UINT64_MAX - now ? UINT64_MAX : now + duration;
    }
    endpoint_mutex_lock(&endpoint->mutex);
    if (endpoint->null_endpoint) accepted = length;
    while (accepted < length && !endpoint->null_endpoint) {
        size_t available;
        size_t tail;
        size_t first;
        size_t amount;
        if (operation_cancelled && atomic_load_explicit(
                    operation_cancelled, memory_order_acquire)) {
            status = RXVM_CHANNEL_ALREADY_TERMINAL;
            break;
        }
        if (endpoint->cancelled || endpoint->read_closed ||
            endpoint->write_closed) {
            status = RXVM_CHANNEL_CLOSED;
            break;
        }
        available = endpoint->capacity - endpoint->length;
        if (!available) {
            int64_t remaining;
            int waited;
            if (wait_microseconds == 0) {
                status = accepted ? RXVM_CHANNEL_OK : RXVM_CHANNEL_WOULD_BLOCK;
                break;
            }
            remaining = deadline ? endpoint_remaining(deadline) : -1;
            if (deadline && remaining == 0) {
                status = accepted ? RXVM_CHANNEL_OK : RXVM_CHANNEL_TIMEOUT;
                break;
            }
            waited = endpoint_condition_wait(
                    &endpoint->changed, &endpoint->mutex, remaining);
            if (waited < 0) {
                status = RXVM_CHANNEL_INTERNAL_ERROR;
                break;
            }
            if (!waited) {
                status = accepted ? RXVM_CHANNEL_OK : RXVM_CHANNEL_TIMEOUT;
                break;
            }
            continue;
        }
        amount = length - accepted;
        if (amount > available) amount = available;
        tail = (endpoint->head + endpoint->length) % endpoint->capacity;
        first = endpoint->capacity - tail;
        if (first > amount) first = amount;
        memcpy(endpoint->bytes + tail,
               (const unsigned char *)bytes + accepted, first);
        if (amount > first) {
            memcpy(endpoint->bytes,
                   (const unsigned char *)bytes + accepted + first,
                   amount - first);
        }
        endpoint->length += amount;
        accepted += amount;
        endpoint_condition_broadcast(&endpoint->changed);
    }
    endpoint_mutex_unlock(&endpoint->mutex);
    if (accepted_out) *accepted_out = accepted;
    return status;
}

rxvm_channel_status rxvm_byte_endpoint_half_close(
        rxvm_byte_endpoint *endpoint,
        int direction) {
    if (!endpoint || direction < RXVM_BYTE_ENDPOINT_READ ||
        direction > RXVM_BYTE_ENDPOINT_DUPLEX) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    endpoint_mutex_lock(&endpoint->mutex);
    if (direction & RXVM_BYTE_ENDPOINT_READ) endpoint->read_closed = 1u;
    if (direction & RXVM_BYTE_ENDPOINT_WRITE) endpoint->write_closed = 1u;
    endpoint_condition_broadcast(&endpoint->changed);
    endpoint_mutex_unlock(&endpoint->mutex);
    return RXVM_CHANNEL_OK;
}

void rxvm_byte_endpoint_cancel(rxvm_byte_endpoint *endpoint) {
    if (!endpoint) return;
    endpoint_mutex_lock(&endpoint->mutex);
    endpoint->cancelled = 1u;
    endpoint_condition_broadcast(&endpoint->changed);
    endpoint_mutex_unlock(&endpoint->mutex);
}

void rxvm_byte_endpoint_wake(rxvm_byte_endpoint *endpoint) {
    if (!endpoint) return;
    endpoint_mutex_lock(&endpoint->mutex);
    endpoint_condition_broadcast(&endpoint->changed);
    endpoint_mutex_unlock(&endpoint->mutex);
}

int rxvm_byte_endpoint_direction(rxvm_byte_endpoint *endpoint) {
    int direction;
    if (!endpoint) return 0;
    endpoint_mutex_lock(&endpoint->mutex);
    direction = endpoint->direction;
    endpoint_mutex_unlock(&endpoint->mutex);
    return direction;
}

size_t rxvm_byte_endpoint_capacity(rxvm_byte_endpoint *endpoint) {
    size_t result;
    if (!endpoint) return 0u;
    endpoint_mutex_lock(&endpoint->mutex);
    result = endpoint->capacity;
    endpoint_mutex_unlock(&endpoint->mutex);
    return result;
}

size_t rxvm_byte_endpoint_buffered(rxvm_byte_endpoint *endpoint) {
    size_t result;
    if (!endpoint) return 0u;
    endpoint_mutex_lock(&endpoint->mutex);
    result = endpoint->length;
    endpoint_mutex_unlock(&endpoint->mutex);
    return result;
}
