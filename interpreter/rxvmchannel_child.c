/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmchannel_child.h"

#include "rxcrexxcmd.h"
#include "rxvmintp.h"

#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
typedef CRITICAL_SECTION child_mutex;
typedef CONDITION_VARIABLE child_condition;
typedef HANDLE child_thread;
#define CHILD_THREAD_RETURN DWORD WINAPI
static int child_mutex_init(child_mutex *mutex) {
    InitializeCriticalSection(mutex);
    return 1;
}
static void child_mutex_destroy(child_mutex *mutex) {
    DeleteCriticalSection(mutex);
}
static void child_mutex_lock(child_mutex *mutex) {
    EnterCriticalSection(mutex);
}
static void child_mutex_unlock(child_mutex *mutex) {
    LeaveCriticalSection(mutex);
}
static int child_condition_init(child_condition *condition) {
    InitializeConditionVariable(condition);
    return 1;
}
static void child_condition_destroy(child_condition *condition) {
    (void)condition;
}
static void child_condition_broadcast(child_condition *condition) {
    WakeAllConditionVariable(condition);
}
static int child_condition_wait(child_condition *condition,
                                child_mutex *mutex,
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
typedef pthread_mutex_t child_mutex;
typedef pthread_cond_t child_condition;
typedef pthread_t child_thread;
#define CHILD_THREAD_RETURN void *
static int child_mutex_init(child_mutex *mutex) {
    return pthread_mutex_init(mutex, 0) == 0;
}
static void child_mutex_destroy(child_mutex *mutex) {
    if (pthread_mutex_destroy(mutex) != 0) abort();
}
static void child_mutex_lock(child_mutex *mutex) {
    if (pthread_mutex_lock(mutex) != 0) abort();
}
static void child_mutex_unlock(child_mutex *mutex) {
    if (pthread_mutex_unlock(mutex) != 0) abort();
}
static int child_condition_init(child_condition *condition) {
    return pthread_cond_init(condition, 0) == 0;
}
static void child_condition_destroy(child_condition *condition) {
    if (pthread_cond_destroy(condition) != 0) abort();
}
static void child_condition_broadcast(child_condition *condition) {
    if (pthread_cond_broadcast(condition) != 0) abort();
}
static int child_condition_wait(child_condition *condition,
                                child_mutex *mutex,
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

#define CHILD_MAX_DOCUMENT (16u * 1024u * 1024u)
#define CHILD_CAPABILITIES UINT64_C(0x00ff)

typedef struct child_node {
    unsigned int tag;
    const unsigned char *payload;
    size_t payload_length;
    size_t total_length;
} child_node;

typedef struct child_record {
    const unsigned char *schema;
    size_t schema_length;
    uint32_t version;
    uint64_t field_count;
    const unsigned char *fields;
    size_t fields_length;
} child_record;

typedef struct child_buffer {
    unsigned char *data;
    size_t length;
    size_t capacity;
} child_buffer;

typedef struct child_channel child_channel;

typedef struct child_request {
    child_channel *owner;
    struct child_request *next;
    child_thread thread;
    char **argv;
    int argc;
    char *command;
    char *working_directory;
    char **environment;
    char **bindings;
    size_t binding_count;
    rxvm_byte_endpoint *input;
    rxvm_byte_endpoint *output;
    rxvm_byte_endpoint *error;
    int mode;
    unsigned char path_command_line;
    int64_t wait_microseconds;
    atomic_uchar cancelled;
    atomic_uchar input_stopped;
    atomic_uchar output_stopped;
    unsigned char thread_started;
    unsigned char thread_joined;
    unsigned char terminal;
    int64_t completion_state;
    int64_t error_code;
    char *message;
    uint64_t completion_order;
    unsigned char *result_node;
    size_t result_node_length;
    unsigned char *details_node;
    size_t details_node_length;
} child_request;

struct child_channel {
    rxvm_channel_byte_registry *byte_registry;
    child_request *requests;
    child_mutex mutex;
    child_condition changed;
    size_t capacity;
    size_t running;
    uint64_t completion_generation;
    uint64_t next_completion_order;
    unsigned char closed;
};

static uint16_t child_u16(const unsigned char *data) {
    return (uint16_t)data[0] | (uint16_t)((uint16_t)data[1] << 8u);
}

static uint32_t child_u32(const unsigned char *data) {
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8u) |
           ((uint32_t)data[2] << 16u) | ((uint32_t)data[3] << 24u);
}

static uint64_t child_u64(const unsigned char *data) {
    uint64_t result = 0u;
    unsigned int index;
    for (index = 0u; index < 8u; index++) {
        result |= (uint64_t)data[index] << (index * 8u);
    }
    return result;
}

static int64_t child_i64(const unsigned char *data) {
    uint64_t bits = child_u64(data);
    int64_t result;
    memcpy(&result, &bits, sizeof(result));
    return result;
}

static void child_put_u32(unsigned char *data, uint32_t value) {
    unsigned int index;
    for (index = 0u; index < 4u; index++) {
        data[index] = (unsigned char)(value >> (index * 8u));
    }
}

static void child_put_u64(unsigned char *data, uint64_t value) {
    unsigned int index;
    for (index = 0u; index < 8u; index++) {
        data[index] = (unsigned char)(value >> (index * 8u));
    }
}

static int child_node_open(const unsigned char *data,
                           size_t length,
                           child_node *node) {
    uint64_t payload_length;
    if (!data || !node || length < 12u || data[1] || child_u16(data + 2u)) {
        return 0;
    }
    payload_length = child_u64(data + 4u);
    if (payload_length > SIZE_MAX ||
        (size_t)payload_length > length - 12u) return 0;
    node->tag = data[0];
    node->payload = data + 12u;
    node->payload_length = (size_t)payload_length;
    node->total_length = 12u + node->payload_length;
    return 1;
}

static int child_document_root(const void *document,
                               size_t length,
                               child_node *root) {
    const unsigned char *data = (const unsigned char *)document;
    return data && length >= 28u && length <= CHILD_MAX_DOCUMENT &&
           !memcmp(data, "RXCV", 4u) && data[4] == 1u && !data[5] &&
           child_u64(data + 8u) == length &&
           child_node_open(data + 16u, length - 16u, root) &&
           root->total_length == length - 16u;
}

static int child_record_open(const child_node *node, child_record *record) {
    const unsigned char *cursor;
    size_t remaining;
    uint64_t schema_length;
    if (!node || !record || node->tag != 9u || node->payload_length < 20u) {
        return 0;
    }
    cursor = node->payload;
    remaining = node->payload_length;
    schema_length = child_u64(cursor);
    if (!schema_length || schema_length > SIZE_MAX ||
        (size_t)schema_length > remaining - 8u) return 0;
    record->schema = cursor + 8u;
    record->schema_length = (size_t)schema_length;
    cursor += 8u + record->schema_length;
    remaining -= 8u + record->schema_length;
    if (remaining < 12u) return 0;
    record->version = child_u32(cursor);
    record->field_count = child_u64(cursor + 4u);
    record->fields = cursor + 12u;
    record->fields_length = remaining - 12u;
    return 1;
}

static int child_record_schema(const child_record *record,
                               const char *schema,
                               uint64_t fields) {
    size_t length = strlen(schema);
    return record && record->schema_length == length &&
           !memcmp(record->schema, schema, length) &&
           record->version == 1u && record->field_count == fields;
}

static int child_record_field(const child_record *record,
                              const char *name,
                              child_node *node) {
    const unsigned char *cursor;
    size_t remaining;
    size_t expected = strlen(name);
    uint64_t index;
    if (!record || !node) return 0;
    cursor = record->fields;
    remaining = record->fields_length;
    for (index = 0u; index < record->field_count; index++) {
        uint64_t length;
        child_node candidate;
        if (remaining < 8u) return 0;
        length = child_u64(cursor);
        cursor += 8u;
        remaining -= 8u;
        if (length > SIZE_MAX || (size_t)length > remaining ||
            !child_node_open(cursor + (size_t)length,
                             remaining - (size_t)length,
                             &candidate)) return 0;
        if ((size_t)length == expected && !memcmp(cursor, name, expected)) {
            *node = candidate;
            return 1;
        }
        cursor += (size_t)length + candidate.total_length;
        remaining -= (size_t)length + candidate.total_length;
    }
    return 0;
}

static int child_node_integer(const child_node *node, int64_t *value) {
    if (!node || node->tag != 3u || node->payload_length != 8u) return 0;
    if (value) *value = child_i64(node->payload);
    return 1;
}

static char *child_node_string_copy(const child_node *node) {
    char *copy;
    if (!node || node->tag != 6u || node->payload_length == SIZE_MAX ||
        memchr(node->payload, '\0', node->payload_length)) return 0;
    copy = (char *)malloc(node->payload_length + 1u);
    if (!copy) return 0;
    memcpy(copy, node->payload, node->payload_length);
    copy[node->payload_length] = '\0';
    return copy;
}

static char *child_strdup(const char *text) {
    size_t length;
    char *copy;
    if (!text) return 0;
    length = strlen(text);
    copy = (char *)malloc(length + 1u);
    if (copy) memcpy(copy, text, length + 1u);
    return copy;
}

static int child_buffer_append(child_buffer *buffer,
                               const void *data,
                               size_t length) {
    size_t needed;
    size_t next;
    unsigned char *replacement;
    if (!buffer || (!data && length) || length > SIZE_MAX - buffer->length) {
        return 0;
    }
    needed = buffer->length + length;
    if (needed > buffer->capacity) {
        next = buffer->capacity ? buffer->capacity : 128u;
        while (next < needed) {
            if (next > SIZE_MAX / 2u) return 0;
            next *= 2u;
        }
        replacement = (unsigned char *)realloc(buffer->data, next);
        if (!replacement) return 0;
        buffer->data = replacement;
        buffer->capacity = next;
    }
    if (length) memcpy(buffer->data + buffer->length, data, length);
    buffer->length = needed;
    return 1;
}

static int child_buffer_u64(child_buffer *buffer, uint64_t value) {
    unsigned char encoded[8];
    child_put_u64(encoded, value);
    return child_buffer_append(buffer, encoded, sizeof(encoded));
}

static int child_buffer_integer_node(child_buffer *buffer, int64_t value) {
    unsigned char node[20] = {3u};
    uint64_t bits;
    memcpy(&bits, &value, sizeof(bits));
    child_put_u64(node + 4u, 8u);
    child_put_u64(node + 12u, bits);
    return child_buffer_append(buffer, node, sizeof(node));
}

static int child_buffer_boolean_node(child_buffer *buffer, int value) {
    unsigned char node[12] = {0};
    node[0] = value ? 2u : 1u;
    return child_buffer_append(buffer, node, sizeof(node));
}

static int child_buffer_field(child_buffer *buffer, const char *name) {
    size_t length = strlen(name);
    return child_buffer_u64(buffer, length) &&
           child_buffer_append(buffer, name, length);
}

static unsigned char *child_integer_node(int64_t value, size_t *length_out) {
    child_buffer buffer = {0};
    if (!child_buffer_integer_node(&buffer, value)) return 0;
    if (length_out) *length_out = buffer.length;
    return buffer.data;
}

static unsigned char *child_details_node(int exit_status,
                                         int launch_state,
                                         int stdin_state,
                                         int stdout_state,
                                         int stderr_state,
                                         size_t *length_out) {
    const char *schema = "crexx.channel.child-process-details";
    child_buffer payload = {0};
    child_buffer node = {0};
    unsigned char header[12] = {9u};
    if (!child_buffer_u64(&payload, strlen(schema)) ||
        !child_buffer_append(&payload, schema, strlen(schema)) ||
        !child_buffer_append(&payload, "\1\0\0\0", 4u) ||
        !child_buffer_u64(&payload, 7u) ||
        !child_buffer_field(&payload, "ambiguousOutcome") ||
        !child_buffer_boolean_node(&payload, 0) ||
        !child_buffer_field(&payload, "exitStatus") ||
        !child_buffer_integer_node(&payload, exit_status) ||
        !child_buffer_field(&payload, "launchState") ||
        !child_buffer_integer_node(&payload, launch_state) ||
        !child_buffer_field(&payload, "stderrState") ||
        !child_buffer_integer_node(&payload, stderr_state) ||
        !child_buffer_field(&payload, "stdinState") ||
        !child_buffer_integer_node(&payload, stdin_state) ||
        !child_buffer_field(&payload, "stdoutState") ||
        !child_buffer_integer_node(&payload, stdout_state) ||
        !child_buffer_field(&payload, "terminatingSignal") ||
        !child_buffer_integer_node(&payload, 0)) goto failed;
    child_put_u64(header + 4u, payload.length);
    if (!child_buffer_append(&node, header, sizeof(header)) ||
        !child_buffer_append(&node, payload.data, payload.length)) goto failed;
    free(payload.data);
    if (length_out) *length_out = node.length;
    return node.data;

failed:
    free(payload.data);
    free(node.data);
    return 0;
}

static void child_string_array_free(char **items) {
    size_t index;
    if (!items) return;
    for (index = 0u; items[index]; index++) free(items[index]);
    free(items);
}

static int child_environment_name_equal(const char *left,
                                        const char *right) {
    const char *left_equals;
    const char *right_equals;
    size_t left_length;
    size_t right_length;
    if (!left || !right) return 0;
    left_equals = strchr(left, '=');
    right_equals = strchr(right, '=');
    if (!left_equals || !right_equals) return 0;
    left_length = (size_t)(left_equals - left);
    right_length = (size_t)(right_equals - right);
    if (left_length != right_length) return 0;
#if defined(_WIN32)
    return _strnicmp(left, right, left_length) == 0;
#else
    return strncmp(left, right, left_length) == 0;
#endif
}

/* Apply the request record as an overlay to the controller's immutable
 * logical-process snapshot. The resulting vector is wholly request-owned and
 * may cross to an asynchronous child without retaining VM state. */
static int child_environment_merge(char ***environment_io,
                                   char **overrides) {
    char **environment;
    size_t count = 0u;
    size_t override_index;
    if (!environment_io) return 0;
    environment = *environment_io;
    if (environment) while (environment[count]) count++;
    if (!overrides) return 1;
    for (override_index = 0u; overrides[override_index]; override_index++) {
        char *copy;
        size_t index;
        size_t replacement = SIZE_MAX;
        for (index = 0u; index < count; index++) {
            if (child_environment_name_equal(
                    environment[index], overrides[override_index])) {
                replacement = index;
                break;
            }
        }
        copy = child_strdup(overrides[override_index]);
        if (!copy) return 0;
        if (replacement != SIZE_MAX) {
            free(environment[replacement]);
            environment[replacement] = copy;
        } else {
            char **resized = (char **)realloc(
                    environment, (count + 2u) * sizeof(*environment));
            if (!resized) {
                free(copy);
                return 0;
            }
            environment = resized;
            environment[count++] = copy;
            environment[count] = 0;
            *environment_io = environment;
        }
    }
    *environment_io = environment;
    return 1;
}

static int child_parse_arguments(const child_node *node,
                                 const char *executable,
                                 char ***argv_out,
                                 int *argc_out) {
    const unsigned char *cursor;
    size_t remaining;
    uint64_t count;
    uint64_t index;
    char **argv;
    if (argv_out) *argv_out = 0;
    if (argc_out) *argc_out = 0;
    if (!node || node->tag != 8u || node->payload_length < 8u ||
        !executable || !argv_out || !argc_out) return 0;
    count = child_u64(node->payload);
    if (count > INT_MAX - 1 || count > SIZE_MAX / sizeof(*argv) - 2u) {
        return 0;
    }
    argv = (char **)calloc((size_t)count + 2u, sizeof(*argv));
    if (!argv) return 0;
    argv[0] = child_strdup(executable);
    if (!argv[0]) goto failed;
    cursor = node->payload + 8u;
    remaining = node->payload_length - 8u;
    for (index = 0u; index < count; index++) {
        child_node item;
        if (!child_node_open(cursor, remaining, &item) || item.tag != 6u) {
            goto failed;
        }
        argv[(size_t)index + 1u] = child_node_string_copy(&item);
        if (!argv[(size_t)index + 1u]) goto failed;
        cursor += item.total_length;
        remaining -= item.total_length;
    }
    if (remaining) goto failed;
    *argv_out = argv;
    *argc_out = (int)count + 1;
    return 1;

failed:
    child_string_array_free(argv);
    return 0;
}

static int child_parse_string_array(const child_node *node,
                                    char ***items_out,
                                    size_t *count_out) {
    const unsigned char *cursor;
    size_t remaining;
    uint64_t count;
    uint64_t index;
    char **items;
    if (items_out) *items_out = 0;
    if (count_out) *count_out = 0u;
    if (!node || !items_out || !count_out || node->tag != 8u ||
        node->payload_length < 8u) return 0;
    count = child_u64(node->payload);
    if (count > SIZE_MAX / sizeof(*items) - 1u) return 0;
    items = (char **)calloc((size_t)count + 1u, sizeof(*items));
    if (!items) return 0;
    cursor = node->payload + 8u;
    remaining = node->payload_length - 8u;
    for (index = 0u; index < count; index++) {
        child_node item;
        if (!child_node_open(cursor, remaining, &item) || item.tag != 6u) {
            child_string_array_free(items);
            return 0;
        }
        items[index] = child_node_string_copy(&item);
        if (!items[index]) {
            child_string_array_free(items);
            return 0;
        }
        cursor += item.total_length;
        remaining -= item.total_length;
    }
    if (remaining) {
        child_string_array_free(items);
        return 0;
    }
    *items_out = items;
    *count_out = (size_t)count;
    return 1;
}

static int child_parse_environment(const child_node *node,
                                   char ***environment_out) {
    child_record record;
    const unsigned char *cursor;
    size_t remaining;
    uint64_t index;
    char **environment;
    if (environment_out) *environment_out = 0;
    if (!node || !environment_out || node->tag == 0u) return node && node->tag == 0u;
    if (!child_record_open(node, &record) ||
        !child_record_schema(&record, "crexx.channel.environment",
                             record.field_count) ||
        record.field_count > SIZE_MAX / sizeof(*environment) - 1u) return 0;
    environment = (char **)calloc(
            (size_t)record.field_count + 1u, sizeof(*environment));
    if (!environment) return 0;
    cursor = record.fields;
    remaining = record.fields_length;
    for (index = 0u; index < record.field_count; index++) {
        uint64_t name_length;
        child_node value;
        size_t total;
        if (remaining < 8u) goto failed;
        name_length = child_u64(cursor);
        cursor += 8u;
        remaining -= 8u;
        if (!name_length || name_length > SIZE_MAX ||
            (size_t)name_length > remaining ||
            memchr(cursor, '=', (size_t)name_length) ||
            memchr(cursor, '\0', (size_t)name_length) ||
            !child_node_open(cursor + (size_t)name_length,
                             remaining - (size_t)name_length, &value) ||
            value.tag != 6u ||
            memchr(value.payload, '\0', value.payload_length) ||
            (size_t)name_length > SIZE_MAX - value.payload_length - 2u) {
            goto failed;
        }
        total = (size_t)name_length + value.payload_length + 2u;
        environment[index] = (char *)malloc(total);
        if (!environment[index]) goto failed;
        memcpy(environment[index], cursor, (size_t)name_length);
        environment[index][name_length] = '=';
        memcpy(environment[index] + name_length + 1u,
               value.payload, value.payload_length);
        environment[index][total - 1u] = '\0';
        cursor += (size_t)name_length + value.total_length;
        remaining -= (size_t)name_length + value.total_length;
    }
    if (remaining) goto failed;
    *environment_out = environment;
    return 1;

failed:
    child_string_array_free(environment);
    return 0;
}

static rxvm_channel_status child_endpoint_from_node(
        child_channel *channel,
        const child_node *node,
        uint32_t required_right,
        rxvm_byte_endpoint **endpoint_out) {
    if (!node || !endpoint_out) return RXVM_CHANNEL_INVALID_CONFIGURATION;
    if (node->tag == 0u) {
        *endpoint_out = 0;
        return RXVM_CHANNEL_OK;
    }
    return rxvm_channel_byte_reference_retain(
            channel->byte_registry, node->payload - 12u,
            node->total_length, required_right, endpoint_out);
}

static void child_request_publish(child_request *request,
                                  int spawn_status,
                                  int exit_status,
                                  int termination_reason,
                                  int stdin_state,
                                  int stdout_state,
                                  int stderr_state,
                                  char *message) {
    child_channel *channel = request->owner;
    int cancelled;
    if (termination_reason == 0 && spawn_status == SHELLSPAWN_OK) {
        request->result_node = child_integer_node(
                exit_status, &request->result_node_length);
    }
    request->details_node = child_details_node(
            exit_status, spawn_status == SHELLSPAWN_OK ? 2 : 3,
            stdin_state, stdout_state, stderr_state,
            &request->details_node_length);
    if ((termination_reason == 0 && spawn_status == SHELLSPAWN_OK &&
         !request->result_node) ||
        !request->details_node) {
        spawn_status = SHELLSPAWN_FAILURE;
        free(request->result_node);
        free(request->details_node);
        request->result_node = 0;
        request->details_node = 0;
        request->result_node_length = 0u;
        request->details_node_length = 0u;
    }
    child_mutex_lock(&channel->mutex);
    if (request->terminal || !channel->running) abort();
    /* Cancellation and publication share this lock: first one wins. */
    cancelled = atomic_load_explicit(
            &request->cancelled, memory_order_acquire) != 0;
    if (cancelled) {
        free(request->result_node);
        request->result_node = 0;
        request->result_node_length = 0u;
    }
    request->message = message;
    if (cancelled || termination_reason == 1) {
        request->completion_state = 3;
        request->error_code = 0;
    } else if (termination_reason == 2) {
        request->completion_state = 4;
        request->error_code = RXVM_CHANNEL_TIMEOUT;
    } else if (spawn_status == SHELLSPAWN_OK) {
        request->completion_state = 1;
        request->error_code = 0;
    } else {
        request->completion_state = 2;
        request->error_code = spawn_status == SHELLSPAWN_NOFOUND
                ? RXVM_CHANNEL_PROVIDER_UNAVAILABLE
                : RXVM_CHANNEL_PROVIDER_FAILURE;
    }
    request->completion_order = ++channel->next_completion_order;
    request->terminal = 1u;
    channel->running--;
    channel->completion_generation++;
    child_condition_broadcast(&channel->changed);
    child_mutex_unlock(&channel->mutex);
}

static void child_request_execute(child_request *request) {
    REDIRECT *input_redirect = 0;
    REDIRECT *output_redirect = 0;
    REDIRECT *error_redirect = 0;
    int spawn_status = SHELLSPAWN_FAILURE;
    int exit_status = 0;
    int stdin_state = 1;
    int stdout_state = 1;
    int stderr_state = 1;
    int termination_reason = 0;
    char *message = 0;
    if (request->input) input_redirect = rxspawn_redirect_from_byte_endpoint(
            request->input, &request->input_stopped);
    if (request->output) output_redirect = rxspawn_redirect_to_byte_endpoint(
            request->output, &request->output_stopped);
    if (request->error) error_redirect = rxspawn_redirect_to_byte_endpoint(
            request->error, &request->output_stopped);
    if ((!request->input || input_redirect) &&
        (!request->output || output_redirect) &&
        (!request->error || error_redirect)) {
        if (request->mode == SHELLSPAWN_MODE_PATH) {
            if (request->path_command_line) {
                spawn_status = shellspawn_snapshot(
                        request->command,
                        input_redirect, output_redirect, error_redirect,
                        request->working_directory,
                        (const char *const *)request->environment,
                        request->mode, request->wait_microseconds,
                        &request->cancelled, &request->input_stopped,
                        &request->output_stopped,
                        &termination_reason,
                        &exit_status, &message);
            } else {
                spawn_status = shellspawn_argv_snapshot(
                        (const char *const *)request->argv, request->argc,
                        input_redirect, output_redirect, error_redirect,
                        request->working_directory,
                        (const char *const *)request->environment,
                        request->wait_microseconds, &request->cancelled,
                        &request->input_stopped, &request->output_stopped,
                        &termination_reason,
                        &exit_status, &message);
            }
        } else if (request->mode == SHELLSPAWN_MODE_SHELL ||
                   request->mode == SHELLSPAWN_MODE_CONFIGURED_SHELL) {
            spawn_status = shellspawn_snapshot(
                    request->command, input_redirect, output_redirect,
                    error_redirect, request->working_directory,
                    (const char *const *)request->environment,
                    request->mode, request->wait_microseconds,
                    &request->cancelled, &request->input_stopped,
                    &request->output_stopped,
                    &termination_reason,
                    &exit_status, &message);
        } else if (request->mode == SHELLSPAWN_MODE_CREXX) {
            spawn_status = shellspawn_snapshot_bindings(
                    request->command,
                    input_redirect, output_redirect, error_redirect,
                    request->working_directory,
                    (const char *const *)request->environment,
                    (const char *const *)request->bindings,
                    request->binding_count,
                    request->mode, request->wait_microseconds,
                    &request->cancelled, &request->input_stopped,
                    &request->output_stopped,
                    &termination_reason,
                    &exit_status, &message);
        }
    } else {
        message = child_strdup("child redirect adapter allocation failed");
    }
    atomic_store_explicit(
            &request->input_stopped, 1u, memory_order_release);
    atomic_store_explicit(
            &request->output_stopped, 1u, memory_order_release);
    if (request->input) rxvm_byte_endpoint_wake(request->input);
    if (request->output) rxvm_byte_endpoint_wake(request->output);
    if (request->error) rxvm_byte_endpoint_wake(request->error);
    if (rxspawn_redirect_byte_endpoint_destroy(input_redirect) != 0) {
        stdin_state = 2;
    }
    if (request->input) (void)rxvm_byte_endpoint_half_close(
            request->input, RXVM_BYTE_ENDPOINT_READ);
    if (rxspawn_redirect_byte_endpoint_destroy(output_redirect) != 0) {
        stdout_state = 2;
    }
    if (rxspawn_redirect_byte_endpoint_destroy(error_redirect) != 0) {
        stderr_state = 2;
    }
    child_request_publish(request, spawn_status, exit_status,
                          termination_reason,
                          stdin_state, stdout_state, stderr_state, message);
}

static CHILD_THREAD_RETURN child_request_run(void *opaque) {
    child_request_execute((child_request *)opaque);
#if defined(_WIN32)
    return 0u;
#else
    return 0;
#endif
}

static int child_thread_start(child_request *request) {
#if defined(_WIN32)
    request->thread = CreateThread(0, 0, child_request_run, request, 0, 0);
    return request->thread != 0;
#else
    return pthread_create(&request->thread, 0, child_request_run, request) == 0;
#endif
}

static int child_thread_join(child_request *request) {
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

static rxvm_channel_status child_channel_open(
        void *module_state,
        struct rxvm_context *context,
        const void *configuration,
        size_t configuration_length,
        void **channel_state_out) {
    child_node root;
    child_record record;
    child_node capacity_node;
    int64_t capacity;
    child_channel *channel;
    if (channel_state_out) *channel_state_out = 0;
    if (!module_state || !context || !channel_state_out ||
        !child_document_root(configuration, configuration_length, &root) ||
        !child_record_open(&root, &record) ||
        !child_record_schema(
                &record, "crexx.channel.child-process-provider", 1u) ||
        !child_record_field(&record, "capacity", &capacity_node) ||
        !child_node_integer(&capacity_node, &capacity) ||
        capacity < 1 || capacity > 65535) {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    channel = (child_channel *)calloc(1u, sizeof(*channel));
    if (!channel) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    if (!child_mutex_init(&channel->mutex)) {
        free(channel);
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    if (!child_condition_init(&channel->changed)) {
        child_mutex_destroy(&channel->mutex);
        free(channel);
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    channel->byte_registry = (rxvm_channel_byte_registry *)module_state;
    channel->capacity = (size_t)capacity;
    *channel_state_out = channel;
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status child_channel_start(
        void *channel_state,
        const void *envelope,
        size_t envelope_length,
        int64_t wait_microseconds,
        void **request_state_out) {
    child_channel *channel = (child_channel *)channel_state;
    child_request *request;
    child_node root;
    child_record record;
    child_node arguments_node;
    child_node bindings_node;
    child_node environment_node;
    child_node executable_node;
    child_node mode_node;
    child_node stderr_node;
    child_node stdin_node;
    child_node stdout_node;
    child_node working_node;
    int64_t mode;
    char *executable = 0;
    char *active_working_directory = 0;
    char **active_environment = 0;
    char **environment_overrides = 0;
    rxvm_channel_status status;
    if (request_state_out) *request_state_out = 0;
    if (!channel || !request_state_out || wait_microseconds < -1 ||
        !child_document_root(envelope, envelope_length, &root) ||
        !child_record_open(&root, &record) ||
        !child_record_schema(
                &record, "crexx.channel.child-process-start", 9u) ||
        !child_record_field(&record, "arguments", &arguments_node) ||
        !child_record_field(&record, "bindings", &bindings_node) ||
        !child_record_field(&record, "environment", &environment_node) ||
        !child_record_field(&record, "executable", &executable_node) ||
        !child_record_field(&record, "mode", &mode_node) ||
        !child_record_field(&record, "stderr", &stderr_node) ||
        !child_record_field(&record, "stdin", &stdin_node) ||
        !child_record_field(&record, "stdout", &stdout_node) ||
        !child_record_field(&record, "workingDirectory", &working_node) ||
        !child_node_integer(&mode_node, &mode) || mode < 0 || mode > 3) {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    executable = child_node_string_copy(&executable_node);
    if (!executable || (!*executable && mode != SHELLSPAWN_MODE_CREXX)) {
        free(executable);
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    request = (child_request *)calloc(1u, sizeof(*request));
    if (!request) {
        free(executable);
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    request->owner = channel;
    request->mode = (int)mode;
    request->wait_microseconds = wait_microseconds;
    atomic_init(&request->cancelled, 0u);
    atomic_init(&request->input_stopped, 0u);
    atomic_init(&request->output_stopped, 0u);
    if (!child_parse_string_array(
            &bindings_node, &request->bindings,
            &request->binding_count)) goto invalid;
    if (mode != SHELLSPAWN_MODE_CREXX && request->binding_count) goto invalid;
    if (mode == SHELLSPAWN_MODE_PATH) {
        uint64_t argument_count = arguments_node.tag == 8u &&
                                  arguments_node.payload_length >= 8u
                ? child_u64(arguments_node.payload) : UINT64_MAX;
        if (argument_count == 0u) {
            request->command = child_strdup(executable);
            if (!request->command) goto resource;
            request->path_command_line = 1u;
        } else if (!child_parse_arguments(
                       &arguments_node, executable, &request->argv,
                       &request->argc)) goto invalid;
    } else {
        uint64_t argument_count = arguments_node.tag == 8u &&
                                  arguments_node.payload_length >= 8u
                ? child_u64(arguments_node.payload) : UINT64_MAX;
        if (argument_count) goto invalid;
        request->command = child_strdup(executable);
        if (!request->command) goto resource;
    }
    if (!child_parse_environment(
            &environment_node, &environment_overrides)) goto invalid;
    if (rxcrexxcmd_active_process_snapshot(
            &active_working_directory, &active_environment) != 0) {
        goto resource;
    }
    if (!child_environment_merge(
            &active_environment, environment_overrides)) goto resource;
    child_string_array_free(environment_overrides);
    environment_overrides = 0;
    request->environment = active_environment;
    active_environment = 0;
    if (working_node.tag == 6u) {
        request->working_directory = child_node_string_copy(&working_node);
        if (!request->working_directory) goto resource;
        free(active_working_directory);
        active_working_directory = 0;
    } else if (working_node.tag != 0u) goto invalid;
    else {
        request->working_directory = active_working_directory;
        active_working_directory = 0;
    }
    status = child_endpoint_from_node(
            channel, &stdin_node, RXVM_BYTE_ENDPOINT_READ, &request->input);
    if (status != RXVM_CHANNEL_OK) goto status_failure;
    status = child_endpoint_from_node(
            channel, &stdout_node, RXVM_BYTE_ENDPOINT_WRITE, &request->output);
    if (status != RXVM_CHANNEL_OK) goto status_failure;
    status = child_endpoint_from_node(
            channel, &stderr_node, RXVM_BYTE_ENDPOINT_WRITE, &request->error);
    if (status != RXVM_CHANNEL_OK) goto status_failure;
    child_mutex_lock(&channel->mutex);
    if (channel->closed) {
        child_mutex_unlock(&channel->mutex);
        status = RXVM_CHANNEL_CLOSED;
        goto status_failure;
    }
    if (channel->running >= channel->capacity) {
        child_mutex_unlock(&channel->mutex);
        status = RXVM_CHANNEL_BACKPRESSURE;
        goto status_failure;
    }
    request->next = channel->requests;
    channel->requests = request;
    channel->running++;
    child_mutex_unlock(&channel->mutex);
    if (mode == SHELLSPAWN_MODE_CREXX) {
        free(executable);
        *request_state_out = request;
        child_request_execute(request);
        return RXVM_CHANNEL_OK;
    }
    if (!child_thread_start(request)) {
        child_request **cursor;
        child_mutex_lock(&channel->mutex);
        cursor = &channel->requests;
        while (*cursor && *cursor != request) cursor = &(*cursor)->next;
        if (*cursor == request) *cursor = request->next;
        channel->running--;
        child_mutex_unlock(&channel->mutex);
        status = RXVM_CHANNEL_RESOURCE_EXHAUSTED;
        goto status_failure;
    }
    request->thread_started = 1u;
    free(executable);
    *request_state_out = request;
    return RXVM_CHANNEL_OK;

resource:
    status = RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    goto status_failure;
invalid:
    status = RXVM_CHANNEL_INVALID_CONFIGURATION;
status_failure:
    free(executable);
    free(active_working_directory);
    child_string_array_free(active_environment);
    child_string_array_free(environment_overrides);
    child_string_array_free(request->argv);
    child_string_array_free(request->environment);
    child_string_array_free(request->bindings);
    free(request->command);
    free(request->working_directory);
    rxvm_byte_endpoint_release(request->input);
    rxvm_byte_endpoint_release(request->output);
    rxvm_byte_endpoint_release(request->error);
    free(request);
    return status;
}

static int child_channel_terminal_snapshot(
        void *channel_state,
        void *request_state,
        rxvm_channel_provider_completion *completion_out,
        uint64_t *completion_order_out) {
    child_channel *channel = (child_channel *)channel_state;
    child_request *request = (child_request *)request_state;
    int terminal;
    if (completion_out) memset(completion_out, 0, sizeof(*completion_out));
    if (completion_order_out) *completion_order_out = 0u;
    if (!channel || !request || request->owner != channel) return -1;
    child_mutex_lock(&channel->mutex);
    terminal = request->terminal;
    if (terminal && completion_out) {
        completion_out->state = request->completion_state;
        completion_out->error_code = request->error_code;
        completion_out->message = request->message ? request->message : "";
        completion_out->result_node = request->result_node;
        completion_out->result_node_length = request->result_node_length;
        completion_out->details_node = request->details_node;
        completion_out->details_node_length = request->details_node_length;
    }
    if (terminal && completion_order_out) {
        *completion_order_out = request->completion_order;
    }
    child_mutex_unlock(&channel->mutex);
    return terminal;
}

static uint64_t child_channel_completion_generation(void *channel_state) {
    child_channel *channel = (child_channel *)channel_state;
    uint64_t generation;
    if (!channel) return 0u;
    child_mutex_lock(&channel->mutex);
    generation = channel->completion_generation;
    child_mutex_unlock(&channel->mutex);
    return generation;
}

static int child_channel_completion_wait(void *channel_state,
                                         uint64_t observed_generation,
                                         int64_t wait_microseconds) {
    child_channel *channel = (child_channel *)channel_state;
    int result = 0;
    if (!channel || wait_microseconds < -1) return -1;
    child_mutex_lock(&channel->mutex);
    if (channel->completion_generation != observed_generation) result = 1;
    else if (wait_microseconds != 0) {
        int waited = child_condition_wait(
                &channel->changed, &channel->mutex, wait_microseconds);
        if (waited < 0) result = -1;
        else if (channel->completion_generation != observed_generation) {
            result = 1;
        }
    }
    child_mutex_unlock(&channel->mutex);
    return result;
}

static rxvm_channel_status child_channel_cancel(
        void *channel_state,
        void *request_state,
        const void *reason,
        size_t reason_length) {
    child_channel *channel = (child_channel *)channel_state;
    child_request *request = (child_request *)request_state;
    (void)reason;
    (void)reason_length;
    if (!channel || !request || request->owner != channel) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    child_mutex_lock(&channel->mutex);
    if (request->terminal) {
        child_mutex_unlock(&channel->mutex);
        return RXVM_CHANNEL_ALREADY_TERMINAL;
    }
    atomic_store_explicit(&request->cancelled, 1u, memory_order_release);
    atomic_store_explicit(
            &request->input_stopped, 1u, memory_order_release);
    atomic_store_explicit(
            &request->output_stopped, 1u, memory_order_release);
    child_mutex_unlock(&channel->mutex);
    if (request->input) rxvm_byte_endpoint_wake(request->input);
    if (request->output) rxvm_byte_endpoint_wake(request->output);
    if (request->error) rxvm_byte_endpoint_wake(request->error);
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status child_channel_close(void *channel_state,
                                               int64_t mode) {
    child_channel *channel = (child_channel *)channel_state;
    child_request *request;
    if (!channel || (mode != 1 && mode != 2)) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    child_mutex_lock(&channel->mutex);
    if (channel->closed) {
        child_mutex_unlock(&channel->mutex);
        return RXVM_CHANNEL_CLOSED;
    }
    channel->closed = 1u;
    if (mode == 2) {
        for (request = channel->requests; request; request = request->next) {
            if (request->terminal) continue;
            atomic_store_explicit(
                    &request->cancelled, 1u, memory_order_release);
            atomic_store_explicit(
                    &request->input_stopped, 1u, memory_order_release);
            atomic_store_explicit(
                    &request->output_stopped, 1u, memory_order_release);
            if (request->input) rxvm_byte_endpoint_wake(request->input);
            if (request->output) rxvm_byte_endpoint_wake(request->output);
            if (request->error) rxvm_byte_endpoint_wake(request->error);
        }
    }
    child_mutex_unlock(&channel->mutex);
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status child_channel_request_destroy(
        void *channel_state,
        void *request_state) {
    child_channel *channel = (child_channel *)channel_state;
    child_request *request = (child_request *)request_state;
    child_request **cursor;
    if (!channel || !request || request->owner != channel ||
        !child_thread_join(request)) return RXVM_CHANNEL_INTERNAL_ERROR;
    child_mutex_lock(&channel->mutex);
    cursor = &channel->requests;
    while (*cursor && *cursor != request) cursor = &(*cursor)->next;
    if (*cursor != request) {
        child_mutex_unlock(&channel->mutex);
        return RXVM_CHANNEL_INTERNAL_ERROR;
    }
    *cursor = request->next;
    child_mutex_unlock(&channel->mutex);
    child_string_array_free(request->argv);
    child_string_array_free(request->environment);
    child_string_array_free(request->bindings);
    free(request->command);
    free(request->working_directory);
    free(request->message);
    free(request->result_node);
    free(request->details_node);
    rxvm_byte_endpoint_release(request->input);
    rxvm_byte_endpoint_release(request->output);
    rxvm_byte_endpoint_release(request->error);
    free(request);
    return RXVM_CHANNEL_OK;
}

static void child_channel_destroy(void *channel_state) {
    child_channel *channel = (child_channel *)channel_state;
    if (!channel) return;
    if (channel->requests || channel->running) abort();
    child_condition_destroy(&channel->changed);
    child_mutex_destroy(&channel->mutex);
    free(channel);
}

void rxvm_channel_child_provider_descriptor(
        rxvm_channel_byte_registry *byte_registry,
        rxvm_channel_provider_descriptor *descriptor) {
    if (!descriptor) return;
    memset(descriptor, 0, sizeof(*descriptor));
    descriptor->type = RXVM_CHANNEL_PROVIDER_CHILD_PROCESS;
    descriptor->name = "crexx.core.child-process";
    descriptor->abi_version = RXVM_CHANNEL_PROVIDER_ABI_VERSION;
    descriptor->configuration_version_min = 1u;
    descriptor->configuration_version_max = 1u;
    descriptor->capabilities = CHILD_CAPABILITIES;
    descriptor->module_state = byte_registry;
    descriptor->operations.open = child_channel_open;
    descriptor->operations.start = child_channel_start;
    descriptor->operations.terminal_snapshot =
            child_channel_terminal_snapshot;
    descriptor->operations.completion_generation =
            child_channel_completion_generation;
    descriptor->operations.completion_wait = child_channel_completion_wait;
    descriptor->operations.cancel = child_channel_cancel;
    descriptor->operations.close = child_channel_close;
    descriptor->operations.request_destroy = child_channel_request_destroy;
    descriptor->operations.channel_destroy = child_channel_destroy;
}
