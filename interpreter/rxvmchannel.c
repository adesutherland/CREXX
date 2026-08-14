/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmchannel.h"

#include "rxvmexecutor.h"
#include "rxvmintp.h"
#include "rxvmprogram.h"
#include "rxvmworker.h"
#include "utf.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
typedef CRITICAL_SECTION rxvm_channel_mutex;
static int channel_mutex_init(rxvm_channel_mutex *mutex) {
    InitializeCriticalSection(mutex);
    return 1;
}
static void channel_mutex_destroy(rxvm_channel_mutex *mutex) {
    DeleteCriticalSection(mutex);
}
static void channel_mutex_lock(rxvm_channel_mutex *mutex) {
    EnterCriticalSection(mutex);
}
static void channel_mutex_unlock(rxvm_channel_mutex *mutex) {
    LeaveCriticalSection(mutex);
}
#else
#include <pthread.h>
typedef pthread_mutex_t rxvm_channel_mutex;
static int channel_mutex_init(rxvm_channel_mutex *mutex) {
    return pthread_mutex_init(mutex, 0) == 0;
}
static void channel_mutex_destroy(rxvm_channel_mutex *mutex) {
    if (pthread_mutex_destroy(mutex) != 0) abort();
}
static void channel_mutex_lock(rxvm_channel_mutex *mutex) {
    if (pthread_mutex_lock(mutex) != 0) abort();
}
static void channel_mutex_unlock(rxvm_channel_mutex *mutex) {
    if (pthread_mutex_unlock(mutex) != 0) abort();
}
#endif

#define RXVM_CHANNEL_MAX_DOCUMENT (16u * 1024u * 1024u)
#define RXVM_CHANNEL_MAX_MEMBERS 1048576u
#define RXVM_CHANNEL_MAX_DEPTH 64u
#define RXVM_CHANNEL_MAX_SLOTS 65535u

#define RXVM_CHANNEL_PROVIDER_LOCAL 1
#define RXVM_CHANNEL_CAP_ADMISSION UINT64_C(0x0001)
#define RXVM_CHANNEL_CAP_CANCELLATION UINT64_C(0x0002)
#define RXVM_CHANNEL_CAP_COMPLETION_ORDER UINT64_C(0x0008)
#define RXVM_CHANNEL_LOCAL_CAPABILITIES \
    (RXVM_CHANNEL_CAP_ADMISSION | RXVM_CHANNEL_CAP_CANCELLATION | \
     RXVM_CHANNEL_CAP_COMPLETION_ORDER)

#define RXCV_TAG_NULL 0u
#define RXCV_TAG_FALSE 1u
#define RXCV_TAG_TRUE 2u
#define RXCV_TAG_INTEGER 3u
#define RXCV_TAG_FLOAT 4u
#define RXCV_TAG_DECIMAL 5u
#define RXCV_TAG_STRING 6u
#define RXCV_TAG_BINARY 7u
#define RXCV_TAG_ARRAY 8u
#define RXCV_TAG_RECORD 9u
#define RXCV_TAG_PROVIDER_REFERENCE 10u
#define RXCV_TAG_LOCAL_CAPABILITY 11u

typedef struct rxvm_channel_provider_descriptor {
    int64_t type;
    const char *name;
    uint32_t abi_version;
    uint64_t capabilities;
} rxvm_channel_provider_descriptor;

typedef struct rxvm_channel_runtime {
    rxvm_runtime *runtime;
    rxvm_channel_provider_descriptor local_provider;
    size_t live_executions;
    rxvm_channel_mutex mutex;
} rxvm_channel_runtime;

typedef enum rxvm_channel_lifecycle {
    RXVM_CHANNEL_SLOT_OPEN = 1,
    RXVM_CHANNEL_SLOT_CLOSING_DRAIN = 2,
    RXVM_CHANNEL_SLOT_CLOSING_CANCEL = 3
} rxvm_channel_lifecycle;

typedef struct rxvm_channel_slot {
    rxvm_executor *executor;
    const rxvm_channel_provider_descriptor *provider;
    size_t admission_capacity;
    size_t worker_count;
    size_t next_worker;
    uint64_t next_submission_sequence;
    uint16_t generation;
    rxvm_channel_lifecycle state;
    unsigned char live;
    unsigned char retired;
} rxvm_channel_slot;

typedef struct rxvm_channel_ticket_slot {
    rxvm_executor_request *request;
    uint64_t submission_sequence;
    int64_t capability;
    uint16_t generation;
    uint16_t channel_slot;
    unsigned char live;
    unsigned char observed;
    unsigned char retired;
} rxvm_channel_ticket_slot;

typedef struct rxvm_channel_context {
    rxvm_channel_runtime *runtime_state;
    rxvm_channel_slot *channels;
    rxvm_channel_ticket_slot *tickets;
    size_t channel_count;
    size_t channel_capacity;
    size_t ticket_count;
    size_t ticket_capacity;
    size_t live_channels;
    size_t live_tickets;
    uint32_t owner_id;
} rxvm_channel_context;

typedef struct rxcv_node {
    uint8_t tag;
    const unsigned char *payload;
    size_t payload_length;
    size_t total_length;
} rxcv_node;

typedef struct rxcv_record {
    const unsigned char *schema;
    size_t schema_length;
    uint32_t version;
    uint64_t field_count;
    const unsigned char *fields;
    size_t fields_length;
} rxcv_record;

typedef struct rxcv_buffer {
    unsigned char *data;
    size_t length;
    size_t capacity;
} rxcv_buffer;

typedef struct rxvm_channel_invoke {
    uint64_t callable_id;
    rxvm_executor_register_image *arguments;
    size_t argument_count;
} rxvm_channel_invoke;

static volatile uint32_t rxvm_channel_next_owner_id;

static uint16_t rxcv_u16(const unsigned char *data) {
    return (uint16_t)data[0] | (uint16_t)((uint16_t)data[1] << 8u);
}

static uint32_t rxcv_u32(const unsigned char *data) {
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8u) |
           ((uint32_t)data[2] << 16u) |
           ((uint32_t)data[3] << 24u);
}

static uint64_t rxcv_u64(const unsigned char *data) {
    uint64_t value = 0u;
    unsigned int i;
    for (i = 0u; i < 8u; i++) value |= (uint64_t)data[i] << (i * 8u);
    return value;
}

static int64_t rxcv_i64(const unsigned char *data) {
    uint64_t bits = rxcv_u64(data);
    int64_t value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

static void rxcv_put_u16(unsigned char *data, uint16_t value) {
    data[0] = (unsigned char)(value & 0xffu);
    data[1] = (unsigned char)((value >> 8u) & 0xffu);
}

static void rxcv_put_u32(unsigned char *data, uint32_t value) {
    unsigned int i;
    for (i = 0u; i < 4u; i++) {
        data[i] = (unsigned char)((value >> (i * 8u)) & 0xffu);
    }
}

static void rxcv_put_u64(unsigned char *data, uint64_t value) {
    unsigned int i;
    for (i = 0u; i < 8u; i++) {
        data[i] = (unsigned char)((value >> (i * 8u)) & 0xffu);
    }
}

static int rxcv_parse_node(const unsigned char *data,
                           size_t length,
                           rxcv_node *node) {
    uint64_t payload_length;

    if (!data || !node || length < 12u || data[1] || rxcv_u16(data + 2u)) {
        return 0;
    }
    payload_length = rxcv_u64(data + 4u);
    if (payload_length > SIZE_MAX || (size_t)payload_length > length - 12u) {
        return 0;
    }
    node->tag = data[0];
    node->payload = data + 12u;
    node->payload_length = (size_t)payload_length;
    node->total_length = 12u + (size_t)payload_length;
    return 1;
}

static int rxcv_name_compare(const unsigned char *left,
                             size_t left_length,
                             const unsigned char *right,
                             size_t right_length) {
    size_t common = left_length < right_length ? left_length : right_length;
    int comparison = common ? memcmp(left, right, common) : 0;
    if (comparison) return comparison;
    if (left_length < right_length) return -1;
    if (left_length > right_length) return 1;
    return 0;
}

static int rxcv_validate_node(const unsigned char *data,
                              size_t length,
                              unsigned int depth,
                              uint16_t *computed_flags,
                              uint64_t *member_total,
                              size_t *consumed_out) {
    rxcv_node node;
    const unsigned char *cursor;
    size_t remaining;
    uint64_t count;
    uint64_t index;

    if (consumed_out) *consumed_out = 0u;
    if (depth > RXVM_CHANNEL_MAX_DEPTH ||
        !rxcv_parse_node(data, length, &node)) return 0;
    cursor = node.payload;
    remaining = node.payload_length;
    switch (node.tag) {
        case RXCV_TAG_NULL:
        case RXCV_TAG_FALSE:
        case RXCV_TAG_TRUE:
            if (remaining) return 0;
            break;
        case RXCV_TAG_INTEGER:
            if (remaining != 8u) return 0;
            break;
        case RXCV_TAG_FLOAT:
            if (remaining != 8u) return 0;
            {
                uint64_t bits = rxcv_u64(cursor);
                if ((bits & UINT64_C(0x7ff0000000000000)) ==
                        UINT64_C(0x7ff0000000000000) &&
                    (bits & UINT64_C(0x000fffffffffffff)) &&
                    bits != UINT64_C(0x7ff8000000000000)) return 0;
            }
            break;
        case RXCV_TAG_DECIMAL:
            if (remaining < 13u) return 0;
            count = rxcv_u64(cursor + 5u);
            if (count > SIZE_MAX || (size_t)count != remaining - 13u ||
                cursor[0] > 1u || !count) return 0;
            for (index = 0u; index < count; index++) {
                if (cursor[13u + (size_t)index] > 9u) return 0;
            }
            if (count == 1u && cursor[13] == 0u) {
                if (cursor[0] || rxcv_u32(cursor + 1u)) return 0;
            } else if (cursor[13] == 0u ||
                       cursor[12u + (size_t)count] == 0u) {
                return 0;
            }
            break;
        case RXCV_TAG_STRING:
            if (remaining && utf8nvalid(cursor, remaining)) return 0;
            break;
        case RXCV_TAG_BINARY:
            break;
        case RXCV_TAG_ARRAY:
            if (remaining < 8u) return 0;
            count = rxcv_u64(cursor);
            cursor += 8u;
            remaining -= 8u;
            if (count > RXVM_CHANNEL_MAX_MEMBERS ||
                *member_total > RXVM_CHANNEL_MAX_MEMBERS - count) return 0;
            *member_total += count;
            for (index = 0u; index < count; index++) {
                size_t consumed;
                if (!rxcv_validate_node(cursor, remaining, depth + 1u,
                                        computed_flags, member_total,
                                        &consumed)) return 0;
                cursor += consumed;
                remaining -= consumed;
            }
            if (remaining) return 0;
            break;
        case RXCV_TAG_RECORD:
            if (remaining < 20u) return 0;
            count = rxcv_u64(cursor);
            if (!count || count > remaining - 8u ||
                utf8nvalid(cursor + 8u, (size_t)count)) return 0;
            cursor += 8u + (size_t)count;
            remaining -= 8u + (size_t)count;
            if (remaining < 12u) return 0;
            cursor += 4u;
            remaining -= 4u;
            count = rxcv_u64(cursor);
            cursor += 8u;
            remaining -= 8u;
            if (count > RXVM_CHANNEL_MAX_MEMBERS ||
                *member_total > RXVM_CHANNEL_MAX_MEMBERS - count) return 0;
            *member_total += count;
            {
                const unsigned char *previous = 0;
                size_t previous_length = 0u;
                for (index = 0u; index < count; index++) {
                    uint64_t name_length;
                    const unsigned char *name;
                    size_t consumed;
                    if (remaining < 8u) return 0;
                    name_length = rxcv_u64(cursor);
                    cursor += 8u;
                    remaining -= 8u;
                    if (!name_length || name_length > SIZE_MAX ||
                        (size_t)name_length > remaining) return 0;
                    name = cursor;
                    if (utf8nvalid(name, (size_t)name_length) ||
                        (previous && rxcv_name_compare(
                            previous, previous_length, name,
                            (size_t)name_length) >= 0)) return 0;
                    cursor += (size_t)name_length;
                    remaining -= (size_t)name_length;
                    if (!rxcv_validate_node(cursor, remaining, depth + 1u,
                                            computed_flags, member_total,
                                            &consumed)) return 0;
                    cursor += consumed;
                    remaining -= consumed;
                    previous = name;
                    previous_length = (size_t)name_length;
                }
            }
            if (remaining) return 0;
            break;
        case RXCV_TAG_PROVIDER_REFERENCE:
            if (remaining < 32u || cursor[16] < 1u || cursor[16] > 3u ||
                memcmp(cursor + 17u, "\0\0\0\0\0\0\0", 7u) != 0) {
                return 0;
            }
            count = rxcv_u64(cursor + 24u);
            if (count > SIZE_MAX || (size_t)count != remaining - 32u) return 0;
            *computed_flags |= 1u;
            break;
        case RXCV_TAG_LOCAL_CAPABILITY:
            if (remaining != 24u || cursor[8] > 1u ||
                memcmp(cursor + 9u, "\0\0\0\0\0\0\0", 7u) != 0) {
                return 0;
            }
            *computed_flags |= 2u;
            break;
        default:
            return 0;
    }
    if (consumed_out) *consumed_out = node.total_length;
    return 1;
}

static rxvm_channel_status rxcv_document_root(
        const void *document,
        size_t length,
        rxcv_node *root) {
    const unsigned char *data = (const unsigned char *)document;
    uint16_t computed_flags = 0u;
    uint64_t members = 0u;
    uint64_t declared_length;
    size_t consumed;

    if (!data || length < 16u || length > RXVM_CHANNEL_MAX_DOCUMENT ||
        memcmp(data, "RXCV", 4u) != 0) {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    if (data[4] != 1u || data[5] != 0u) {
        return RXVM_CHANNEL_INCOMPATIBLE_VERSION;
    }
    declared_length = rxcv_u64(data + 8u);
    if (declared_length != length ||
        !rxcv_validate_node(data + 16u, length - 16u, 1u,
                            &computed_flags, &members, &consumed) ||
        consumed != length - 16u ||
        rxcv_u16(data + 6u) != computed_flags ||
        !rxcv_parse_node(data + 16u, length - 16u, root)) {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    return RXVM_CHANNEL_OK;
}

static int rxcv_record_open(const rxcv_node *node, rxcv_record *record) {
    const unsigned char *cursor;
    size_t remaining;
    uint64_t schema_length;

    if (!node || !record || node->tag != RXCV_TAG_RECORD ||
        node->payload_length < 20u) return 0;
    cursor = node->payload;
    remaining = node->payload_length;
    schema_length = rxcv_u64(cursor);
    if (!schema_length || schema_length > SIZE_MAX ||
        (size_t)schema_length > remaining - 8u) return 0;
    record->schema = cursor + 8u;
    record->schema_length = (size_t)schema_length;
    cursor += 8u + record->schema_length;
    remaining -= 8u + record->schema_length;
    if (remaining < 12u) return 0;
    record->version = rxcv_u32(cursor);
    record->field_count = rxcv_u64(cursor + 4u);
    record->fields = cursor + 12u;
    record->fields_length = remaining - 12u;
    return 1;
}

static int rxcv_record_schema(const rxcv_record *record,
                              const char *schema,
                              uint32_t version,
                              uint64_t field_count) {
    size_t length = strlen(schema);
    return record && record->schema_length == length &&
           memcmp(record->schema, schema, length) == 0 &&
           record->version == version &&
           record->field_count == field_count;
}

static int rxcv_record_field(const rxcv_record *record,
                             const char *field_name,
                             rxcv_node *field_node) {
    const unsigned char *cursor;
    size_t remaining;
    uint64_t index;
    size_t expected_length = strlen(field_name);

    if (!record || !field_node) return 0;
    cursor = record->fields;
    remaining = record->fields_length;
    for (index = 0u; index < record->field_count; index++) {
        uint64_t name_length;
        rxcv_node node;
        if (remaining < 8u) return 0;
        name_length = rxcv_u64(cursor);
        cursor += 8u;
        remaining -= 8u;
        if (name_length > SIZE_MAX || (size_t)name_length > remaining) return 0;
        if (!rxcv_parse_node(cursor + (size_t)name_length,
                             remaining - (size_t)name_length, &node)) return 0;
        if ((size_t)name_length == expected_length &&
            memcmp(cursor, field_name, expected_length) == 0) {
            *field_node = node;
            return 1;
        }
        cursor += (size_t)name_length + node.total_length;
        remaining -= (size_t)name_length + node.total_length;
    }
    return 0;
}

static int rxcv_node_integer(const rxcv_node *node, int64_t *value_out) {
    if (!node || node->tag != RXCV_TAG_INTEGER ||
        node->payload_length != 8u) return 0;
    if (value_out) *value_out = rxcv_i64(node->payload);
    return 1;
}

static int rxcv_node_array(const rxcv_node *node,
                           uint64_t *count_out,
                           const unsigned char **items_out,
                           size_t *items_length_out) {
    if (!node || node->tag != RXCV_TAG_ARRAY ||
        node->payload_length < 8u) return 0;
    if (count_out) *count_out = rxcv_u64(node->payload);
    if (items_out) *items_out = node->payload + 8u;
    if (items_length_out) *items_length_out = node->payload_length - 8u;
    return 1;
}

static rxvm_channel_status rxcv_parse_pool_configuration(
        const void *data,
        size_t length,
        size_t *worker_count_out,
        size_t *admission_capacity_out) {
    rxcv_node root;
    rxcv_record record;
    rxcv_node admission_node;
    rxcv_node worker_node;
    int64_t admission;
    int64_t workers;
    rxvm_channel_status status = rxcv_document_root(data, length, &root);

    if (status != RXVM_CHANNEL_OK) return status;
    if (!rxcv_record_open(&root, &record) ||
        !rxcv_record_schema(&record, "crexx.channel.local-task-pool", 1u, 2u) ||
        !rxcv_record_field(&record, "admissionCapacity", &admission_node) ||
        !rxcv_record_field(&record, "workerCount", &worker_node) ||
        !rxcv_node_integer(&admission_node, &admission) ||
        !rxcv_node_integer(&worker_node, &workers) ||
        admission < 1 || admission > 65535 || workers < 1 || workers > 65535) {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    *worker_count_out = (size_t)workers;
    *admission_capacity_out = (size_t)admission;
    return RXVM_CHANNEL_OK;
}

static void rxcv_invoke_free(rxvm_channel_invoke *invoke) {
    if (!invoke) return;
    free(invoke->arguments);
    memset(invoke, 0, sizeof(*invoke));
}

static rxvm_channel_status rxcv_parse_task_target(
        const rxcv_node *node,
        uint64_t *callable_id_out) {
    rxcv_record record;
    rxcv_node callable_node;
    rxcv_node factory_node;
    rxcv_node image_node;
    rxcv_node kind_node;
    rxcv_node signature_node;
    int64_t callable_id;
    int64_t kind;
    uint64_t factory_count;

    if (!rxcv_record_open(node, &record) ||
        !rxcv_record_schema(&record, "crexx.channel.task-target", 1u, 5u) ||
        !rxcv_record_field(&record, "callableId", &callable_node) ||
        !rxcv_record_field(&record, "factoryArguments", &factory_node) ||
        !rxcv_record_field(&record, "imageDigest", &image_node) ||
        !rxcv_record_field(&record, "kind", &kind_node) ||
        !rxcv_record_field(&record, "signatureDigest", &signature_node) ||
        !rxcv_node_integer(&callable_node, &callable_id) || callable_id < 0 ||
        !rxcv_node_integer(&kind_node, &kind) || kind != 1 ||
        !rxcv_node_array(&factory_node, &factory_count, 0, 0) || factory_count ||
        image_node.tag != RXCV_TAG_BINARY || image_node.payload_length != 32u ||
        signature_node.tag != RXCV_TAG_BINARY ||
        signature_node.payload_length != 32u) {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    *callable_id_out = (uint64_t)callable_id;
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status rxcv_parse_task_invoke(
        const void *data,
        size_t length,
        rxvm_channel_invoke *invoke) {
    rxcv_node root;
    rxcv_record record;
    rxcv_node arguments_node;
    rxcv_node target_node;
    const unsigned char *cursor;
    size_t remaining;
    uint64_t count;
    uint64_t index;
    rxvm_channel_status status;

    memset(invoke, 0, sizeof(*invoke));
    status = rxcv_document_root(data, length, &root);
    if (status != RXVM_CHANNEL_OK) return status;
    if (!rxcv_record_open(&root, &record) ||
        !rxcv_record_schema(&record, "crexx.channel.task-invoke", 1u, 2u) ||
        !rxcv_record_field(&record, "arguments", &arguments_node) ||
        !rxcv_record_field(&record, "target", &target_node) ||
        !rxcv_node_array(&arguments_node, &count, &cursor, &remaining) ||
        count > INT_MAX || count > SIZE_MAX / sizeof(*invoke->arguments)) {
        return RXVM_CHANNEL_INVALID_CONFIGURATION;
    }
    status = rxcv_parse_task_target(&target_node, &invoke->callable_id);
    if (status != RXVM_CHANNEL_OK) return status;
    if (count) {
        invoke->arguments = (rxvm_executor_register_image *)calloc(
                (size_t)count, sizeof(*invoke->arguments));
        if (!invoke->arguments) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    invoke->argument_count = (size_t)count;
    for (index = 0u; index < count; index++) {
        rxcv_node item;
        if (!rxcv_parse_node(cursor, remaining, &item)) {
            rxcv_invoke_free(invoke);
            return RXVM_CHANNEL_INVALID_CONFIGURATION;
        }
        if (item.tag == RXCV_TAG_INTEGER && item.payload_length == 8u) {
            invoke->arguments[index].type = RXVM_EXECUTOR_REGISTER_INTEGER;
            invoke->arguments[index].integer = rxcv_i64(item.payload);
        } else if (item.tag == RXCV_TAG_STRING &&
                   !memchr(item.payload, 0, item.payload_length)) {
            invoke->arguments[index].type = RXVM_EXECUTOR_REGISTER_STRING;
            invoke->arguments[index].bytes = (const char *)item.payload;
            invoke->arguments[index].length = item.payload_length;
        } else {
            rxcv_invoke_free(invoke);
            return RXVM_CHANNEL_INVALID_CONFIGURATION;
        }
        cursor += item.total_length;
        remaining -= item.total_length;
    }
    return RXVM_CHANNEL_OK;
}

static int rxcv_buffer_reserve(rxcv_buffer *buffer, size_t addition) {
    size_t required;
    size_t capacity;
    unsigned char *data;

    if (addition > SIZE_MAX - buffer->length) return 0;
    required = buffer->length + addition;
    if (required <= buffer->capacity) return 1;
    capacity = buffer->capacity ? buffer->capacity : 256u;
    while (capacity < required) {
        size_t next = capacity > SIZE_MAX / 2u ? required : capacity * 2u;
        if (next < capacity) return 0;
        capacity = next;
    }
    data = (unsigned char *)realloc(buffer->data, capacity);
    if (!data) return 0;
    buffer->data = data;
    buffer->capacity = capacity;
    return 1;
}

static int rxcv_buffer_append(rxcv_buffer *buffer,
                              const void *data,
                              size_t length) {
    if (!rxcv_buffer_reserve(buffer, length)) return 0;
    if (length) memcpy(buffer->data + buffer->length, data, length);
    buffer->length += length;
    return 1;
}

static int rxcv_buffer_u32(rxcv_buffer *buffer, uint32_t value) {
    unsigned char bytes[4];
    rxcv_put_u32(bytes, value);
    return rxcv_buffer_append(buffer, bytes, sizeof(bytes));
}

static int rxcv_buffer_u64(rxcv_buffer *buffer, uint64_t value) {
    unsigned char bytes[8];
    rxcv_put_u64(bytes, value);
    return rxcv_buffer_append(buffer, bytes, sizeof(bytes));
}

static int rxcv_buffer_node(rxcv_buffer *buffer,
                            uint8_t tag,
                            const void *payload,
                            size_t payload_length) {
    unsigned char header[12];
    memset(header, 0, sizeof(header));
    header[0] = tag;
    rxcv_put_u64(header + 4u, payload_length);
    return rxcv_buffer_append(buffer, header, sizeof(header)) &&
           rxcv_buffer_append(buffer, payload, payload_length);
}

static int rxcv_buffer_integer_node(rxcv_buffer *buffer, int64_t value) {
    uint64_t bits;
    unsigned char payload[8];
    memcpy(&bits, &value, sizeof(bits));
    rxcv_put_u64(payload, bits);
    return rxcv_buffer_node(buffer, RXCV_TAG_INTEGER,
                            payload, sizeof(payload));
}

static int rxcv_buffer_field_name(rxcv_buffer *buffer, const char *name) {
    size_t length = strlen(name);
    return rxcv_buffer_u64(buffer, length) &&
           rxcv_buffer_append(buffer, name, length);
}

static const char *channel_completion_message(
        rxvm_executor_request_state state) {
    switch (state) {
        case RXVM_EXECUTOR_REQUEST_COMPLETED: return "";
        case RXVM_EXECUTOR_REQUEST_CANCELLED: return "cancelled";
        case RXVM_EXECUTOR_REQUEST_PROCEDURE_NOT_FOUND:
            return "task target not found";
        case RXVM_EXECUTOR_REQUEST_SETUP_FAILED: return "task setup failed";
        case RXVM_EXECUTOR_REQUEST_DEADLINE_EXCEEDED:
            return "deadline exceeded";
        case RXVM_EXECUTOR_REQUEST_KILLED: return "task terminated";
        case RXVM_EXECUTOR_REQUEST_SHUTDOWN: return "runtime shutdown";
        default: return "task failed";
    }
}

static int64_t channel_completion_state(
        rxvm_executor_request_state state) {
    if (state == RXVM_EXECUTOR_REQUEST_COMPLETED) return 1;
    if (state == RXVM_EXECUTOR_REQUEST_CANCELLED ||
        state == RXVM_EXECUTOR_REQUEST_SHUTDOWN) return 3;
    if (state == RXVM_EXECUTOR_REQUEST_DEADLINE_EXCEEDED) return 4;
    return 2;
}

static rxvm_channel_status rxcv_encode_completion(
        const rxvm_channel_ticket_slot *ticket,
        const rxvm_executor_completion *completion,
        int64_t provider_type,
        rxvm_channel_binary *output) {
    rxcv_buffer payload;
    rxcv_buffer document;
    const char *message;
    size_t message_length;
    unsigned char document_header[16];

    memset(&payload, 0, sizeof(payload));
    memset(&document, 0, sizeof(document));
    memset(output, 0, sizeof(*output));
    message = channel_completion_message(completion->state);
    message_length = strlen(message);
    if (!rxcv_buffer_u64(&payload, strlen("crexx.channel.completion")) ||
        !rxcv_buffer_append(&payload, "crexx.channel.completion",
                            strlen("crexx.channel.completion")) ||
        !rxcv_buffer_u32(&payload, 1u) ||
        !rxcv_buffer_u64(&payload, 8u) ||
        !rxcv_buffer_field_name(&payload, "details") ||
        !rxcv_buffer_node(&payload, RXCV_TAG_NULL, 0, 0u) ||
        !rxcv_buffer_field_name(&payload, "errorCode") ||
        !rxcv_buffer_integer_node(
            &payload, completion->state == RXVM_EXECUTOR_REQUEST_COMPLETED
                ? 0 : (int64_t)completion->state) ||
        !rxcv_buffer_field_name(&payload, "message") ||
        !rxcv_buffer_node(&payload, RXCV_TAG_STRING,
                          message, message_length) ||
        !rxcv_buffer_field_name(&payload, "providerType") ||
        !rxcv_buffer_integer_node(&payload, provider_type) ||
        !rxcv_buffer_field_name(&payload, "result")) goto out_of_memory;
    if (completion->state == RXVM_EXECUTOR_REQUEST_COMPLETED) {
        if (!rxcv_buffer_integer_node(&payload,
                                      completion->result.integer)) {
            goto out_of_memory;
        }
    } else if (!rxcv_buffer_node(&payload, RXCV_TAG_NULL, 0, 0u)) {
        goto out_of_memory;
    }
    if (!rxcv_buffer_field_name(&payload, "sequence") ||
        !rxcv_buffer_integer_node(&payload,
                                  (int64_t)ticket->submission_sequence) ||
        !rxcv_buffer_field_name(&payload, "state") ||
        !rxcv_buffer_integer_node(&payload,
                                  channel_completion_state(completion->state)) ||
        !rxcv_buffer_field_name(&payload, "ticket") ||
        !rxcv_buffer_integer_node(&payload, ticket->capability)) {
        goto out_of_memory;
    }
    memset(document_header, 0, sizeof(document_header));
    memcpy(document_header, "RXCV", 4u);
    document_header[4] = 1u;
    rxcv_put_u64(document_header + 8u, 16u + 12u + payload.length);
    if (!rxcv_buffer_append(&document, document_header,
                            sizeof(document_header)) ||
        !rxcv_buffer_node(&document, RXCV_TAG_RECORD,
                          payload.data, payload.length)) {
        goto out_of_memory;
    }
    free(payload.data);
    output->data = document.data;
    output->length = document.length;
    return RXVM_CHANNEL_OK;

out_of_memory:
    free(payload.data);
    free(document.data);
    return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
}

static uint64_t channel_monotonic_microseconds(void) {
#if defined(_WIN32)
    return (uint64_t)GetTickCount64() * UINT64_C(1000);
#else
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) abort();
    return (uint64_t)now.tv_sec * UINT64_C(1000000) +
           (uint64_t)now.tv_nsec / UINT64_C(1000);
#endif
}

static uint32_t channel_allocate_owner_id(void) {
    const uint32_t maximum = UINT32_C(0x3fffffff);
#if defined(_WIN32)
    LONG observed = InterlockedCompareExchange(
            (volatile LONG *)&rxvm_channel_next_owner_id, 0, 0);
    for (;;) {
        LONG desired;
        if ((uint32_t)observed >= maximum) return 0u;
        desired = observed + 1;
        {
            LONG actual = InterlockedCompareExchange(
                    (volatile LONG *)&rxvm_channel_next_owner_id,
                    desired, observed);
            if (actual == observed) return (uint32_t)desired;
            observed = actual;
        }
    }
#elif defined(__GNUC__) || defined(__clang__)
    uint32_t observed = __atomic_load_n(&rxvm_channel_next_owner_id,
                                        __ATOMIC_RELAXED);
    for (;;) {
        uint32_t desired;
        if (observed >= maximum) return 0u;
        desired = observed + 1u;
        if (__atomic_compare_exchange_n(
                &rxvm_channel_next_owner_id, &observed, desired, 0,
                __ATOMIC_RELAXED, __ATOMIC_RELAXED)) return desired;
    }
#else
#error Gate F owner allocation requires Windows or GCC/Clang atomics
#endif
}

static void channel_runtime_destroy(void *state) {
    rxvm_channel_runtime *runtime = (rxvm_channel_runtime *)state;
    if (!runtime) return;
    channel_mutex_lock(&runtime->mutex);
    if (runtime->live_executions) abort();
    runtime->runtime = 0;
    channel_mutex_unlock(&runtime->mutex);
    channel_mutex_destroy(&runtime->mutex);
    free(runtime);
}

static rxvm_channel_runtime *channel_runtime_for(rxvm_runtime *runtime) {
    rxvm_channel_runtime *state;
    rxvm_channel_runtime *candidate;

    state = (rxvm_channel_runtime *)rxvm_runtime_channel_state(runtime);
    if (state) return state;
    candidate = (rxvm_channel_runtime *)calloc(1u, sizeof(*candidate));
    if (!candidate) return 0;
    if (!channel_mutex_init(&candidate->mutex)) {
        free(candidate);
        return 0;
    }
    candidate->runtime = runtime;
    candidate->local_provider.type = RXVM_CHANNEL_PROVIDER_LOCAL;
    candidate->local_provider.name = "crexx.core.local-thread";
    candidate->local_provider.abi_version = 1u;
    candidate->local_provider.capabilities = RXVM_CHANNEL_LOCAL_CAPABILITIES;
    if (rxvm_runtime_install_channel_state(
            runtime, candidate, channel_runtime_destroy)) return candidate;
    channel_runtime_destroy(candidate);
    return (rxvm_channel_runtime *)rxvm_runtime_channel_state(runtime);
}

static rxvm_channel_context *channel_context_for(rxvm_context *context,
                                                 int create) {
    rxvm_channel_context *state;
    rxvm_channel_runtime *runtime_state;
    uint32_t owner;

    if (!context || !context->worker.runtime) return 0;
    state = context->channel_context;
    if (state || !create) return state;
    owner = channel_allocate_owner_id();
    if (!owner) return 0;
    runtime_state = channel_runtime_for(context->worker.runtime);
    if (!runtime_state) return 0;
    state = (rxvm_channel_context *)calloc(1u, sizeof(*state));
    if (!state) return 0;
    state->owner_id = owner;
    state->runtime_state = runtime_state;
    channel_mutex_lock(&runtime_state->mutex);
    runtime_state->live_executions++;
    channel_mutex_unlock(&runtime_state->mutex);
    context->channel_context = state;
    return state;
}

static int64_t channel_capability(uint32_t owner_id,
                                  unsigned int kind,
                                  uint16_t generation,
                                  size_t slot) {
    uint64_t capability = ((uint64_t)owner_id << 33u) |
                          ((uint64_t)(kind & 1u) << 32u) |
                          ((uint64_t)generation << 16u) |
                          (uint64_t)(slot + 1u);
    return (int64_t)capability;
}

static rxvm_channel_status channel_decode_capability(
        const rxvm_channel_context *state,
        int64_t capability,
        unsigned int expected_kind,
        size_t *slot_out,
        uint16_t *generation_out) {
    uint64_t bits;
    uint32_t owner;
    unsigned int kind;
    uint16_t generation;
    uint16_t slot_plus_one;

    if (capability <= 0) return RXVM_CHANNEL_STALE_CAPABILITY;
    bits = (uint64_t)capability;
    if (bits >> 63u) return RXVM_CHANNEL_STALE_CAPABILITY;
    owner = (uint32_t)((bits >> 33u) & UINT64_C(0x3fffffff));
    kind = (unsigned int)((bits >> 32u) & 1u);
    generation = (uint16_t)((bits >> 16u) & UINT64_C(0xffff));
    slot_plus_one = (uint16_t)(bits & UINT64_C(0xffff));
    if (!owner || !generation || !slot_plus_one || kind != expected_kind) {
        return RXVM_CHANNEL_STALE_CAPABILITY;
    }
    if (!state || owner != state->owner_id) return RXVM_CHANNEL_WRONG_OWNER;
    *slot_out = (size_t)slot_plus_one - 1u;
    *generation_out = generation;
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status channel_resolve(
        rxvm_channel_context *state,
        int64_t capability,
        size_t *slot_out,
        rxvm_channel_slot **channel_out) {
    size_t slot;
    uint16_t generation;
    rxvm_channel_status status = channel_decode_capability(
            state, capability, 0u, &slot, &generation);
    if (status != RXVM_CHANNEL_OK) return status;
    if (slot >= state->channel_count ||
        !state->channels[slot].live ||
        state->channels[slot].generation != generation) {
        return RXVM_CHANNEL_STALE_CAPABILITY;
    }
    if (slot_out) *slot_out = slot;
    if (channel_out) *channel_out = &state->channels[slot];
    return RXVM_CHANNEL_OK;
}

static rxvm_channel_status ticket_resolve(
        rxvm_channel_context *state,
        int64_t capability,
        size_t channel_slot,
        size_t *slot_out,
        rxvm_channel_ticket_slot **ticket_out) {
    size_t slot;
    uint16_t generation;
    rxvm_channel_status status = channel_decode_capability(
            state, capability, 1u, &slot, &generation);
    if (status != RXVM_CHANNEL_OK) return status;
    if (slot >= state->ticket_count || !state->tickets[slot].live ||
        state->tickets[slot].generation != generation) {
        return RXVM_CHANNEL_STALE_CAPABILITY;
    }
    if (state->tickets[slot].channel_slot != channel_slot) {
        return RXVM_CHANNEL_UNKNOWN_TICKET;
    }
    if (slot_out) *slot_out = slot;
    if (ticket_out) *ticket_out = &state->tickets[slot];
    return RXVM_CHANNEL_OK;
}

static int channel_grow(void **storage,
                        size_t *capacity,
                        size_t required,
                        size_t item_size) {
    size_t next;
    void *replacement;
    if (required <= *capacity) return 1;
    next = *capacity ? *capacity * 2u : 8u;
    if (next < required) next = required;
    if (next > RXVM_CHANNEL_MAX_SLOTS) next = RXVM_CHANNEL_MAX_SLOTS;
    if (next < required || next > SIZE_MAX / item_size) return 0;
    replacement = realloc(*storage, next * item_size);
    if (!replacement) return 0;
    memset((unsigned char *)replacement + *capacity * item_size, 0,
           (next - *capacity) * item_size);
    *storage = replacement;
    *capacity = next;
    return 1;
}

static int channel_allocate_slot(rxvm_channel_context *state,
                                 size_t *slot_out,
                                 rxvm_channel_slot **channel_out) {
    size_t slot;
    for (slot = 0u; slot < state->channel_count; slot++) {
        if (!state->channels[slot].live && !state->channels[slot].retired) break;
    }
    if (slot == state->channel_count) {
        if (state->channel_count == RXVM_CHANNEL_MAX_SLOTS ||
            !channel_grow((void **)&state->channels,
                          &state->channel_capacity,
                          state->channel_count + 1u,
                          sizeof(*state->channels))) return 0;
        state->channel_count++;
    }
    if (!state->channels[slot].generation) {
        state->channels[slot].generation = 1u;
    }
    state->channels[slot].live = 1u;
    state->live_channels++;
    *slot_out = slot;
    *channel_out = &state->channels[slot];
    return 1;
}

static int ticket_allocate_slot(rxvm_channel_context *state,
                                size_t *slot_out,
                                rxvm_channel_ticket_slot **ticket_out) {
    size_t slot;
    for (slot = 0u; slot < state->ticket_count; slot++) {
        if (!state->tickets[slot].live && !state->tickets[slot].retired) break;
    }
    if (slot == state->ticket_count) {
        if (state->ticket_count == RXVM_CHANNEL_MAX_SLOTS ||
            !channel_grow((void **)&state->tickets,
                          &state->ticket_capacity,
                          state->ticket_count + 1u,
                          sizeof(*state->tickets))) return 0;
        state->ticket_count++;
    }
    if (!state->tickets[slot].generation) {
        state->tickets[slot].generation = 1u;
    }
    state->tickets[slot].live = 1u;
    state->live_tickets++;
    *slot_out = slot;
    *ticket_out = &state->tickets[slot];
    return 1;
}

static void ticket_release_slot(rxvm_channel_context *state,
                                rxvm_channel_ticket_slot *ticket) {
    if (!ticket->live || !state->live_tickets) abort();
    ticket->live = 0u;
    ticket->observed = 0u;
    ticket->request = 0;
    ticket->capability = 0;
    ticket->submission_sequence = 0u;
    state->live_tickets--;
    ticket->generation++;
    if (!ticket->generation) ticket->retired = 1u;
}

static void channel_release_slot(rxvm_channel_context *state,
                                 rxvm_channel_slot *channel) {
    if (!channel->live || !state->live_channels) abort();
    channel->live = 0u;
    channel->executor = 0;
    channel->provider = 0;
    state->live_channels--;
    channel->generation++;
    if (!channel->generation) channel->retired = 1u;
}

static size_t channel_active_requests(rxvm_channel_context *state,
                                      size_t channel_slot) {
    size_t active = 0u;
    size_t index;
    for (index = 0u; index < state->ticket_count; index++) {
        rxvm_channel_ticket_slot *ticket = &state->tickets[index];
        if (ticket->live && ticket->channel_slot == channel_slot &&
            !rxvm_executor_request_completion_snapshot(
                ticket->request, 0, 0)) active++;
    }
    return active;
}

static int64_t channel_remaining_wait(uint64_t deadline) {
    uint64_t now = channel_monotonic_microseconds();
    uint64_t remaining;
    if (now >= deadline) return 0;
    remaining = deadline - now;
    return remaining > (uint64_t)INT64_MAX
            ? INT64_MAX : (int64_t)remaining;
}

rxvm_channel_status rxvm_channel_open(
        rxvm_context *context,
        int64_t provider_type,
        int64_t required_capabilities,
        const void *configuration,
        size_t configuration_length,
        int64_t *channel_out) {
    rxvm_channel_context *state;
    rxvm_channel_slot *channel;
    const rxvm_channel_provider_descriptor *provider;
    const rxvm_program_generation *generation;
    rxvm_executor_result executor_result;
    rxvm_program_result program_result;
    size_t worker_count;
    size_t admission_capacity;
    size_t slot;
    rxvm_channel_status status;

    if (channel_out) *channel_out = 0;
    if (!context || !channel_out || required_capabilities < 0) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    if (provider_type <= 0 || provider_type != RXVM_CHANNEL_PROVIDER_LOCAL) {
        return RXVM_CHANNEL_INVALID_PROVIDER;
    }
    status = rxcv_parse_pool_configuration(
            configuration, configuration_length,
            &worker_count, &admission_capacity);
    if (status != RXVM_CHANNEL_OK) return status;
    state = channel_context_for(context, 1);
    if (!state) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    provider = &state->runtime_state->local_provider;
    if (provider->type != provider_type || !provider->name ||
        provider->abi_version != 1u) {
        return RXVM_CHANNEL_PROVIDER_UNAVAILABLE;
    }
    if (((uint64_t)required_capabilities & ~provider->capabilities) != 0u) {
        return RXVM_CHANNEL_UNSUPPORTED_CAPABILITY;
    }
    generation = rxvm_program_generation_current(context);
    if (!generation) {
        program_result = rxvm_program_generation_seal(context, &generation);
        if (program_result != RXVM_PROGRAM_OK) {
            return program_result == RXVM_PROGRAM_OUT_OF_MEMORY
                ? RXVM_CHANNEL_RESOURCE_EXHAUSTED
                : RXVM_CHANNEL_PROVIDER_UNAVAILABLE;
        }
    }
    if (!channel_allocate_slot(state, &slot, &channel)) {
        return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    }
    channel->executor = rxvm_executor_create_attached(
            context->worker.runtime, generation, worker_count,
            admission_capacity, &executor_result);
    if (!channel->executor) {
        channel_release_slot(state, channel);
        return executor_result == RXVM_EXECUTOR_OUT_OF_MEMORY
            ? RXVM_CHANNEL_RESOURCE_EXHAUSTED
            : RXVM_CHANNEL_PROVIDER_FAILURE;
    }
    channel->admission_capacity = admission_capacity;
    channel->worker_count = worker_count;
    channel->next_worker = 0u;
    channel->next_submission_sequence = 1u;
    channel->provider = provider;
    channel->state = RXVM_CHANNEL_SLOT_OPEN;
    *channel_out = channel_capability(
            state->owner_id, 0u, channel->generation, slot);
    return RXVM_CHANNEL_OK;
}

rxvm_channel_status rxvm_channel_start(
        rxvm_context *context,
        int64_t channel_capability_value,
        const void *envelope,
        size_t envelope_length,
        int64_t wait_microseconds,
        int64_t *ticket_out) {
    rxvm_channel_context *state;
    rxvm_channel_slot *channel;
    rxvm_channel_ticket_slot *ticket;
    rxvm_channel_invoke invoke;
    rxvm_executor_result result;
    rxvm_channel_status status;
    size_t channel_slot;
    size_t ticket_slot;
    uint64_t deadline = 0u;

    if (ticket_out) *ticket_out = 0;
    if (!context || !ticket_out || wait_microseconds < -1) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    state = channel_context_for(context, 1);
    if (!state) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    status = channel_resolve(state, channel_capability_value,
                             &channel_slot, &channel);
    if (status != RXVM_CHANNEL_OK) return status;
    if (channel->state != RXVM_CHANNEL_SLOT_OPEN) {
        return RXVM_CHANNEL_CLOSED;
    }
    status = rxcv_parse_task_invoke(envelope, envelope_length, &invoke);
    if (status != RXVM_CHANNEL_OK) return status;
    if (wait_microseconds > 0) {
        uint64_t now = channel_monotonic_microseconds();
        uint64_t duration = (uint64_t)wait_microseconds;
        deadline = duration > UINT64_MAX - now ? UINT64_MAX : now + duration;
    }
    for (;;) {
        uint64_t observed_generation;
        size_t attempts;
        if (channel_active_requests(state, channel_slot) <
                channel->admission_capacity) {
            if (!ticket_allocate_slot(state, &ticket_slot, &ticket)) {
                rxcv_invoke_free(&invoke);
                return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
            }
            ticket->channel_slot = (uint16_t)channel_slot;
            result = RXVM_EXECUTOR_QUEUE_FULL;
            for (attempts = 0u; attempts < channel->worker_count; attempts++) {
                size_t worker = (channel->next_worker + attempts) %
                                channel->worker_count;
                result = rxvm_executor_submit_callable_registers(
                        channel->executor, worker, invoke.callable_id,
                        invoke.argument_count, invoke.arguments,
                        &ticket->request);
                if (result != RXVM_EXECUTOR_QUEUE_FULL) {
                    channel->next_worker = (worker + 1u) % channel->worker_count;
                    break;
                }
            }
            if (result == RXVM_EXECUTOR_OK) {
                ticket->submission_sequence =
                        channel->next_submission_sequence++;
                ticket->capability = channel_capability(
                        state->owner_id, 1u, ticket->generation, ticket_slot);
                *ticket_out = ticket->capability;
                rxcv_invoke_free(&invoke);
                return RXVM_CHANNEL_OK;
            }
            ticket_release_slot(state, ticket);
            if (result != RXVM_EXECUTOR_QUEUE_FULL) {
                rxcv_invoke_free(&invoke);
                return result == RXVM_EXECUTOR_OUT_OF_MEMORY
                    ? RXVM_CHANNEL_RESOURCE_EXHAUSTED
                    : (result == RXVM_EXECUTOR_STOPPING
                        ? RXVM_CHANNEL_SHUTTING_DOWN
                        : RXVM_CHANNEL_PROVIDER_FAILURE);
            }
        }
        if (wait_microseconds == 0) {
            rxcv_invoke_free(&invoke);
            return RXVM_CHANNEL_BACKPRESSURE;
        }
        observed_generation = rxvm_executor_completion_generation_get(
                channel->executor);
        if (channel_active_requests(state, channel_slot) <
                channel->admission_capacity) continue;
        if (wait_microseconds < 0) {
            (void)rxvm_executor_completion_generation_wait(
                    channel->executor, observed_generation, -1, 0);
        } else {
            int64_t remaining = channel_remaining_wait(deadline);
            if (!remaining || !rxvm_executor_completion_generation_wait(
                    channel->executor, observed_generation, remaining, 0)) {
                rxcv_invoke_free(&invoke);
                return RXVM_CHANNEL_TIMEOUT;
            }
        }
    }
}

rxvm_channel_status rxvm_channel_wait(
        rxvm_context *context,
        int64_t channel_capability_value,
        int64_t wait_microseconds,
        rxvm_channel_binary *completion_out) {
    rxvm_channel_context *state;
    rxvm_channel_slot *channel;
    rxvm_channel_status status;
    size_t channel_slot;
    uint64_t deadline = 0u;

    if (completion_out) memset(completion_out, 0, sizeof(*completion_out));
    if (!context || !completion_out || wait_microseconds < -1) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    state = channel_context_for(context, 1);
    if (!state) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    status = channel_resolve(state, channel_capability_value,
                             &channel_slot, &channel);
    if (status != RXVM_CHANNEL_OK) return status;
    if (wait_microseconds > 0) {
        uint64_t now = channel_monotonic_microseconds();
        uint64_t duration = (uint64_t)wait_microseconds;
        deadline = duration > UINT64_MAX - now ? UINT64_MAX : now + duration;
    }
    for (;;) {
        rxvm_channel_ticket_slot *best = 0;
        rxvm_executor_completion best_completion;
        uint64_t best_terminal_sequence = UINT64_MAX;
        uint64_t observed_generation;
        size_t index;

        observed_generation = rxvm_executor_completion_generation_get(
                channel->executor);
        for (index = 0u; index < state->ticket_count; index++) {
            rxvm_channel_ticket_slot *ticket = &state->tickets[index];
            rxvm_executor_completion completion;
            uint64_t terminal_sequence;
            if (!ticket->live || ticket->observed ||
                ticket->channel_slot != channel_slot ||
                !rxvm_executor_request_completion_snapshot(
                    ticket->request, &completion, &terminal_sequence)) {
                continue;
            }
            if (terminal_sequence && terminal_sequence < best_terminal_sequence) {
                best = ticket;
                best_completion = completion;
                best_terminal_sequence = terminal_sequence;
            }
        }
        if (best) {
            rxvm_channel_binary encoded = {0, 0, 0};
            status = rxcv_encode_completion(best, &best_completion,
                                            channel->provider->type, &encoded);
            if (status == RXVM_CHANNEL_OK && encoded.length) {
                completion_out->data = (unsigned char *)rxvm_memory_alloc_bytes(
                        context->worker.memory_worker, encoded.length);
                if (!completion_out->data) {
                    status = RXVM_CHANNEL_RESOURCE_EXHAUSTED;
                } else {
                    memcpy(completion_out->data, encoded.data, encoded.length);
                    completion_out->length = encoded.length;
                    completion_out->capacity =
                            rxvm_memory_capacity(completion_out->data);
                }
            }
            free(encoded.data);
            if (status == RXVM_CHANNEL_OK) best->observed = 1u;
            return status;
        }
        if (wait_microseconds == 0) return RXVM_CHANNEL_WOULD_BLOCK;
        if (wait_microseconds < 0) {
            (void)rxvm_executor_completion_generation_wait(
                    channel->executor, observed_generation, -1, 0);
        } else {
            int64_t remaining = channel_remaining_wait(deadline);
            if (!remaining || !rxvm_executor_completion_generation_wait(
                    channel->executor, observed_generation, remaining, 0)) {
                return RXVM_CHANNEL_TIMEOUT;
            }
        }
    }
}

rxvm_channel_status rxvm_channel_cancel(
        rxvm_context *context,
        int64_t channel_capability_value,
        int64_t ticket_capability_value,
        const void *reason,
        size_t reason_length) {
    rxvm_channel_context *state;
    rxvm_channel_slot *channel;
    rxvm_channel_ticket_slot *ticket;
    rxcv_node reason_root;
    rxvm_channel_status status;
    rxvm_executor_result result;
    size_t channel_slot;

    if (!context) return RXVM_CHANNEL_INVALID_ARGUMENT;
    status = rxcv_document_root(reason, reason_length, &reason_root);
    if (status != RXVM_CHANNEL_OK) return status;
    state = channel_context_for(context, 1);
    if (!state) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    status = channel_resolve(state, channel_capability_value,
                             &channel_slot, &channel);
    if (status != RXVM_CHANNEL_OK) return status;
    status = ticket_resolve(state, ticket_capability_value,
                            channel_slot, 0, &ticket);
    if (status != RXVM_CHANNEL_OK) return status;
    result = rxvm_executor_cancel(ticket->request);
    if (result == RXVM_EXECUTOR_OK) return RXVM_CHANNEL_OK;
    if (result == RXVM_EXECUTOR_ALREADY_TERMINAL) {
        return RXVM_CHANNEL_ALREADY_TERMINAL;
    }
    if (result == RXVM_EXECUTOR_STOPPING) return RXVM_CHANNEL_SHUTTING_DOWN;
    return RXVM_CHANNEL_PROVIDER_FAILURE;
}

static rxvm_channel_status channel_close_slot(
        rxvm_channel_context *state,
        size_t channel_slot,
        rxvm_channel_slot *channel,
        int64_t mode) {
    size_t index;
    size_t leaks;

    channel->state = mode == 1
        ? RXVM_CHANNEL_SLOT_CLOSING_DRAIN
        : RXVM_CHANNEL_SLOT_CLOSING_CANCEL;
    if (mode == 2) {
        for (index = 0u; index < state->ticket_count; index++) {
            rxvm_channel_ticket_slot *ticket = &state->tickets[index];
            if (!ticket->live || ticket->channel_slot != channel_slot) continue;
            (void)rxvm_executor_cancel(ticket->request);
        }
    }
    leaks = rxvm_executor_destroy(channel->executor);
    channel->executor = 0;
    if (leaks) return RXVM_CHANNEL_INTERNAL_ERROR;
    for (index = 0u; index < state->ticket_count; index++) {
        rxvm_channel_ticket_slot *ticket = &state->tickets[index];
        if (!ticket->live || ticket->channel_slot != channel_slot) continue;
        (void)rxvm_executor_request_wait(ticket->request, 0);
        if (rxvm_executor_request_destroy(ticket->request) !=
                RXVM_EXECUTOR_OK) return RXVM_CHANNEL_INTERNAL_ERROR;
        ticket_release_slot(state, ticket);
    }
    channel_release_slot(state, channel);
    return RXVM_CHANNEL_OK;
}

rxvm_channel_status rxvm_channel_close(
        rxvm_context *context,
        int64_t channel_capability_value,
        int64_t mode) {
    rxvm_channel_context *state;
    rxvm_channel_slot *channel;
    rxvm_channel_status status;
    size_t channel_slot;

    if (!context || (mode != 1 && mode != 2)) {
        return RXVM_CHANNEL_INVALID_ARGUMENT;
    }
    state = channel_context_for(context, 1);
    if (!state) return RXVM_CHANNEL_RESOURCE_EXHAUSTED;
    status = channel_resolve(state, channel_capability_value,
                             &channel_slot, &channel);
    if (status != RXVM_CHANNEL_OK) return status;
    if (channel->state != RXVM_CHANNEL_SLOT_OPEN) {
        return RXVM_CHANNEL_CLOSED;
    }
    return channel_close_slot(state, channel_slot, channel, mode);
}

void rxvm_channel_binary_free(rxvm_channel_binary *binary) {
    if (!binary) return;
    if (binary->data &&
        rxvm_memory_release(binary->data) != RXVM_MEMORY_OK) abort();
    binary->data = 0;
    binary->length = 0u;
    binary->capacity = 0u;
}

void rxvm_channel_context_destroy(rxvm_context *context) {
    rxvm_channel_context *state;
    size_t index;

    if (!context || !context->channel_context) return;
    state = context->channel_context;
    for (index = 0u; index < state->channel_count; index++) {
        rxvm_channel_slot *channel = &state->channels[index];
        if (!channel->live) continue;
        (void)channel_close_slot(state, index, channel, 2);
    }
    if (state->live_channels || state->live_tickets) abort();
    channel_mutex_lock(&state->runtime_state->mutex);
    if (!state->runtime_state->live_executions) abort();
    state->runtime_state->live_executions--;
    channel_mutex_unlock(&state->runtime_state->mutex);
    free(state->channels);
    free(state->tickets);
    free(state);
    context->channel_context = 0;
}

size_t rxvm_channel_context_live_channels(const rxvm_context *context) {
    return context && context->channel_context
        ? context->channel_context->live_channels : 0u;
}

size_t rxvm_channel_context_live_tickets(const rxvm_context *context) {
    return context && context->channel_context
        ? context->channel_context->live_tickets : 0u;
}
