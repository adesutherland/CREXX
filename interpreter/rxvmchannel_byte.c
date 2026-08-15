/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmchannel_byte.h"

#include <limits.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#ifndef RtlGenRandom
extern BOOLEAN NTAPI SystemFunction036(PVOID, ULONG);
#define RtlGenRandom SystemFunction036
#endif
typedef CRITICAL_SECTION byte_channel_mutex;
typedef CONDITION_VARIABLE byte_channel_condition;
typedef HANDLE byte_channel_thread;
#define BYTE_THREAD_RETURN DWORD WINAPI
static int byte_mutex_init(byte_channel_mutex *mutex) {
    InitializeCriticalSection(mutex);
    return 1;
}
static void byte_mutex_destroy(byte_channel_mutex *mutex) {
    DeleteCriticalSection(mutex);
}
static void byte_mutex_lock(byte_channel_mutex *mutex) {
    EnterCriticalSection(mutex);
}
static void byte_mutex_unlock(byte_channel_mutex *mutex) {
    LeaveCriticalSection(mutex);
}
static int byte_condition_init(byte_channel_condition *condition) {
    InitializeConditionVariable(condition);
    return 1;
}
static void byte_condition_destroy(byte_channel_condition *condition) {
    (void)condition;
}
static void byte_condition_broadcast(byte_channel_condition *condition) {
    WakeAllConditionVariable(condition);
}
static int byte_condition_wait(byte_channel_condition *condition,
                               byte_channel_mutex *mutex,
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
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#if defined(__linux__)
#include <sys/random.h>
#endif
typedef pthread_mutex_t byte_channel_mutex;
typedef pthread_cond_t byte_channel_condition;
typedef pthread_t byte_channel_thread;
#define BYTE_THREAD_RETURN void *
static int byte_mutex_init(byte_channel_mutex *mutex) {
    return pthread_mutex_init(mutex, 0) == 0;
}
static void byte_mutex_destroy(byte_channel_mutex *mutex) {
    if (pthread_mutex_destroy(mutex) != 0) abort();
}
static void byte_mutex_lock(byte_channel_mutex *mutex) {
    if (pthread_mutex_lock(mutex) != 0) abort();
}
static void byte_mutex_unlock(byte_channel_mutex *mutex) {
    if (pthread_mutex_unlock(mutex) != 0) abort();
}
static int byte_condition_init(byte_channel_condition *condition) {
    return pthread_cond_init(condition, 0) == 0;
}
static void byte_condition_destroy(byte_channel_condition *condition) {
    if (pthread_cond_destroy(condition) != 0) abort();
}
static void byte_condition_broadcast(byte_channel_condition *condition) {
    if (pthread_cond_broadcast(condition) != 0) abort();
}
static int byte_condition_wait(byte_channel_condition *condition,
                               byte_channel_mutex *mutex,
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

#define BYTE_CHANNEL_MAX_DOCUMENT (16u * 1024u * 1024u)
#define BYTE_CHANNEL_CAPABILITIES UINT64_C(0x007f)
#define BYTE_REFERENCE_IDENTITY_LENGTH 32u

enum byte_request_operation {
    BYTE_REQUEST_READ = 1,
    BYTE_REQUEST_WRITE = 2,
    BYTE_REQUEST_DRAIN = 3,
    BYTE_REQUEST_HALF_CLOSE = 4,
    BYTE_REQUEST_EXPORT_REFERENCE = 5
};

typedef struct byte_reference_entry {
    struct byte_reference_entry *next;
    rxvm_byte_endpoint *endpoint;
    uint32_t rights;
    unsigned char identity[BYTE_REFERENCE_IDENTITY_LENGTH];
} byte_reference_entry;

struct rxvm_channel_byte_registry {
    byte_channel_mutex mutex;
    byte_reference_entry *references;
};

typedef struct byte_node {
    unsigned int tag;
    const unsigned char *payload;
    size_t payload_length;
    size_t total_length;
} byte_node;

typedef struct byte_record {
    const unsigned char *schema;
    size_t schema_length;
    uint32_t version;
    uint64_t field_count;
    const unsigned char *fields;
    size_t fields_length;
} byte_record;

typedef struct byte_channel_state byte_channel_state;

typedef struct byte_channel_request {
    byte_channel_state *owner;
    struct byte_channel_request *next;
    byte_channel_thread thread;
    unsigned char *payload;
    size_t payload_length;
    size_t maximum_bytes;
    int64_t wait_microseconds;
    int direction;
    enum byte_request_operation operation;
    atomic_uchar cancelled;
    unsigned char thread_started;
    unsigned char thread_joined;
    unsigned char terminal;
    int64_t completion_state;
    int64_t error_code;
    const char *message;
    uint64_t completion_order;
    unsigned char *result_node;
    size_t result_node_length;
    unsigned char *details_node;
    size_t details_node_length;
} byte_channel_request;

struct byte_channel_state {
    rxvm_channel_byte_registry *registry;
    rxvm_byte_endpoint *endpoint;
    byte_channel_request *requests;
    byte_channel_mutex mutex;
    byte_channel_condition changed;
    uint64_t completion_generation;
    uint64_t next_completion_order;
    uint32_t rights;
    unsigned char owns_lifecycle;
    unsigned char closed;
};

static uint32_t byte_u32(const unsigned char *data);
static uint64_t byte_u64(const unsigned char *data);
static int64_t byte_i64(const unsigned char *data);
static int byte_node_open(const unsigned char *data,
                          size_t length,
                          byte_node *node);

static int byte_random(void *buffer, size_t length) {
#if defined(_WIN32)
    return length <= ULONG_MAX && RtlGenRandom(buffer, (ULONG)length) ? 1 : 0;
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || \
      defined(__NetBSD__)
    arc4random_buf(buffer, length);
    return 1;
#elif defined(__linux__)
    {
        ssize_t received = getrandom(buffer, length, 0);
        if (received == (ssize_t)length) return 1;
    }
#endif
#if !defined(_WIN32)
    {
        int descriptor = open("/dev/urandom", O_RDONLY);
        size_t offset = 0u;
        if (descriptor < 0) return 0;
        while (offset < length) {
            ssize_t received = read(
                    descriptor, (unsigned char *)buffer + offset,
                    length - offset);
            if (received <= 0) {
                close(descriptor);
                return 0;
            }
            offset += (size_t)received;
        }
        close(descriptor);
        return 1;
    }
#endif
}

static int byte_identity_equal(const unsigned char *left,
                               const unsigned char *right) {
    unsigned char different = 0u;
    size_t index;
    for (index = 0u; index < BYTE_REFERENCE_IDENTITY_LENGTH; index++) {
        different |= left[index] ^ right[index];
    }
    return different == 0u;
}

static uint64_t byte_monotonic_microseconds(void) {
#if defined(_WIN32)
    return (uint64_t)GetTickCount64() * UINT64_C(1000);
#else
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) return 0u;
    return (uint64_t)now.tv_sec * UINT64_C(1000000) +
           (uint64_t)now.tv_nsec / UINT64_C(1000);
#endif
}

static int64_t byte_remaining_microseconds(uint64_t deadline) {
    uint64_t now;
    uint64_t remaining;
    if (!deadline) return -1;
    now = byte_monotonic_microseconds();
    if (!now || now >= deadline) return 0;
    remaining = deadline - now;
    return remaining > (uint64_t)INT64_MAX
            ? INT64_MAX : (int64_t)remaining;
}

rxvm_channel_byte_registry *rxvm_channel_byte_registry_create(void) {
    rxvm_channel_byte_registry *registry =
            (rxvm_channel_byte_registry *)calloc(1u, sizeof(*registry));
    if (!registry) return 0;
    if (!byte_mutex_init(&registry->mutex)) {
        free(registry);
        return 0;
    }
    return registry;
}

void rxvm_channel_byte_registry_destroy(
        rxvm_channel_byte_registry *registry) {
    byte_reference_entry *entry;
    if (!registry) return;
    entry = registry->references;
    while (entry) {
        byte_reference_entry *next = entry->next;
        rxvm_byte_endpoint_release(entry->endpoint);
        free(entry);
        entry = next;
    }
    byte_mutex_destroy(&registry->mutex);
    free(registry);
}

static rxvm_channel_status byte_registry_export(
        rxvm_channel_byte_registry *registry,
        rxvm_byte_endpoint *endpoint,
        uint32_t rights,
        unsigned char identity[BYTE_REFERENCE_IDENTITY_LENGTH]) {
    byte_reference_entry *entry;
    byte_reference_entry *candidate;
    if (!registry || !endpoint || !rights || rights > 3u ||
        (rights & (uint32_t)rxvm_byte_endpoint_direction(endpoint)) != rights) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    byte_mutex_lock(&registry->mutex);
    for (entry = registry->references; entry; entry = entry->next) {
        if (entry->endpoint == endpoint && entry->rights == rights) {
            memcpy(identity, entry->identity, BYTE_REFERENCE_IDENTITY_LENGTH);
            byte_mutex_unlock(&registry->mutex);
            return RXVM_CHANNEL_OK;
        }
    }
    candidate = (byte_reference_entry *)calloc(1u, sizeof(*candidate));
    if (!candidate || !byte_random(
            candidate->identity, sizeof(candidate->identity))) {
        free(candidate);
        byte_mutex_unlock(&registry->mutex);
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    for (entry = registry->references; entry; entry = entry->next) {
        if (byte_identity_equal(entry->identity, candidate->identity)) {
            free(candidate);
            byte_mutex_unlock(&registry->mutex);
            return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
        }
    }
    candidate->endpoint = endpoint;
    candidate->rights = rights;
    rxvm_byte_endpoint_retain(endpoint);
    candidate->next = registry->references;
    registry->references = candidate;
    memcpy(identity, candidate->identity, BYTE_REFERENCE_IDENTITY_LENGTH);
    byte_mutex_unlock(&registry->mutex);
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status byte_registry_resolve(
        rxvm_channel_byte_registry *registry,
        const unsigned char identity[BYTE_REFERENCE_IDENTITY_LENGTH],
        uint32_t rights,
        rxvm_byte_endpoint **endpoint_out) {
    byte_reference_entry *entry;
    if (endpoint_out) *endpoint_out = 0;
    if (!registry || !identity || !endpoint_out || !rights || rights > 3u) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    byte_mutex_lock(&registry->mutex);
    for (entry = registry->references; entry; entry = entry->next) {
        if (entry->rights == rights &&
            byte_identity_equal(entry->identity, identity)) {
            rxvm_byte_endpoint_retain(entry->endpoint);
            *endpoint_out = entry->endpoint;
            byte_mutex_unlock(&registry->mutex);
            return RXVM_CHANNEL_OK;
        }
    }
    byte_mutex_unlock(&registry->mutex);
    return RXVM_CHANNEL_STALE_CAPABILITY;
}

rxvm_channel_status rxvm_channel_byte_reference_retain(
        rxvm_channel_byte_registry *registry,
        const unsigned char *reference_node,
        size_t reference_node_length,
        uint32_t required_rights,
        rxvm_byte_endpoint **endpoint_out) {
    byte_node node;
    int64_t provider_type;
    uint32_t reference_version;
    uint32_t reference_rights;
    uint64_t identity_length;
    if (endpoint_out) *endpoint_out = 0;
    if (!registry || !endpoint_out || !required_rights ||
        required_rights > 3u ||
        !byte_node_open(reference_node, reference_node_length, &node) ||
        node.total_length != reference_node_length || node.tag != 10u ||
        node.payload_length != 32u + BYTE_REFERENCE_IDENTITY_LENGTH) {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    provider_type = byte_i64(node.payload);
    reference_version = byte_u32(node.payload + 8u);
    reference_rights = byte_u32(node.payload + 12u);
    identity_length = byte_u64(node.payload + 24u);
    if (provider_type != RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT ||
        reference_version != 1u || !reference_rights ||
        reference_rights > 3u ||
        (reference_rights & required_rights) != required_rights ||
        node.payload[16u] != 1u ||
        memcmp(node.payload + 17u, "\0\0\0\0\0\0\0", 7u) != 0 ||
        identity_length != BYTE_REFERENCE_IDENTITY_LENGTH) {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    return byte_registry_resolve(
            registry, node.payload + 32u, reference_rights, endpoint_out);
}

static uint16_t byte_u16(const unsigned char *data) {
    return (uint16_t)data[0] | (uint16_t)((uint16_t)data[1] << 8u);
}

static uint32_t byte_u32(const unsigned char *data) {
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8u) |
           ((uint32_t)data[2] << 16u) | ((uint32_t)data[3] << 24u);
}

static uint64_t byte_u64(const unsigned char *data) {
    uint64_t result = 0u;
    unsigned int index;
    for (index = 0u; index < 8u; index++) {
        result |= (uint64_t)data[index] << (index * 8u);
    }
    return result;
}

static int64_t byte_i64(const unsigned char *data) {
    uint64_t bits = byte_u64(data);
    int64_t result;
    memcpy(&result, &bits, sizeof(result));
    return result;
}

static void byte_put_u32(unsigned char *data, uint32_t value) {
    unsigned int index;
    for (index = 0u; index < 4u; index++) {
        data[index] = (unsigned char)(value >> (index * 8u));
    }
}

static void byte_put_u64(unsigned char *data, uint64_t value) {
    unsigned int index;
    for (index = 0u; index < 8u; index++) {
        data[index] = (unsigned char)(value >> (index * 8u));
    }
}

static int byte_node_open(const unsigned char *data,
                          size_t length,
                          byte_node *node) {
    uint64_t payload_length;
    if (!data || !node || length < 12u || data[1] || byte_u16(data + 2u)) {
        return 0;
    }
    payload_length = byte_u64(data + 4u);
    if (payload_length > SIZE_MAX || (size_t)payload_length > length - 12u) {
        return 0;
    }
    node->tag = data[0];
    node->payload = data + 12u;
    node->payload_length = (size_t)payload_length;
    node->total_length = 12u + node->payload_length;
    return 1;
}

static int byte_document_root(const void *document,
                              size_t length,
                              byte_node *root) {
    const unsigned char *data = (const unsigned char *)document;
    return data && length >= 28u && length <= BYTE_CHANNEL_MAX_DOCUMENT &&
           !memcmp(data, "RXCV", 4u) && data[4] == 1u && !data[5] &&
           byte_u64(data + 8u) == length &&
           byte_node_open(data + 16u, length - 16u, root) &&
           root->total_length == length - 16u;
}

static int byte_record_open(const byte_node *node, byte_record *record) {
    const unsigned char *cursor;
    size_t remaining;
    uint64_t schema_length;
    if (!node || !record || node->tag != 9u || node->payload_length < 20u) {
        return 0;
    }
    cursor = node->payload;
    remaining = node->payload_length;
    schema_length = byte_u64(cursor);
    if (!schema_length || schema_length > SIZE_MAX ||
        (size_t)schema_length > remaining - 8u) return 0;
    record->schema = cursor + 8u;
    record->schema_length = (size_t)schema_length;
    cursor += 8u + record->schema_length;
    remaining -= 8u + record->schema_length;
    if (remaining < 12u) return 0;
    record->version = byte_u32(cursor);
    record->field_count = byte_u64(cursor + 4u);
    record->fields = cursor + 12u;
    record->fields_length = remaining - 12u;
    return 1;
}

static int byte_record_schema(const byte_record *record,
                              const char *schema,
                              uint64_t minimum_fields,
                              uint64_t maximum_fields) {
    size_t length = strlen(schema);
    return record && record->schema_length == length &&
           !memcmp(record->schema, schema, length) && record->version == 1u &&
           record->field_count >= minimum_fields &&
           record->field_count <= maximum_fields;
}

static int byte_record_field(const byte_record *record,
                             const char *name,
                             byte_node *node) {
    const unsigned char *cursor;
    size_t remaining;
    size_t expected = strlen(name);
    uint64_t index;
    if (!record || !node) return 0;
    cursor = record->fields;
    remaining = record->fields_length;
    for (index = 0u; index < record->field_count; index++) {
        uint64_t length;
        byte_node candidate;
        if (remaining < 8u) return 0;
        length = byte_u64(cursor);
        cursor += 8u;
        remaining -= 8u;
        if (length > SIZE_MAX || (size_t)length > remaining ||
            !byte_node_open(cursor + (size_t)length,
                            remaining - (size_t)length, &candidate)) return 0;
        if ((size_t)length == expected && !memcmp(cursor, name, expected)) {
            *node = candidate;
            return 1;
        }
        cursor += (size_t)length + candidate.total_length;
        remaining -= (size_t)length + candidate.total_length;
    }
    return 0;
}

static int byte_node_integer(const byte_node *node, int64_t *value) {
    if (!node || node->tag != 3u || node->payload_length != 8u) return 0;
    if (value) *value = byte_i64(node->payload);
    return 1;
}

static unsigned char *byte_integer_node(int64_t value, size_t *length_out) {
    unsigned char *node = (unsigned char *)calloc(1u, 20u);
    uint64_t bits;
    if (!node) return 0;
    memcpy(&bits, &value, sizeof(bits));
    node[0] = 3u;
    byte_put_u64(node + 4u, 8u);
    byte_put_u64(node + 12u, bits);
    if (length_out) *length_out = 20u;
    return node;
}

static unsigned char *byte_binary_node(const void *bytes,
                                       size_t length,
                                       size_t *length_out) {
    unsigned char *node;
    if (length > SIZE_MAX - 12u) return 0;
    node = (unsigned char *)calloc(1u, 12u + length);
    if (!node) return 0;
    node[0] = 7u;
    byte_put_u64(node + 4u, length);
    if (length) memcpy(node + 12u, bytes, length);
    if (length_out) *length_out = 12u + length;
    return node;
}

static unsigned char *byte_provider_reference_node(
        uint32_t rights,
        const unsigned char identity[BYTE_REFERENCE_IDENTITY_LENGTH],
        size_t *length_out) {
    const size_t payload_length = 32u + BYTE_REFERENCE_IDENTITY_LENGTH;
    unsigned char *node = (unsigned char *)calloc(1u, 12u + payload_length);
    if (!node) return 0;
    node[0] = 10u;
    byte_put_u64(node + 4u, payload_length);
    byte_put_u64(node + 12u, (uint64_t)RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT);
    byte_put_u32(node + 20u, 1u);
    byte_put_u32(node + 24u, rights);
    node[28u] = 1u;
    byte_put_u64(node + 36u, BYTE_REFERENCE_IDENTITY_LENGTH);
    memcpy(node + 44u, identity, BYTE_REFERENCE_IDENTITY_LENGTH);
    if (length_out) *length_out = 12u + payload_length;
    return node;
}

static unsigned char *byte_eof_details_node(int eof, size_t *length_out) {
    const char *schema = "crexx.channel.byte-read-details";
    const char *field = "eof";
    size_t schema_length = strlen(schema);
    size_t field_length = strlen(field);
    size_t payload_length = 8u + schema_length + 4u + 8u +
                            8u + field_length + 12u;
    unsigned char *node = (unsigned char *)calloc(1u, 12u + payload_length);
    unsigned char *cursor;
    if (!node) return 0;
    node[0] = 9u;
    byte_put_u64(node + 4u, payload_length);
    cursor = node + 12u;
    byte_put_u64(cursor, schema_length);
    memcpy(cursor + 8u, schema, schema_length);
    cursor += 8u + schema_length;
    byte_put_u32(cursor, 1u);
    byte_put_u64(cursor + 4u, 1u);
    cursor += 12u;
    byte_put_u64(cursor, field_length);
    memcpy(cursor + 8u, field, field_length);
    cursor += 8u + field_length;
    cursor[0] = eof ? 2u : 1u;
    if (length_out) *length_out = 12u + payload_length;
    return node;
}

static void byte_request_publish(byte_channel_request *request,
                                 rxvm_channel_status status,
                                 unsigned char *result_node,
                                 size_t result_node_length,
                                 unsigned char *details_node,
                                 size_t details_node_length) {
    byte_channel_state *state = request->owner;
    int cancelled;
    byte_mutex_lock(&state->mutex);
    if (request->terminal) abort();
    /* Cancellation and publication share this lock: first one wins. */
    cancelled = atomic_load_explicit(
            &request->cancelled, memory_order_acquire) != 0;
    request->result_node = result_node;
    request->result_node_length = result_node_length;
    request->details_node = details_node;
    request->details_node_length = details_node_length;
    if (cancelled || status == RXVM_CHANNEL_ALREADY_TERMINAL) {
        request->completion_state = 3;
        request->error_code = 0;
        request->message = "byte operation cancelled";
    } else if (status == RXVM_CHANNEL_OK) {
        request->completion_state = 1;
        request->error_code = 0;
        request->message = "";
    } else if (status == RXVM_CHANNEL_TIMEOUT) {
        request->completion_state = 4;
        request->error_code = RXVM_CHANNEL_TIMEOUT;
        request->message = "byte operation deadline exceeded";
    } else if (status == RXVM_CHANNEL_CLOSED) {
        request->completion_state = 6;
        request->error_code = RXVM_CHANNEL_CLOSED;
        request->message = "byte endpoint closed";
    } else if (status == RXVM_CHANNEL_WOULD_BLOCK) {
        request->completion_state = 5;
        request->error_code = RXVM_CHANNEL_WOULD_BLOCK;
        request->message = "byte operation would block";
    } else {
        request->completion_state = 2;
        request->error_code = status;
        request->message = "byte endpoint provider failure";
    }
    request->completion_order = ++state->next_completion_order;
    request->terminal = 1u;
    state->completion_generation++;
    byte_condition_broadcast(&state->changed);
    byte_mutex_unlock(&state->mutex);
}

static BYTE_THREAD_RETURN byte_request_run(void *opaque) {
    byte_channel_request *request = (byte_channel_request *)opaque;
    rxvm_channel_status status;
    unsigned char *result_node = 0;
    size_t result_length = 0u;
    unsigned char *details_node = 0;
    size_t details_length = 0u;
    if (request->operation == BYTE_REQUEST_READ ||
        request->operation == BYTE_REQUEST_DRAIN) {
        unsigned char *bytes = (unsigned char *)malloc(request->maximum_bytes);
        size_t length = 0u;
        int eof = 0;
        if (!bytes) status = RXVM_CHANNEL_RESOURCE_EXHAUSTED;
        else if (request->operation == BYTE_REQUEST_DRAIN) {
            uint64_t deadline = 0u;
            if (request->wait_microseconds > 0) {
                uint64_t now = byte_monotonic_microseconds();
                uint64_t duration = (uint64_t)request->wait_microseconds;
                deadline = duration > UINT64_MAX - now
                        ? UINT64_MAX : now + duration;
            }
            status = RXVM_CHANNEL_OK;
            while (length < request->maximum_bytes && !eof) {
                size_t chunk = 0u;
                int64_t remaining = deadline
                        ? byte_remaining_microseconds(deadline)
                        : request->wait_microseconds;
                if (deadline && remaining == 0) {
                    status = length ? RXVM_CHANNEL_OK : RXVM_CHANNEL_TIMEOUT;
                    break;
                }
                status = rxvm_byte_endpoint_read(
                        request->owner->endpoint, bytes + length,
                        request->maximum_bytes - length,
                        remaining, &request->cancelled,
                        &chunk, &eof);
                length += chunk;
                if (status != RXVM_CHANNEL_OK) {
                    if (length && (status == RXVM_CHANNEL_TIMEOUT ||
                                   status == RXVM_CHANNEL_WOULD_BLOCK)) {
                        status = RXVM_CHANNEL_OK;
                    }
                    break;
                }
            }
            if (status == RXVM_CHANNEL_OK) {
                result_node = byte_binary_node(bytes, length, &result_length);
                details_node = byte_eof_details_node(eof, &details_length);
                if (!result_node || !details_node) {
                    free(result_node);
                    free(details_node);
                    result_node = 0;
                    details_node = 0;
                    result_length = 0u;
                    details_length = 0u;
                    status = RXVM_CHANNEL_RESOURCE_EXHAUSTED;
                }
            }
            free(bytes);
        } else {
            status = rxvm_byte_endpoint_read(
                    request->owner->endpoint, bytes, request->maximum_bytes,
                    request->wait_microseconds, &request->cancelled,
                    &length, &eof);
            if (status == RXVM_CHANNEL_OK) {
                result_node = byte_binary_node(bytes, length, &result_length);
                details_node = byte_eof_details_node(eof, &details_length);
                if (!result_node || !details_node) {
                    free(result_node);
                    free(details_node);
                    result_node = 0;
                    details_node = 0;
                    result_length = 0u;
                    details_length = 0u;
                    status = RXVM_CHANNEL_RESOURCE_EXHAUSTED;
                }
            }
            free(bytes);
        }
    } else {
        size_t accepted = 0u;
        status = rxvm_byte_endpoint_write(
                request->owner->endpoint, request->payload,
                request->payload_length, request->wait_microseconds,
                &request->cancelled, &accepted);
        if (status == RXVM_CHANNEL_OK || accepted) {
            result_node = byte_integer_node((int64_t)accepted, &result_length);
            if (!result_node) status = RXVM_CHANNEL_RESOURCE_EXHAUSTED;
        }
    }
    byte_request_publish(request, status, result_node, result_length,
                         details_node, details_length);
#if defined(_WIN32)
    return 0u;
#else
    return 0;
#endif
}

static int byte_request_thread_start(byte_channel_request *request) {
#if defined(_WIN32)
    request->thread = CreateThread(0, 0, byte_request_run, request, 0, 0);
    return request->thread != 0;
#else
    return pthread_create(&request->thread, 0, byte_request_run, request) == 0;
#endif
}

static int byte_request_thread_join(byte_channel_request *request) {
    if (!request->thread_started || request->thread_joined) return 1;
#if defined(_WIN32)
    if (WaitForSingleObject(request->thread, INFINITE) != WAIT_OBJECT_0) {
        return 0;
    }
    CloseHandle(request->thread);
#else
    if (pthread_join(request->thread, 0) != 0) return 0;
#endif
    request->thread_joined = 1u;
    return 1;
}

static rxvm_channel_status byte_channel_open(
        void *module_state,
        struct rxvm_context *context,
        const void *configuration,
        size_t configuration_length,
        void **channel_state_out) {
    byte_node root;
    byte_record record;
    byte_node capacity_node;
    byte_node direction_node;
    byte_node initial_node;
    byte_node reference_node;
    byte_channel_state *state;
    rxvm_byte_endpoint *endpoint = 0;
    rxvm_channel_byte_registry *registry =
            (rxvm_channel_byte_registry *)module_state;
    int64_t direction;
    uint32_t rights = 0u;
    int owns_lifecycle = 1;
    if (channel_state_out) *channel_state_out = 0;
    if (!registry || !context || !channel_state_out ||
        !byte_document_root(configuration, configuration_length, &root) ||
        !byte_record_open(&root, &record)) {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    if (byte_record_schema(&record, "crexx.channel.byte-memory", 2u, 3u)) {
        int64_t capacity;
        const unsigned char *initial = 0;
        size_t initial_length = 0u;
        if (!byte_record_field(&record, "capacity", &capacity_node) ||
            !byte_record_field(&record, "direction", &direction_node) ||
            !byte_node_integer(&capacity_node, &capacity) ||
            !byte_node_integer(&direction_node, &direction) ||
            capacity < 1 || capacity > (int64_t)BYTE_CHANNEL_MAX_DOCUMENT ||
            direction < RXVM_BYTE_ENDPOINT_READ ||
            direction > RXVM_BYTE_ENDPOINT_DUPLEX) {
            return RXVM_CHANNEL_INVALID_CONFIGURATION;
        }
        if (record.field_count == 3u) {
            if (!byte_record_field(&record, "initial", &initial_node) ||
                initial_node.tag != 7u) {
                return RXVM_CHANNEL_INVALID_CONFIGURATION;
            }
            initial = initial_node.payload;
            initial_length = initial_node.payload_length;
        }
        endpoint = rxvm_byte_endpoint_create(
                (int)direction, (size_t)capacity,
                initial, initial_length, 0);
        rights = (uint32_t)direction;
    } else if (byte_record_schema(
                       &record, "crexx.channel.byte-null", 1u, 1u)) {
        if (!byte_record_field(&record, "direction", &direction_node) ||
            !byte_node_integer(&direction_node, &direction) ||
            direction < RXVM_BYTE_ENDPOINT_READ ||
            direction > RXVM_BYTE_ENDPOINT_DUPLEX) {
            return RXVM_CHANNEL_INVALID_CONFIGURATION;
        }
        endpoint = rxvm_byte_endpoint_create(
                (int)direction, 0u, 0, 0u, 1);
        rights = (uint32_t)direction;
    } else if (byte_record_schema(
                       &record, "crexx.channel.byte-reference", 1u, 1u)) {
        int64_t provider_type;
        uint32_t reference_version;
        uint32_t reference_rights;
        uint64_t identity_length;
        rxvm_channel_status status;
        if (!byte_record_field(&record, "reference", &reference_node) ||
            reference_node.tag != 10u ||
            reference_node.payload_length !=
                    32u + BYTE_REFERENCE_IDENTITY_LENGTH) {
            return RXVM_CHANNEL_INVALID_CONFIGURATION;
        }
        provider_type = byte_i64(reference_node.payload);
        reference_version = byte_u32(reference_node.payload + 8u);
        reference_rights = byte_u32(reference_node.payload + 12u);
        identity_length = byte_u64(reference_node.payload + 24u);
        if (provider_type != RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT ||
            reference_version != 1u || !reference_rights ||
            reference_rights > 3u || reference_node.payload[16u] != 1u ||
            memcmp(reference_node.payload + 17u,
                   "\0\0\0\0\0\0\0", 7u) != 0 ||
            identity_length != BYTE_REFERENCE_IDENTITY_LENGTH) {
            return RXVM_CHANNEL_INVALID_CONFIGURATION;
        }
        status = byte_registry_resolve(
                registry, reference_node.payload + 32u,
                reference_rights, &endpoint);
        if (status != RXVM_CHANNEL_OK) return status;
        rights = reference_rights;
        owns_lifecycle = 0;
    } else {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    if (!endpoint) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    state = (byte_channel_state *)calloc(1u, sizeof(*state));
    if (!state) {
        rxvm_byte_endpoint_release(endpoint);
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    if (!byte_mutex_init(&state->mutex)) {
        rxvm_byte_endpoint_release(endpoint);
        free(state);
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    if (!byte_condition_init(&state->changed)) {
        byte_mutex_destroy(&state->mutex);
        rxvm_byte_endpoint_release(endpoint);
        free(state);
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    state->endpoint = endpoint;
    state->registry = registry;
    state->rights = rights;
    state->owns_lifecycle = owns_lifecycle ? 1u : 0u;
    *channel_state_out = state;
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status byte_channel_start(
        void *channel_state,
        const void *envelope,
        size_t envelope_length,
        int64_t wait_microseconds,
        void **request_state_out) {
    byte_channel_state *state = (byte_channel_state *)channel_state;
    byte_channel_request *request;
    byte_node root;
    byte_record record;
    byte_node field;
    int64_t value;
    if (request_state_out) *request_state_out = 0;
    if (!state || !request_state_out || wait_microseconds < -1 ||
        !byte_document_root(envelope, envelope_length, &root) ||
        !byte_record_open(&root, &record)) {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    request = (byte_channel_request *)calloc(1u, sizeof(*request));
    if (!request) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    request->owner = state;
    request->wait_microseconds = wait_microseconds;
    atomic_init(&request->cancelled, 0u);
    if (byte_record_schema(&record, "crexx.channel.byte-read", 1u, 1u)) {
        if (!byte_record_field(&record, "maximumBytes", &field) ||
            !byte_node_integer(&field, &value) || value < 1 ||
            value > (int64_t)BYTE_CHANNEL_MAX_DOCUMENT ||
            !(state->rights & RXVM_BYTE_ENDPOINT_READ)) goto invalid;
        request->operation = BYTE_REQUEST_READ;
        request->maximum_bytes = (size_t)value;
    } else if (byte_record_schema(
                       &record, "crexx.channel.byte-write", 1u, 1u)) {
        if (!byte_record_field(&record, "bytes", &field) || field.tag != 7u) {
            goto invalid;
        }
        if (!(state->rights & RXVM_BYTE_ENDPOINT_WRITE)) goto invalid;
        if (field.payload_length) {
            request->payload = (unsigned char *)malloc(field.payload_length);
            if (!request->payload) {
                free(request);
                return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
            }
            memcpy(request->payload, field.payload, field.payload_length);
        }
        request->payload_length = field.payload_length;
        request->operation = BYTE_REQUEST_WRITE;
    } else if (byte_record_schema(
                       &record, "crexx.channel.byte-drain", 0u, 1u)) {
        if (!(state->rights & RXVM_BYTE_ENDPOINT_READ)) goto invalid;
        request->operation = BYTE_REQUEST_DRAIN;
        request->maximum_bytes = rxvm_byte_endpoint_capacity(state->endpoint);
        if (record.field_count) {
            if (!byte_record_field(&record, "maximumBytes", &field) ||
                !byte_node_integer(&field, &value) || value < 1 ||
                value > (int64_t)BYTE_CHANNEL_MAX_DOCUMENT) goto invalid;
            request->maximum_bytes = (size_t)value;
        }
        if (!request->maximum_bytes) request->maximum_bytes = 1u;
    } else if (byte_record_schema(
                       &record, "crexx.channel.byte-half-close", 1u, 1u)) {
        if (!byte_record_field(&record, "direction", &field) ||
            !byte_node_integer(&field, &value) ||
            value < RXVM_BYTE_ENDPOINT_READ ||
            value > RXVM_BYTE_ENDPOINT_DUPLEX ||
            ((uint32_t)value & state->rights) != (uint32_t)value) goto invalid;
        request->operation = BYTE_REQUEST_HALF_CLOSE;
        request->direction = (int)value;
    } else if (byte_record_schema(
                       &record, "crexx.channel.byte-export-reference",
                       2u, 2u)) {
        byte_node scope_node;
        if (!byte_record_field(&record, "requestedRights", &field) ||
            !byte_record_field(&record, "requestedScope", &scope_node) ||
            !byte_node_integer(&field, &value) || value < 1 || value > 3 ||
            ((uint32_t)value & state->rights) != (uint32_t)value ||
            !byte_node_integer(&scope_node, 0)) goto invalid;
        {
            int64_t scope;
            if (!byte_node_integer(&scope_node, &scope) || scope != 1) {
                goto invalid;
            }
        }
        request->operation = BYTE_REQUEST_EXPORT_REFERENCE;
        request->direction = (int)value;
    } else {
        goto invalid;
    }
    byte_mutex_lock(&state->mutex);
    if (state->closed) {
        byte_mutex_unlock(&state->mutex);
        free(request->payload);
        free(request);
        return RXVM_CHANNEL_CLOSED;
    }
    request->next = state->requests;
    state->requests = request;
    byte_mutex_unlock(&state->mutex);
    if (request->operation == BYTE_REQUEST_HALF_CLOSE) {
        rxvm_channel_status status = rxvm_byte_endpoint_half_close(
                state->endpoint, request->direction);
        byte_request_publish(request, status, 0, 0u, 0, 0u);
    } else if (request->operation == BYTE_REQUEST_EXPORT_REFERENCE) {
        unsigned char identity[BYTE_REFERENCE_IDENTITY_LENGTH];
        unsigned char *reference_node = 0;
        size_t reference_length = 0u;
        rxvm_channel_status status = byte_registry_export(
                state->registry, state->endpoint,
                (uint32_t)request->direction, identity);
        if (status == RXVM_CHANNEL_OK) {
            reference_node = byte_provider_reference_node(
                    (uint32_t)request->direction, identity,
                    &reference_length);
            if (!reference_node) status = RXVM_CHANNEL_RESOURCE_EXHAUSTED;
        }
        byte_request_publish(request, status, reference_node,
                             reference_length, 0, 0u);
    } else {
        if (!byte_request_thread_start(request)) {
            byte_channel_request **cursor;
            byte_mutex_lock(&state->mutex);
            cursor = &state->requests;
            while (*cursor && *cursor != request) cursor = &(*cursor)->next;
            if (*cursor == request) *cursor = request->next;
            byte_mutex_unlock(&state->mutex);
            free(request->payload);
            free(request);
            return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
        }
        request->thread_started = 1u;
    }
    *request_state_out = request;
    return RXVM_CHANNEL_OK;

invalid:
    free(request->payload);
    free(request);
    return RXVM_CHANNEL_INVALID_CONFIGURATION;
}

static int byte_channel_terminal_snapshot(
        void *channel_state,
        void *request_state,
        rxvm_channel_provider_completion *completion_out,
        uint64_t *completion_order_out) {
    byte_channel_state *state = (byte_channel_state *)channel_state;
    byte_channel_request *request = (byte_channel_request *)request_state;
    int terminal;
    if (completion_out) memset(completion_out, 0, sizeof(*completion_out));
    if (completion_order_out) *completion_order_out = 0u;
    if (!state || !request || request->owner != state) return -1;
    byte_mutex_lock(&state->mutex);
    terminal = request->terminal;
    if (terminal && completion_out) {
        completion_out->state = request->completion_state;
        completion_out->error_code = request->error_code;
        completion_out->message = request->message;
        completion_out->result_node = request->result_node;
        completion_out->result_node_length = request->result_node_length;
        completion_out->details_node = request->details_node;
        completion_out->details_node_length = request->details_node_length;
    }
    if (terminal && completion_order_out) {
        *completion_order_out = request->completion_order;
    }
    byte_mutex_unlock(&state->mutex);
    return terminal;
}

static uint64_t byte_channel_completion_generation(void *channel_state) {
    byte_channel_state *state = (byte_channel_state *)channel_state;
    uint64_t generation;
    if (!state) return 0u;
    byte_mutex_lock(&state->mutex);
    generation = state->completion_generation;
    byte_mutex_unlock(&state->mutex);
    return generation;
}

static int byte_channel_completion_wait(void *channel_state,
                                        uint64_t observed_generation,
                                        int64_t wait_microseconds) {
    byte_channel_state *state = (byte_channel_state *)channel_state;
    int result = 0;
    if (!state || wait_microseconds < -1) return -1;
    byte_mutex_lock(&state->mutex);
    if (state->completion_generation != observed_generation) result = 1;
    else if (wait_microseconds != 0) {
        int waited = byte_condition_wait(
                &state->changed, &state->mutex, wait_microseconds);
        if (waited < 0) result = -1;
        else if (state->completion_generation != observed_generation) result = 1;
    }
    byte_mutex_unlock(&state->mutex);
    return result;
}

static rxvm_channel_status byte_channel_cancel(
        void *channel_state,
        void *request_state,
        const void *reason,
        size_t reason_length) {
    byte_channel_state *state = (byte_channel_state *)channel_state;
    byte_channel_request *request = (byte_channel_request *)request_state;
    (void)reason;
    (void)reason_length;
    if (!state || !request || request->owner != state) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    byte_mutex_lock(&state->mutex);
    if (request->terminal) {
        byte_mutex_unlock(&state->mutex);
        return RXVM_CHANNEL_ALREADY_TERMINAL;
    }
    atomic_store_explicit(&request->cancelled, 1u, memory_order_release);
    byte_mutex_unlock(&state->mutex);
    rxvm_byte_endpoint_wake(state->endpoint);
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status byte_channel_close(void *channel_state,
                                              int64_t mode) {
    byte_channel_state *state = (byte_channel_state *)channel_state;
    byte_channel_request *request;
    if (!state || (mode != 1 && mode != 2)) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    byte_mutex_lock(&state->mutex);
    if (state->closed) {
        byte_mutex_unlock(&state->mutex);
        return RXVM_CHANNEL_CLOSED;
    }
    state->closed = 1u;
    if (mode == 2) {
        for (request = state->requests; request; request = request->next) {
            if (!request->terminal) atomic_store_explicit(
                    &request->cancelled, 1u, memory_order_release);
        }
    }
    byte_mutex_unlock(&state->mutex);
    if (state->owns_lifecycle) {
        if (mode == 2) rxvm_byte_endpoint_cancel(state->endpoint);
        else (void)rxvm_byte_endpoint_half_close(
                state->endpoint, RXVM_BYTE_ENDPOINT_DUPLEX);
    } else {
        rxvm_byte_endpoint_wake(state->endpoint);
    }
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status byte_channel_request_destroy(
        void *channel_state,
        void *request_state) {
    byte_channel_state *state = (byte_channel_state *)channel_state;
    byte_channel_request *request = (byte_channel_request *)request_state;
    byte_channel_request **cursor;
    if (!state || !request || request->owner != state ||
        !byte_request_thread_join(request)) {
        return RXVM_CHANNEL_INTERNAL_ERROR;
    }
    byte_mutex_lock(&state->mutex);
    cursor = &state->requests;
    while (*cursor && *cursor != request) cursor = &(*cursor)->next;
    if (*cursor != request) {
        byte_mutex_unlock(&state->mutex);
        return RXVM_CHANNEL_INTERNAL_ERROR;
    }
    *cursor = request->next;
    byte_mutex_unlock(&state->mutex);
    free(request->payload);
    free(request->result_node);
    free(request->details_node);
    free(request);
    return RXVM_CHANNEL_OK;
}

static void byte_channel_destroy(void *channel_state) {
    byte_channel_state *state = (byte_channel_state *)channel_state;
    if (!state) return;
    if (state->requests) abort();
    rxvm_byte_endpoint_release(state->endpoint);
    byte_condition_destroy(&state->changed);
    byte_mutex_destroy(&state->mutex);
    free(state);
}

void rxvm_channel_byte_provider_descriptor(
        rxvm_channel_byte_registry *registry,
        rxvm_channel_provider_descriptor *descriptor) {
    if (!descriptor) return;
    memset(descriptor, 0, sizeof(*descriptor));
    descriptor->type = RXVM_CHANNEL_PROVIDER_BYTE_ENDPOINT;
    descriptor->name = "crexx.core.byte-endpoint";
    descriptor->abi_version = RXVM_CHANNEL_PROVIDER_ABI_VERSION;
    descriptor->configuration_version_min = 1u;
    descriptor->configuration_version_max = 1u;
    descriptor->capabilities = BYTE_CHANNEL_CAPABILITIES;
    descriptor->module_state = registry;
    descriptor->operations.open = byte_channel_open;
    descriptor->operations.start = byte_channel_start;
    descriptor->operations.terminal_snapshot = byte_channel_terminal_snapshot;
    descriptor->operations.completion_generation =
            byte_channel_completion_generation;
    descriptor->operations.completion_wait = byte_channel_completion_wait;
    descriptor->operations.cancel = byte_channel_cancel;
    descriptor->operations.close = byte_channel_close;
    descriptor->operations.request_destroy = byte_channel_request_destroy;
    descriptor->operations.channel_destroy = byte_channel_destroy;
}
