/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxbin.h"
#include "rxsha256.h"

#include <limits.h>
#include <stdarg.h>

typedef struct rxbin007_pool_entry {
    size_t offset;
    chameleon_constant *entry;
} rxbin007_pool_entry;

typedef struct rxbin007_pool {
    unsigned char *data;
    size_t size;
    rxbin007_pool_entry *entries;
    uint32_t entry_count;
    uint32_t entry_capacity;
} rxbin007_pool;

typedef struct rxbin007_module_write {
    module_file *module;
    uint32_t pool_index;
    uint64_t instruction_offset;
    uint64_t instruction_size;
} rxbin007_module_write;

typedef struct rxbin007_section {
    uint32_t kind;
    uint32_t flags;
    uint32_t alignment;
    rxbin_byte_buffer bytes;
    uint64_t file_offset;
    uint64_t expanded_size;
} rxbin007_section;

#define RXBIN007_LZSS_WINDOW 4096u
#define RXBIN007_LZSS_HASH_SIZE 8192u
#define RXBIN007_LZSS_MIN_MATCH 3u
#define RXBIN007_LZSS_MAX_MATCH 18u
#define RXBIN007_LZSS_MAX_CHAIN 64u
#define RXBIN007_LZSS_PREV_SIZE (RXBIN007_LZSS_WINDOW + 1u)

static char rxbin007_error[512];

static void rxbin007_set_error(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vsnprintf(rxbin007_error, sizeof(rxbin007_error), format, args);
    va_end(args);
}

const char *rxbin_last_error(void) {
    return rxbin007_error[0] ? rxbin007_error : 0;
}

static void rxbin007_clear_error(void) {
    rxbin007_error[0] = 0;
}

static int rxbin007_u32(rxbin_byte_buffer *buffer, uint32_t value) {
    unsigned char bytes[4];

    bytes[0] = (unsigned char)(value & 0xffu);
    bytes[1] = (unsigned char)((value >> 8u) & 0xffu);
    bytes[2] = (unsigned char)((value >> 16u) & 0xffu);
    bytes[3] = (unsigned char)((value >> 24u) & 0xffu);
    return rxbin_byte_buffer_append_bytes(buffer, bytes, sizeof(bytes));
}

static int rxbin007_u64(rxbin_byte_buffer *buffer, uint64_t value) {
    unsigned char bytes[8];
    unsigned int i;

    for (i = 0u; i < 8u; i++) {
        bytes[i] = (unsigned char)((value >> (i * 8u)) & UINT64_C(0xff));
    }
    return rxbin_byte_buffer_append_bytes(buffer, bytes, sizeof(bytes));
}

static int rxbin007_i32(rxbin_byte_buffer *buffer, int32_t value) {
    return rxbin007_u32(buffer, (uint32_t)value);
}

static int rxbin007_byte(rxbin_byte_buffer *buffer, unsigned char value) {
    return rxbin_byte_buffer_append_bytes(buffer, &value, 1u);
}

static unsigned int rxbin007_lzss_hash(const unsigned char *input) {
    return (unsigned int)(((unsigned int)input[0] * 251u +
                           (unsigned int)input[1] * 11u +
                           (unsigned int)input[2]) & (RXBIN007_LZSS_HASH_SIZE - 1u));
}

static size_t rxbin007_lzss_match_length(const unsigned char *input,
                                         size_t input_size,
                                         size_t left,
                                         size_t right) {
    size_t max_length;
    size_t length;

    max_length = input_size - right;
    if (max_length > RXBIN007_LZSS_MAX_MATCH) max_length = RXBIN007_LZSS_MAX_MATCH;
    length = 0u;
    while (length < max_length && input[left + length] == input[right + length]) length++;
    return length;
}

static void rxbin007_lzss_index_position(const unsigned char *input,
                                         size_t input_size,
                                         size_t position,
                                         size_t *last_positions,
                                         size_t *previous_positions) {
    unsigned int hash;
    size_t slot;

    slot = position % RXBIN007_LZSS_PREV_SIZE;
    if (position + RXBIN007_LZSS_MIN_MATCH > input_size) {
        previous_positions[slot] = SIZE_MAX;
        return;
    }
    hash = rxbin007_lzss_hash(input + position);
    previous_positions[slot] = last_positions[hash];
    last_positions[hash] = position;
}

static int rxbin007_lzss_compress(const unsigned char *input,
                                  size_t input_size,
                                  rxbin_byte_buffer *output) {
    size_t *previous_positions;
    size_t last_positions[RXBIN007_LZSS_HASH_SIZE];
    size_t position;
    size_t control_index;
    unsigned char control;
    unsigned int control_bit;
    int group_open;
    unsigned int i;

    if (!input_size) return 1;
    previous_positions = (size_t *)malloc(sizeof(size_t) * RXBIN007_LZSS_PREV_SIZE);
    if (!previous_positions) return 0;
    for (i = 0u; i < RXBIN007_LZSS_HASH_SIZE; i++) last_positions[i] = SIZE_MAX;
    for (i = 0u; i < RXBIN007_LZSS_PREV_SIZE; i++) previous_positions[i] = SIZE_MAX;
    position = 0u;
    control_index = 0u;
    control = 0u;
    control_bit = 0u;
    group_open = 0;

    while (position < input_size) {
        size_t best_length;
        size_t best_distance;

        best_length = 0u;
        best_distance = 0u;
        if (!group_open) {
            if (!rxbin007_byte(output, 0u)) goto error;
            control_index = output->size - 1u;
            control = 0u;
            control_bit = 0u;
            group_open = 1;
        }
        if (position + RXBIN007_LZSS_MIN_MATCH <= input_size) {
            unsigned int hash;
            size_t candidate;
            unsigned int chain;

            hash = rxbin007_lzss_hash(input + position);
            candidate = last_positions[hash];
            chain = 0u;
            while (candidate != SIZE_MAX && position > candidate &&
                   position - candidate <= RXBIN007_LZSS_WINDOW &&
                   chain < RXBIN007_LZSS_MAX_CHAIN) {
                size_t candidate_length;

                candidate_length = rxbin007_lzss_match_length(input,
                                                               input_size,
                                                               candidate,
                                                               position);
                if (candidate_length >= RXBIN007_LZSS_MIN_MATCH &&
                    candidate_length > best_length) {
                    best_length = candidate_length;
                    best_distance = position - candidate;
                    if (best_length == RXBIN007_LZSS_MAX_MATCH) break;
                }
                candidate = previous_positions[candidate % RXBIN007_LZSS_PREV_SIZE];
                chain++;
            }
        }
        if (best_length >= RXBIN007_LZSS_MIN_MATCH) {
            unsigned char token_a;
            unsigned char token_b;
            size_t offset;

            token_a = (unsigned char)(((best_length - RXBIN007_LZSS_MIN_MATCH) << 4u) |
                                      (((best_distance - 1u) >> 8u) & 0x0fu));
            token_b = (unsigned char)((best_distance - 1u) & 0xffu);
            control |= (unsigned char)(1u << control_bit);
            if (!rxbin007_byte(output, token_a) || !rxbin007_byte(output, token_b)) goto error;
            for (offset = 0u; offset < best_length; offset++) {
                rxbin007_lzss_index_position(input,
                                             input_size,
                                             position + offset,
                                             last_positions,
                                             previous_positions);
            }
            position += best_length;
        } else {
            if (!rxbin007_byte(output, input[position])) goto error;
            rxbin007_lzss_index_position(input,
                                         input_size,
                                         position,
                                         last_positions,
                                         previous_positions);
            position++;
        }
        control_bit++;
        if (control_bit == 8u) {
            output->data[control_index] = control;
            group_open = 0;
        }
    }
    if (group_open) output->data[control_index] = control;
    free(previous_positions);
    return 1;

error:
    free(previous_positions);
    return 0;
}

static int rxbin007_lzss_decompress(const unsigned char *input,
                                    size_t input_size,
                                    unsigned char *output,
                                    size_t output_size) {
    size_t input_position;
    size_t output_position;

    input_position = 0u;
    output_position = 0u;
    while (output_position < output_size) {
        unsigned char control;
        unsigned int bit;

        if (input_position >= input_size) return 0;
        control = input[input_position++];
        for (bit = 0u; bit < 8u && output_position < output_size; bit++) {
            if (control & (unsigned char)(1u << bit)) {
                size_t length;
                size_t distance;
                size_t copied;

                if (input_size - input_position < 2u) return 0;
                length = (size_t)((input[input_position] >> 4u) + RXBIN007_LZSS_MIN_MATCH);
                distance = (size_t)((((size_t)input[input_position] & 0x0fu) << 8u) |
                                    input[input_position + 1u]) + 1u;
                input_position += 2u;
                if (!distance || distance > output_position ||
                    length > output_size - output_position) return 0;
                for (copied = 0u; copied < length; copied++) {
                    output[output_position] = output[output_position - distance];
                    output_position++;
                }
            } else {
                if (input_position >= input_size) return 0;
                output[output_position++] = input[input_position++];
            }
        }
    }
    return input_position == input_size;
}

static int rxbin007_align(rxbin_byte_buffer *buffer, size_t alignment) {
    static const unsigned char zeros[8] = {0};
    size_t padding;

    if (!alignment || (alignment & (alignment - 1u))) return 0;
    padding = (alignment - (buffer->size & (alignment - 1u))) & (alignment - 1u);
    return rxbin_byte_buffer_append_bytes(buffer, zeros, padding);
}

static uint32_t rxbin007_read_u32_at(const unsigned char *data) {
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8u) |
           ((uint32_t)data[2] << 16u) |
           ((uint32_t)data[3] << 24u);
}

static uint64_t rxbin007_read_u64_at(const unsigned char *data) {
    uint64_t value;
    unsigned int i;

    value = 0u;
    for (i = 0u; i < 8u; i++) value |= ((uint64_t)data[i]) << (i * 8u);
    return value;
}

static int rxbin007_is_metadata(enum const_pool_type type) {
    return type >= META_FUNC && type <= META_AUTOLOAD;
}

static size_t rxbin007_bounded_strlen(const char *text, size_t available) {
    const void *end;

    if (!text) return SIZE_MAX;
    end = memchr(text, 0, available);
    return end ? (size_t)((const char *)end - text) : SIZE_MAX;
}

static void rxbin007_free_pools(rxbin007_pool *pools, uint32_t pool_count) {
    uint32_t i;

    if (!pools) return;
    for (i = 0u; i < pool_count; i++) free(pools[i].entries);
    free(pools);
}

static int rxbin007_pool_add_entry(rxbin007_pool *pool,
                                   size_t offset,
                                   chameleon_constant *entry) {
    rxbin007_pool_entry *entries;
    uint32_t capacity;

    if (pool->entry_count == pool->entry_capacity) {
        capacity = pool->entry_capacity ? pool->entry_capacity * 2u : 32u;
        if (capacity < pool->entry_count ||
            (size_t)capacity > SIZE_MAX / sizeof(*entries)) return 0;
        entries = (rxbin007_pool_entry *)realloc(pool->entries,
                                                 (size_t)capacity * sizeof(*entries));
        if (!entries) return 0;
        pool->entries = entries;
        pool->entry_capacity = capacity;
    }
    pool->entries[pool->entry_count].offset = offset;
    pool->entries[pool->entry_count].entry = entry;
    pool->entry_count++;
    return 1;
}

static int rxbin007_scan_pool(rxbin007_pool *pool, const char *context) {
    size_t offset;

    offset = 0u;
    if (!pool->size) return 1;
    if (!pool->data) {
        rxbin007_set_error("RXBIN 007 %s has a null constant pool", context);
        return 0;
    }
    while (offset < pool->size) {
        chameleon_constant *entry;
        size_t minimum;

        if (pool->size - offset < sizeof(chameleon_constant)) {
            rxbin007_set_error("RXBIN 007 %s constant pool is truncated at offset %lu",
                               context, (unsigned long)offset);
            return 0;
        }
        entry = (chameleon_constant *)(pool->data + offset);
        minimum = sizeof(chameleon_constant);
        if (entry->type < STRING_CONST || entry->type > META_AUTOLOAD ||
            entry->size_in_pool < minimum || (entry->size_in_pool & 7u) ||
            entry->size_in_pool > pool->size - offset) {
            rxbin007_set_error("RXBIN 007 %s has an invalid constant entry at offset %lu",
                               context, (unsigned long)offset);
            return 0;
        }
        if (pool->entry_count == UINT32_MAX ||
            !rxbin007_pool_add_entry(pool, offset, entry)) {
            rxbin007_set_error("RXBIN 007 cannot index the %s constant pool", context);
            return 0;
        }
        offset += entry->size_in_pool;
    }
    return offset == pool->size;
}

static int rxbin007_pool_find_id(const rxbin007_pool *pool,
                                 size_t offset,
                                 uint32_t *id) {
    uint32_t left;
    uint32_t right;

    if (!id) return 0;
    if (offset == SIZE_MAX) {
        *id = RXBIN007_NONE;
        return 1;
    }
    left = 0u;
    right = pool ? pool->entry_count : 0u;
    while (left < right) {
        uint32_t mid;
        mid = left + ((right - left) >> 1u);
        if (pool->entries[mid].offset < offset) left = mid + 1u;
        else right = mid;
    }
    if (pool && left < pool->entry_count && pool->entries[left].offset == offset) {
        *id = left;
        return 1;
    }
    return 0;
}

static int rxbin007_pool_find_int_id(const rxbin007_pool *pool,
                                     int offset,
                                     uint32_t *id) {
    if (offset == -1) {
        *id = RXBIN007_NONE;
        return 1;
    }
    return offset >= 0 && rxbin007_pool_find_id(pool, (size_t)offset, id);
}

typedef struct rxbin007_task_binding_patch {
    string_constant *constant;
    unsigned char binding[RX_GRAPH_TASK_BINDING_SIZE];
} rxbin007_task_binding_patch;

static void rxbin007_task_placeholder(const char *symbol,
                                      unsigned char placeholder[RX_GRAPH_TASK_BINDING_SIZE]) {
    unsigned char digest[32];
    size_t i;

    rx_sha256(symbol, strlen(symbol), digest);
    for (i = 0u; i < RX_GRAPH_TASK_BINDING_SIZE; i++) {
        placeholder[i] = (unsigned char)(
                digest[i & 31u] ^ (unsigned char)(i * 0x5bu + 0x31u));
    }
}

static int rxbin007_record_task_binding_patch(
        rxbin007_task_binding_patch **patches,
        size_t *patch_count,
        size_t *patch_capacity,
        string_constant *binding,
        const unsigned char expected[RX_GRAPH_TASK_BINDING_SIZE]) {
    size_t prior;

    for (prior = 0u; prior < *patch_count; prior++) {
        if ((*patches)[prior].constant == binding) break;
    }
    if (prior < *patch_count) {
        if (memcmp((*patches)[prior].binding, expected,
                   RX_GRAPH_TASK_BINDING_SIZE) != 0) {
            rxbin007_set_error(
                    "RXBIN 007 task relocations alias incompatible targets");
            return 0;
        }
        return 1;
    }
    if (*patch_count == *patch_capacity) {
        rxbin007_task_binding_patch *grown;
        size_t capacity;

        capacity = *patch_capacity ? *patch_capacity * 2u : 8u;
        if (capacity < *patch_count ||
            capacity > SIZE_MAX / sizeof(*grown)) {
            rxbin007_set_error(
                    "RXBIN 007 cannot record task-target relocations");
            return 0;
        }
        grown = (rxbin007_task_binding_patch *)realloc(
                *patches, capacity * sizeof(*grown));
        if (!grown) {
            rxbin007_set_error(
                    "RXBIN 007 cannot allocate task-target relocations");
            return 0;
        }
        *patches = grown;
        *patch_capacity = capacity;
    }
    (*patches)[*patch_count].constant = binding;
    memcpy((*patches)[*patch_count].binding, expected,
           RX_GRAPH_TASK_BINDING_SIZE);
    (*patch_count)++;
    return 1;
}

static int rxbin007_materialize_task_bindings(rxbin007_pool *pools,
                                              uint32_t pool_count,
                                              const RxGraph *graph) {
    rxbin007_task_binding_patch *patches;
    size_t patch_count;
    size_t patch_capacity;
    size_t patch;
    uint32_t pool_index;

    patches = 0;
    patch_count = 0u;
    patch_capacity = 0u;
    for (pool_index = 0u; pool_index < pool_count; pool_index++) {
        rxbin007_pool *pool;
        uint32_t record;

        pool = &pools[pool_index];
        for (record = 0u; record < pool->entry_count; record++) {
            meta_task_target_constant *target;
            string_constant *symbol;
            string_constant *binding;
            unsigned char expected[RX_GRAPH_TASK_BINDING_SIZE];
            uint32_t symbol_id;
            uint32_t binding_id;
            size_t symbol_available;
            size_t binding_available;
            unsigned char placeholder[RX_GRAPH_TASK_BINDING_SIZE];
            uint32_t candidate_pool_index;

            if (pool->entries[record].entry->type != META_TASK_TARGET) continue;
            target = (meta_task_target_constant *)pool->entries[record].entry;
            if (target->base.base.size_in_pool < sizeof(*target) ||
                target->kind < 1u || target->kind > 3u ||
                !rxbin007_pool_find_id(pool, target->symbol, &symbol_id) ||
                !rxbin007_pool_find_id(pool, target->binding, &binding_id)) {
                rxbin007_set_error("RXBIN 007 has an invalid task-target relocation in pool %u record %u",
                                   pool_index, record);
                free(patches);
                return 0;
            }
            symbol = (string_constant *)pool->entries[symbol_id].entry;
            binding = (string_constant *)pool->entries[binding_id].entry;
            symbol_available = symbol->base.size_in_pool - offsetof(string_constant, string);
            binding_available = binding->base.size_in_pool - offsetof(string_constant, string);
            if (symbol->base.type != STRING_CONST ||
                binding->base.type != BINARY_CONST ||
                symbol->string_len >= symbol_available ||
                symbol->string[symbol->string_len] != 0 ||
                binding->string_len != RX_GRAPH_TASK_BINDING_SIZE ||
                binding_available < RX_GRAPH_TASK_BINDING_SIZE ||
                !rx_graph_task_binding(graph,
                                       symbol->string,
                                       target->kind,
                                       expected)) {
                rxbin007_set_error("RXBIN 007 cannot resolve task target in pool %u record %u",
                                   pool_index, record);
                free(patches);
                return 0;
            }
            if (!rxbin007_record_task_binding_patch(
                    &patches, &patch_count, &patch_capacity,
                    binding, expected)) {
                free(patches);
                return 0;
            }

            /* Imported task calls carry the deterministic 80-byte relocation
             * placeholder but do not own the target's META_TASK_TARGET record.
             * Reseal every matching use-site constant across all pools against
             * the final graph, just as the defining metadata binding is resealed. */
            rxbin007_task_placeholder(symbol->string, placeholder);
            for (candidate_pool_index = 0u;
                 candidate_pool_index < pool_count;
                 candidate_pool_index++) {
                rxbin007_pool *candidate_pool = &pools[candidate_pool_index];
                uint32_t candidate_record;

                for (candidate_record = 0u;
                     candidate_record < candidate_pool->entry_count;
                     candidate_record++) {
                    chameleon_constant *candidate_entry =
                            candidate_pool->entries[candidate_record].entry;
                    string_constant *candidate;
                    size_t candidate_available;

                    if (candidate_entry->type != BINARY_CONST) continue;
                    candidate = (string_constant *)candidate_entry;
                    candidate_available = candidate->base.size_in_pool -
                                          offsetof(string_constant, string);
                    if (candidate->string_len != RX_GRAPH_TASK_BINDING_SIZE ||
                        candidate_available < RX_GRAPH_TASK_BINDING_SIZE ||
                        memcmp(candidate->string, placeholder,
                               RX_GRAPH_TASK_BINDING_SIZE) != 0) continue;
                    if (!rxbin007_record_task_binding_patch(
                            &patches, &patch_count, &patch_capacity,
                            candidate, expected)) {
                        free(patches);
                        return 0;
                    }
                }
            }
        }
    }
    for (patch = 0u; patch < patch_count; patch++) {
        memcpy(patches[patch].constant->string,
               patches[patch].binding,
               RX_GRAPH_TASK_BINDING_SIZE);
    }
    free(patches);
    return 1;
}

static int rxbin007_payload_string(rxbin_byte_buffer *payload,
                                   const string_constant *entry) {
    size_t header_size;
    size_t available;

    header_size = offsetof(string_constant, string);
    if (entry->base.size_in_pool < header_size + 1u) return 0;
    available = entry->base.size_in_pool - header_size;
    if (entry->string_len > available ||
        (entry->string_len < available && entry->string[entry->string_len] != 0)) return 0;
    if (!rxbin007_u64(payload, (uint64_t)entry->string_len)) return 0;
#ifndef NUTF8
    if (!rxbin007_u64(payload, (uint64_t)entry->string_chars)) return 0;
#else
    if (!rxbin007_u64(payload, (uint64_t)entry->string_len)) return 0;
#endif
    return rxbin_byte_buffer_append_bytes(payload,
                                          (const unsigned char *)entry->string,
                                          entry->string_len);
}

static int rxbin007_payload_named(rxbin_byte_buffer *payload,
                                  const char *name,
                                  size_t available) {
    size_t length;

    length = rxbin007_bounded_strlen(name, available);
    if (length == SIZE_MAX || length > UINT32_MAX) return 0;
    return rxbin007_u32(payload, (uint32_t)length) &&
           rxbin_byte_buffer_append_bytes(payload,
                                          (const unsigned char *)name,
                                          length);
}

static int rxbin007_payload_meta_base(rxbin_byte_buffer *payload,
                                      const rxbin007_pool *pool,
                                      const meta_entry *entry) {
    uint32_t previous;
    uint32_t next;

    return rxbin007_pool_find_int_id(pool, entry->prev, &previous) &&
           rxbin007_pool_find_int_id(pool, entry->next, &next) &&
           rxbin007_u32(payload, previous) &&
           rxbin007_u32(payload, next) &&
           rxbin007_u64(payload, (uint64_t)entry->address);
}

static int rxbin007_payload_ref(rxbin_byte_buffer *payload,
                                const rxbin007_pool *pool,
                                size_t offset) {
    uint32_t id;

    return rxbin007_pool_find_id(pool, offset, &id) && rxbin007_u32(payload, id);
}

static int rxbin007_encode_entry_payload(const rxbin007_pool *pool,
                                         const rxbin007_pool_entry *indexed,
                                         rxbin_byte_buffer *payload) {
    chameleon_constant *base;

    base = indexed->entry;
    switch (base->type) {
        case STRING_CONST:
        case BINARY_CONST:
        case DECIMAL_CONST:
            return rxbin007_payload_string(payload, (const string_constant *)base);
        case FLOAT_CONST: {
            uint64_t bits;
            memcpy(&bits, &((const float_constant *)base)->double_value, sizeof(bits));
            return rxbin007_u64(payload, bits);
        }
        case PROC_CONST: {
            const proc_constant *entry = (const proc_constant *)base;
            uint32_t next;
            uint32_t exposed;
            size_t header_size = offsetof(proc_constant, name);
            if (base->size_in_pool < header_size + 1u ||
                !rxbin007_pool_find_int_id(pool, entry->next, &next) ||
                !rxbin007_pool_find_id(pool, entry->exposed, &exposed)) return 0;
            return rxbin007_u32(payload, next) &&
                   rxbin007_i32(payload, entry->locals) &&
                   rxbin007_u64(payload, (uint64_t)entry->start) &&
                   rxbin007_u32(payload, exposed) &&
                   rxbin007_payload_named(payload,
                                          entry->name,
                                          base->size_in_pool - header_size);
        }
        case EXPOSE_REG_CONST: {
            const expose_reg_constant *entry = (const expose_reg_constant *)base;
            uint32_t next;
            size_t header_size = offsetof(expose_reg_constant, index);
            if (base->size_in_pool < header_size + 1u ||
                !rxbin007_pool_find_int_id(pool, entry->next, &next)) return 0;
            return rxbin007_u32(payload, next) &&
                   rxbin007_i32(payload, entry->global_reg) &&
                   rxbin007_payload_named(payload,
                                          entry->index,
                                          base->size_in_pool - header_size);
        }
        case EXPOSE_PROC_CONST: {
            const expose_proc_constant *entry = (const expose_proc_constant *)base;
            uint32_t next;
            uint32_t procedure;
            size_t header_size = offsetof(expose_proc_constant, index);
            if (base->size_in_pool < header_size + 1u ||
                !rxbin007_pool_find_int_id(pool, entry->next, &next) ||
                !rxbin007_pool_find_id(pool, entry->procedure, &procedure)) return 0;
            return rxbin007_u32(payload, next) &&
                   rxbin007_u32(payload, procedure) &&
                   rxbin007_u32(payload, entry->imported ? 1u : 0u) &&
                   rxbin007_payload_named(payload,
                                          entry->index,
                                          base->size_in_pool - header_size);
        }
        default:
            break;
    }

    if (!rxbin007_is_metadata(base->type) ||
        !rxbin007_payload_meta_base(payload, pool, (const meta_entry *)base)) return 0;
    switch (base->type) {
        case META_FUNC: {
            const meta_func_constant *entry = (const meta_func_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->option) &&
                   rxbin007_payload_ref(payload, pool, entry->type) &&
                   rxbin007_payload_ref(payload, pool, entry->func) &&
                   rxbin007_payload_ref(payload, pool, entry->args);
        }
        case META_REG: {
            const meta_reg_constant *entry = (const meta_reg_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->option) &&
                   rxbin007_payload_ref(payload, pool, entry->type) &&
                   rxbin007_u64(payload, (uint64_t)entry->reg);
        }
        case META_CONST: {
            const meta_const_constant *entry = (const meta_const_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->option) &&
                   rxbin007_payload_ref(payload, pool, entry->type) &&
                   rxbin007_payload_ref(payload, pool, entry->constant);
        }
        case META_CLEAR:
            return rxbin007_payload_ref(payload,
                                        pool,
                                        ((const meta_clear_constant *)base)->symbol);
        case META_CLASS: {
            const meta_class_constant *entry = (const meta_class_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->option) &&
                   rxbin007_payload_ref(payload, pool, entry->type);
        }
        case META_ATTR: {
            const meta_attr_constant *entry = (const meta_attr_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->option) &&
                   rxbin007_payload_ref(payload, pool, entry->type) &&
                   rxbin007_u64(payload, (uint64_t)entry->reg);
        }
        case META_INTERFACE: {
            const meta_interface_constant *entry = (const meta_interface_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->option) &&
                   rxbin007_payload_ref(payload, pool, entry->type);
        }
        case META_IMPLEMENTS: {
            const meta_implements_constant *entry = (const meta_implements_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->interface_symbol);
        }
        case META_MEMBER: {
            const meta_member_constant *entry = (const meta_member_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->owner) &&
                   rxbin007_payload_ref(payload, pool, entry->kind) &&
                   rxbin007_payload_ref(payload, pool, entry->member) &&
                   rxbin007_payload_ref(payload, pool, entry->type) &&
                   rxbin007_payload_ref(payload, pool, entry->args);
        }
        case META_INLINE: {
            const meta_inline_constant *entry = (const meta_inline_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->payload);
        }
        case META_TASK_TARGET: {
            const meta_task_target_constant *entry =
                (const meta_task_target_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->binding) &&
                   rxbin007_u32(payload, entry->kind);
        }
        case META_PROVIDER: {
            const meta_provider_constant *entry =
                (const meta_provider_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->provider) &&
                   rxbin007_u32(payload, entry->flags);
        }
        case META_INITIALIZER: {
            const meta_initializer_constant *entry =
                (const meta_initializer_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->function);
        }
        case META_AUTOLOAD: {
            const meta_autoload_constant *entry =
                (const meta_autoload_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->artifact);
        }
        case META_SOURCE_STEP: {
            const meta_source_step_constant *entry = (const meta_source_step_constant *)base;
            return rxbin007_payload_ref(payload, pool, entry->file) &&
                   rxbin007_payload_ref(payload, pool, entry->source_line) &&
                   rxbin007_u32(payload, entry->step_id) &&
                   rxbin007_u32(payload, entry->clause_id) &&
                   rxbin007_u32(payload, entry->line) &&
                   rxbin007_u32(payload, entry->active_start_column) &&
                   rxbin007_u32(payload, entry->active_end_column) &&
                   rxbin007_u32(payload, entry->flags);
        }
        case META_TRACE_EVENT: {
            const meta_trace_event_constant *entry = (const meta_trace_event_constant *)base;
            uint32_t value_constant_id;
            uint64_t value_ref;
            if (entry->value_source == RXBIN_TRACE_VALUE_CONSTANT &&
                entry->value_ref != RXBIN_TRACE_REF_NONE) {
                if (!rxbin007_pool_find_id(pool,
                                           entry->value_ref,
                                           &value_constant_id)) return 0;
                value_ref = value_constant_id;
            } else {
                value_ref = entry->value_ref == RXBIN_TRACE_REF_NONE
                    ? UINT64_MAX : (uint64_t)entry->value_ref;
            }
            return rxbin007_u32(payload,
                                (uint32_t)entry->kind |
                                ((uint32_t)entry->value_source << 8u) |
                                ((uint32_t)entry->value_type << 16u) |
                                ((uint32_t)entry->register_type << 24u)) &&
                   rxbin007_u32(payload, entry->mode_mask) &&
                   rxbin007_u32(payload, entry->flags) &&
                   rxbin007_u64(payload, value_ref) &&
                   rxbin007_payload_ref(payload, pool, entry->symbol) &&
                   rxbin007_payload_ref(payload, pool, entry->resolved_name) &&
                   rxbin007_u32(payload, entry->source_step_id) &&
                   rxbin007_u32(payload, entry->clause_id);
        }
        default:
            return 0;
    }
}

static int rxbin007_encode_pool_sections(const rxbin007_pool *pools,
                                         uint32_t pool_count,
                                         rxbin_byte_buffer *constants,
                                         rxbin_byte_buffer *metadata) {
    uint64_t constant_count;
    uint64_t metadata_count;
    uint32_t pool_index;

    constant_count = 0u;
    metadata_count = 0u;
    for (pool_index = 0u; pool_index < pool_count; pool_index++) {
        uint32_t id;
        for (id = 0u; id < pools[pool_index].entry_count; id++) {
            if (rxbin007_is_metadata(pools[pool_index].entries[id].entry->type)) metadata_count++;
            else constant_count++;
        }
    }
    if (constant_count > UINT32_MAX || metadata_count > UINT32_MAX) return 0;
    if (!rxbin_byte_buffer_append_bytes(constants, (const unsigned char *)"RXC7", 4u) ||
        !rxbin007_u32(constants, 1u) ||
        !rxbin007_u32(constants, pool_count) ||
        !rxbin007_u32(constants, (uint32_t)constant_count) ||
        !rxbin_byte_buffer_append_bytes(metadata, (const unsigned char *)"RXD7", 4u) ||
        !rxbin007_u32(metadata, 1u) ||
        !rxbin007_u32(metadata, pool_count) ||
        !rxbin007_u32(metadata, (uint32_t)metadata_count)) return 0;

    for (pool_index = 0u; pool_index < pool_count; pool_index++) {
        uint32_t id;
        for (id = 0u; id < pools[pool_index].entry_count; id++) {
            const rxbin007_pool_entry *entry;
            rxbin_byte_buffer payload;
            rxbin_byte_buffer *section;

            entry = &pools[pool_index].entries[id];
            section = rxbin007_is_metadata(entry->entry->type) ? metadata : constants;
            rxbin_byte_buffer_init(&payload);
            if (!rxbin007_encode_entry_payload(&pools[pool_index], entry, &payload) ||
                !rxbin007_u32(section, pool_index) ||
                !rxbin007_u32(section, id) ||
                !rxbin007_u32(section, (uint32_t)entry->entry->type) ||
                !rxbin007_u32(section, 0u) ||
                !rxbin007_u64(section, (uint64_t)payload.size) ||
                !rxbin_byte_buffer_append_bytes(section, payload.data, payload.size) ||
                !rxbin007_align(section, 8u)) {
                rxbin_byte_buffer_free(&payload);
                rxbin007_set_error("RXBIN 007 cannot encode pool %u record %u (kind %u, native offset %lu)",
                                   pool_index,
                                   id,
                                   (unsigned int)entry->entry->type,
                                   (unsigned long)entry->offset);
                return 0;
            }
            rxbin_byte_buffer_free(&payload);
        }
    }
    return 1;
}

static int rxbin007_put_u32(rxbin_byte_buffer *buffer, size_t offset, uint32_t value) {
    if (!buffer || offset > buffer->size || buffer->size - offset < 4u) return 0;
    buffer->data[offset] = (unsigned char)(value & 0xffu);
    buffer->data[offset + 1u] = (unsigned char)((value >> 8u) & 0xffu);
    buffer->data[offset + 2u] = (unsigned char)((value >> 16u) & 0xffu);
    buffer->data[offset + 3u] = (unsigned char)((value >> 24u) & 0xffu);
    return 1;
}

static int rxbin007_put_u64(rxbin_byte_buffer *buffer, size_t offset, uint64_t value) {
    unsigned int i;
    if (!buffer || offset > buffer->size || buffer->size - offset < 8u) return 0;
    for (i = 0u; i < 8u; i++) {
        buffer->data[offset + i] =
            (unsigned char)((value >> (i * 8u)) & UINT64_C(0xff));
    }
    return 1;
}

static int rxbin007_pool_operand_type(OperandType type) {
    return type == OP_FUNC || type == OP_FLOAT || type == OP_STRING ||
           type == OP_DECIMAL || type == OP_BINARY;
}

static uint32_t rxbin007_opcode_features(int opcode) {
    switch (opcode) {
        case OP_CALL1_REG_FUNC_REG:
        case OP_CALL2_REG_FUNC_REG_REG:
        case OP_CALL3_REG_FUNC_REG_REG_REG:
        case OP_CALL4_REG_FUNC_REG_REG_REG_REG:
            return RXBIN007_FEATURE_FIXED_CALLS;
        case OP_PARSEWORDS3_REG_REG_REG_REG:
        case OP_PARSEPOS2_REG_REG_REG_INT:
        case OP_PARSEWORDS3D_REG_REG_REG_REG:
        case OP_PARSEPLAN_REG_REG_STRING:
            return RXBIN007_FEATURE_FROZEN_PARSE;
        case OP_STEMINIT_REG:
        case OP_STEMGET_REG_REG_REG:
        case OP_STEMSET_REG_REG_REG:
        case OP_STEMRESET_REG_REG:
        case OP_STEMGET2_REG_REG_REG_REG:
        case OP_STEMSET2_REG_REG_REG_REG:
        case OP_STEMSIZE_REG_REG:
        case OP_STEMKEYAT_REG_REG_REG:
        case OP_STEMVALUEAT_REG_REG_REG:
            return RXBIN007_FEATURE_NATIVE_STEM;
        case OP_CHANOPEN_REG_REG_REG_REG_REG:
        case OP_CHANSTART_REG_REG_REG_REG_REG:
        case OP_CHANWAIT_REG_REG_REG_REG:
        case OP_CHANCANCEL_REG_REG_REG_REG:
        case OP_CHANCLOSE_REG_REG_REG:
            return RXBIN007_FEATURE_CHANNELS;
        default:
            return 0u;
    }
}

static int rxbin007_encode_instructions(const module_file *module,
                                        const rxbin007_pool *pool,
                                        const RxGraph *graph,
                                        rxbin_byte_buffer *output,
                                        uint32_t *feature_flags) {
    const bin_code *instructions;
    rxbin_var_writer writer;
    size_t index;

    if (!module || !feature_flags ||
        (module->header.instruction_size && !module->instructions)) return 0;
    instructions = (const bin_code *)module->instructions;
    index = 0u;
    rxbin_var_writer_init(&writer, output);
    while (index < module->header.instruction_size) {
        OpFormat format;
        int opcode;
        size_t operand_count;
        size_t operand_index;

        opcode = instructions[index].instruction.opcode;
        if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return 0;
        *feature_flags |= rxbin007_opcode_features(opcode);
        format = rxbin_opcode_format(opcode);
        operand_count = rxop_format_operand_count(format);
        if (instructions[index].instruction.no_ops != operand_count ||
            index + operand_count >= module->header.instruction_size ||
            !rxbin_var_writer_write(&writer, (uint64_t)opcode)) return 0;
        for (operand_index = 0; operand_index < operand_count; operand_index++) {
            const bin_code *operand;
            RxGraphOperandKind graph_kind;
            uint64_t token;

            operand = &instructions[index + (size_t)operand_index + 1u];
            graph_kind = rx_graph_operand_kind(opcode, (unsigned int)operand_index);
            if (graph_kind != RX_GRAPH_OPERAND_NONE) {
                uint32_t graph_id;
                const char *text;
                uint32_t constant_id;
                char *graph_error;

                if (!graph || rxop_format_operand_type(format, operand_index) != OP_STRING) {
                    rxbin007_set_error("RXBIN 007 graph operand %d:%zu has no semantic graph",
                                       opcode, operand_index);
                    return 0;
                }
                if (module->graph_operands) {
                    char *graph_text;
                    if (operand->index > UINT32_MAX) {
                        rxbin007_set_error("RXBIN 007 graph operand %d:%zu is out of range",
                                           opcode, operand_index);
                        return 0;
                    }
                    graph_id = (uint32_t)operand->index;
                    graph_text = rx_graph_operand_text(graph,
                                                       opcode,
                                                       (unsigned int)operand_index,
                                                       graph_id);
                    if (!graph_text) {
                        rxbin007_set_error("RXBIN 007 graph operand %d:%zu has invalid ID %u",
                                           opcode, operand_index, graph_id);
                        return 0;
                    }
                    free(graph_text);
                    token = graph_id;
                    if (!rxbin_var_writer_write(&writer, token)) return 0;
                    continue;
                }
                if (!rxbin007_pool_find_id(pool, operand->index, &constant_id) ||
                    constant_id >= pool->entry_count ||
                    pool->entries[constant_id].entry->type != STRING_CONST) {
                    rxbin007_set_error("RXBIN 007 graph operand %d:%zu is not a string constant",
                                       opcode, operand_index);
                    return 0;
                }
                text = ((const string_constant *)pool->entries[constant_id].entry)->string;
                graph_error = 0;
                if (!rx_graph_resolve_operand(graph,
                                              opcode,
                                              (unsigned int)operand_index,
                                              text,
                                              &graph_id,
                                              &graph_error)) {
                    rxbin007_set_error("RXBIN 007 cannot resolve graph operand %d:%d: %s",
                                       opcode,
                                       operand_index,
                                       graph_error ? graph_error : "unknown graph error");
                    free(graph_error);
                    return 0;
                }
                free(graph_error);
                token = graph_id;
            } else if (rxbin007_pool_operand_type(rxop_format_operand_type(format, operand_index))) {
                uint32_t constant_id;
                if (!rxbin007_pool_find_id(pool, operand->index, &constant_id)) {
                    rxbin007_set_error("RXBIN 007 instruction has an invalid constant offset");
                    return 0;
                }
                token = constant_id;
            } else if (rxop_format_operand_type(format, operand_index) == OP_INT) {
                token = (((uint64_t)operand->iconst) << 1u) ^
                        (uint64_t)(operand->iconst >>
                                   ((sizeof(rxinteger) * CHAR_BIT) - 1u));
            } else if (rxop_format_operand_type(format, operand_index) == OP_CHAR) {
                token = (uint64_t)(unsigned char)operand->cconst;
            } else {
                token = (uint64_t)operand->index;
            }
            if (!rxbin_var_writer_write(&writer, token)) return 0;
        }
        index += (size_t)operand_count + 1u;
    }
    return rxbin_var_writer_flush(&writer);
}

static int rxbin007_build_module_section(rxbin007_module_write *modules,
                                         size_t module_count,
                                         const rxbin007_pool *pools,
                                         uint32_t pool_count,
                                         rxbin_byte_buffer *section) {
    static const unsigned char zeros[88] = {0};
    size_t records_offset;
    size_t i;

    if (!rxbin_byte_buffer_append_bytes(section, (const unsigned char *)"RXM7", 4u) ||
        !rxbin007_u32(section, 1u) ||
        !rxbin007_u32(section, (uint32_t)module_count) ||
        !rxbin007_u32(section, pool_count)) return 0;
    records_offset = section->size;
    for (i = 0u; i < module_count; i++) {
        if (!rxbin_byte_buffer_append_bytes(section, zeros, sizeof(zeros))) return 0;
    }
    for (i = 0u; i < module_count; i++) {
        module_file *module;
        const rxbin007_pool *pool;
        size_t record;
        size_t name_size;
        size_t description_size;
        uint64_t name_offset;
        uint64_t description_offset;
        uint32_t proc_head;
        uint32_t expose_head;
        uint32_t meta_head;

        module = modules[i].module;
        pool = &pools[modules[i].pool_index];
        name_size = module->name ? strlen(module->name) + 1u : 1u;
        description_size = module->description ? strlen(module->description) + 1u : 1u;
        if (name_size > (size_t)16u * 1024u * 1024u ||
            description_size > (size_t)16u * 1024u * 1024u ||
            !rxbin007_pool_find_int_id(pool, module->header.proc_head, &proc_head) ||
            !rxbin007_pool_find_int_id(pool, module->header.expose_head, &expose_head) ||
            !rxbin007_pool_find_int_id(pool, module->header.meta_head, &meta_head)) {
            rxbin007_set_error("RXBIN 007 module %lu has invalid text or pool heads",
                               (unsigned long)i);
            return 0;
        }
        name_offset = section->size;
        if (!rxbin_byte_buffer_append_bytes(section,
                                            (const unsigned char *)(module->name
                                                ? module->name : ""),
                                            name_size)) return 0;
        description_offset = section->size;
        if (!rxbin_byte_buffer_append_bytes(section,
                                            (const unsigned char *)(module->description
                                                ? module->description : ""),
                                            description_size)) return 0;
        record = records_offset + i * 88u;
        if (!rxbin007_put_u64(section, record, name_offset) ||
            !rxbin007_put_u64(section, record + 8u, name_size) ||
            !rxbin007_put_u64(section, record + 16u, description_offset) ||
            !rxbin007_put_u64(section, record + 24u, description_size) ||
            !rxbin007_put_u64(section, record + 32u, modules[i].instruction_offset) ||
            !rxbin007_put_u64(section, record + 40u, modules[i].instruction_size) ||
            !rxbin007_put_u64(section,
                              record + 48u,
                              (uint64_t)module->header.instruction_size) ||
            !rxbin007_put_u32(section, record + 56u, modules[i].pool_index) ||
            !rxbin007_put_u32(section, record + 60u, module->native ? 1u : 0u) ||
            !rxbin007_put_u32(section, record + 64u, (uint32_t)module->header.globals) ||
            !rxbin007_put_u32(section, record + 68u, proc_head) ||
            !rxbin007_put_u32(section, record + 72u, expose_head) ||
            !rxbin007_put_u32(section, record + 76u, meta_head)) return 0;
    }
    return rxbin007_align(section, 8u);
}

static void rxbin007_free_sections(rxbin007_section *sections) {
    uint32_t i;
    if (!sections) return;
    for (i = 0u; i < RXBIN007_SECTION_COUNT; i++) {
        rxbin_byte_buffer_free(&sections[i].bytes);
    }
}

static int rxbin007_pack_sections(rxbin007_section *sections) {
    uint32_t i;

    for (i = 0u; i < RXBIN007_SECTION_COUNT; i++) {
        rxbin_byte_buffer packed;

        sections[i].expanded_size = sections[i].bytes.size;
        if (!sections[i].bytes.size) continue;
        rxbin_byte_buffer_init(&packed);
        if (!rxbin007_lzss_compress(sections[i].bytes.data,
                                    sections[i].bytes.size,
                                    &packed)) {
            rxbin_byte_buffer_free(&packed);
            rxbin007_set_error("out of memory compressing RXBIN 007 section %lu",
                               (unsigned long)sections[i].kind);
            return 0;
        }
        if (packed.size < sections[i].bytes.size) {
            rxbin_byte_buffer_free(&sections[i].bytes);
            sections[i].bytes = packed;
            sections[i].flags = RXBIN007_SECTION_LZSS;
        } else {
            rxbin_byte_buffer_free(&packed);
        }
    }
    return 1;
}

static int rxbin007_write_container(rxbin007_section *sections,
                                    size_t module_count,
                                    uint32_t feature_flags,
                                    FILE *file) {
    rxbin_byte_buffer output;
    static const unsigned char zeros[304] = {0};
    uint64_t file_size;
    uint32_t i;

    rxbin_byte_buffer_init(&output);
    if (!rxbin007_pack_sections(sections)) goto error;
    if (!rxbin_byte_buffer_append_bytes(&output, zeros, sizeof(zeros))) goto error;
    for (i = 0u; i < RXBIN007_SECTION_COUNT; i++) {
        if (!rxbin007_align(&output, sections[i].alignment)) goto error;
        sections[i].file_offset = output.size;
        if (!rxbin_byte_buffer_append_bytes(&output,
                                            sections[i].bytes.data,
                                            sections[i].bytes.size)) goto error;
    }
    file_size = output.size;
    memcpy(output.data, RXBIN007_MAGIC, 8u);
    if (!rxbin007_put_u32(&output, 8u, RXBIN007_HEADER_SIZE) ||
        !rxbin007_put_u32(&output, 12u, feature_flags) ||
        !rxbin007_put_u64(&output, 16u, file_size) ||
        !rxbin007_put_u32(&output, 24u, RXBIN007_SECTION_COUNT) ||
        !rxbin007_put_u32(&output, 28u, (uint32_t)module_count) ||
        !rxbin007_put_u64(&output, 32u, RXBIN007_HEADER_SIZE)) goto error;
    for (i = 0u; i < RXBIN007_SECTION_COUNT; i++) {
        size_t row;
        row = RXBIN007_HEADER_SIZE + (size_t)i * RXBIN007_DIRECTORY_ENTRY_SIZE;
        if (!rxbin007_put_u32(&output, row, sections[i].kind) ||
            !rxbin007_put_u32(&output, row + 4u, sections[i].flags) ||
            !rxbin007_put_u32(&output, row + 8u, sections[i].alignment) ||
            !rxbin007_put_u64(&output, row + 16u, sections[i].file_offset) ||
            !rxbin007_put_u64(&output, row + 24u, sections[i].bytes.size) ||
            !rxbin007_put_u64(&output, row + 32u, sections[i].expanded_size)) goto error;
    }
    if (fwrite(output.data, 1u, output.size, file) != output.size) {
        rxbin007_set_error("RXBIN 007 cannot write the output container");
        goto error;
    }
    rxbin_byte_buffer_free(&output);
    return 1;

error:
    rxbin_byte_buffer_free(&output);
    if (!rxbin007_error[0]) rxbin007_set_error("RXBIN 007 cannot assemble the output container");
    return 0;
}

int write_modules(module_file *const *input_modules,
                  size_t module_count,
                  RxGraph *semantic_graph,
                  FILE *outFile) {
    rxbin007_module_write *modules;
    rxbin007_pool *pools;
    uint32_t pool_count;
    rxbin007_section sections[RXBIN007_SECTION_COUNT];
    RxGraph *built_graph;
    unsigned char *graph_facts;
    unsigned char *graph_indexes;
    size_t graph_facts_size;
    size_t graph_indexes_size;
    size_t i;
    uint32_t feature_flags;
    int ok;

    rxbin007_clear_error();
    if (!input_modules || !module_count || module_count > UINT32_MAX || !outFile) {
        rxbin007_set_error("RXBIN 007 writer requires one or more modules and an output file");
        return 1;
    }
    modules = (rxbin007_module_write *)calloc(module_count, sizeof(*modules));
    pools = (rxbin007_pool *)calloc(module_count, sizeof(*pools));
    if (!modules || !pools) {
        free(modules);
        free(pools);
        rxbin007_set_error("out of memory preparing RXBIN 007 image");
        return 1;
    }
    pool_count = 0u;
    for (i = 0u; i < module_count; i++) {
        uint32_t pool_index;
        module_file *module;

        module = input_modules[i];
        if (!module) {
            rxbin007_set_error("RXBIN 007 module %lu is null", (unsigned long)i);
            goto error;
        }
        for (pool_index = 0u; pool_index < pool_count; pool_index++) {
            if (pools[pool_index].data == (unsigned char *)module->constant &&
                pools[pool_index].size == module->header.constant_size) break;
        }
        if (pool_index == pool_count) {
            pools[pool_index].data = (unsigned char *)module->constant;
            pools[pool_index].size = module->header.constant_size;
            if (!rxbin007_scan_pool(&pools[pool_index],
                                    module->name ? module->name : "<unnamed>")) goto error;
            pool_count++;
        }
        modules[i].module = module;
        modules[i].pool_index = pool_index;
    }

    built_graph = 0;
    if (!semantic_graph && module_count == 1u && input_modules[0]->semantic_graph) {
        semantic_graph = input_modules[0]->semantic_graph;
    }
    if (!semantic_graph) {
        char *graph_error = 0;
        built_graph = rx_graph_build_crexx(input_modules, module_count, &graph_error);
        if (!built_graph) {
            rxbin007_set_error("RXBIN 007 cannot build semantic graph: %s",
                               graph_error ? graph_error : "unknown graph error");
            free(graph_error);
            goto error;
        }
        free(graph_error);
        semantic_graph = built_graph;
    }
    if (!rxbin007_materialize_task_bindings(pools, pool_count, semantic_graph)) {
        goto error;
    }

    memset(sections, 0, sizeof(sections));
    feature_flags = 0u;
    for (i = 0u; i < pool_count; i++) {
        uint32_t entry_index;
        for (entry_index = 0u;
             entry_index < pools[i].entry_count;
             entry_index++) {
            if (pools[i].entries[entry_index].entry->type == META_PROVIDER) {
                feature_flags |= RXBIN007_FEATURE_NATIVE_PROVIDERS;
            } else if (pools[i].entries[entry_index].entry->type ==
                       META_INITIALIZER) {
                feature_flags |= RXBIN007_FEATURE_INITIALIZERS;
            } else if (pools[i].entries[entry_index].entry->type ==
                       META_AUTOLOAD) {
                feature_flags |= RXBIN007_FEATURE_AUTOLOAD_HINTS;
            }
        }
    }
    for (i = 0u; i < RXBIN007_SECTION_COUNT; i++) {
        sections[i].kind = (uint32_t)i + 1u;
        sections[i].alignment = 8u;
        rxbin_byte_buffer_init(&sections[i].bytes);
    }
    if (!rxbin_byte_buffer_append_bytes(&sections[1].bytes,
                                        (const unsigned char *)"RXQ7",
                                        4u) ||
        !rxbin007_u32(&sections[1].bytes, 1u)) goto section_error;
    for (i = 0u; i < module_count; i++) {
        modules[i].instruction_offset = sections[1].bytes.size;
        if (!rxbin007_encode_instructions(modules[i].module,
                                          &pools[modules[i].pool_index],
                                          semantic_graph,
                                          &sections[1].bytes,
                                          &feature_flags)) goto section_error;
        modules[i].instruction_size = sections[1].bytes.size -
                                      (size_t)modules[i].instruction_offset;
    }
    if (!rxbin007_build_module_section(modules,
                                       module_count,
                                       pools,
                                       pool_count,
                                       &sections[0].bytes) ||
        !rxbin007_encode_pool_sections(pools,
                                       pool_count,
                                       &sections[2].bytes,
                                       &sections[3].bytes)) goto section_error;
    graph_facts = 0;
    graph_indexes = 0;
    graph_facts_size = 0u;
    graph_indexes_size = 0u;
    if (!rx_graph_serialize_sections(semantic_graph,
                                     &graph_facts,
                                     &graph_facts_size,
                                     &graph_indexes,
                                     &graph_indexes_size) ||
        !rxbin_byte_buffer_append_bytes(&sections[4].bytes,
                                        graph_facts,
                                        graph_facts_size) ||
        !rxbin_byte_buffer_append_bytes(&sections[5].bytes,
                                        graph_indexes,
                                        graph_indexes_size)) {
        free(graph_facts);
        free(graph_indexes);
        goto section_error;
    }
    free(graph_facts);
    free(graph_indexes);
    ok = rxbin007_write_container(sections,
                                  module_count,
                                  feature_flags,
                                  outFile);
    rxbin007_free_sections(sections);
    rx_graph_release(&built_graph);
    rxbin007_free_pools(pools, pool_count);
    free(modules);
    return ok ? 0 : 1;

section_error:
    if (!rxbin007_error[0]) rxbin007_set_error("RXBIN 007 cannot encode image sections");
    rxbin007_free_sections(sections);
    rx_graph_release(&built_graph);
error:
    rxbin007_free_pools(pools, pool_count);
    free(modules);
    return 1;
}

int write_module(module_file *module, FILE *outFile) {
    module_file *modules[1];
    modules[0] = module;
    return write_modules(modules,
                         1u,
                         module ? module->semantic_graph : 0,
                         outFile);
}

typedef struct rxbin007_record_view {
    uint32_t type;
    const unsigned char *payload;
    size_t payload_size;
    unsigned char present;
} rxbin007_record_view;

typedef struct rxbin007_pool_read {
    rxbin007_record_view *records;
    uint32_t record_count;
    uint32_t record_capacity;
    size_t *offsets;
    rxbin_shared_constant_pool *shared;
} rxbin007_pool_read;

typedef struct rxbin007_image_state {
    module_file **modules;
    size_t module_count;
} rxbin007_image_state;

typedef struct rxbin007_reader_cursor {
    const unsigned char *cursor;
    const unsigned char *end;
} rxbin007_reader_cursor;

typedef struct rxbin007_section_view {
    const unsigned char *data;
    size_t size;
    unsigned char *owned_data;
} rxbin007_section_view;

static void rxbin007_free_section_views(rxbin007_section_view *sections) {
    uint32_t i;

    if (!sections) return;
    for (i = 0u; i < RXBIN007_SECTION_COUNT; i++) free(sections[i].owned_data);
}

typedef enum rxbin007_ref_kind {
    RXBIN007_REF_ANY,
    RXBIN007_REF_STRING,
    RXBIN007_REF_BINARY,
    RXBIN007_REF_VALUE,
    RXBIN007_REF_PROCEDURE,
    RXBIN007_REF_EXPOSE,
    RXBIN007_REF_METADATA
} rxbin007_ref_kind;

static int rxbin007_reader_take(rxbin007_reader_cursor *reader,
                                void *destination,
                                size_t size) {
    if (!reader || size > (size_t)(reader->end - reader->cursor)) return 0;
    if (destination && size) memcpy(destination, reader->cursor, size);
    reader->cursor += size;
    return 1;
}

static int rxbin007_reader_u32(rxbin007_reader_cursor *reader, uint32_t *value) {
    if (!reader || !value || reader->end - reader->cursor < 4) return 0;
    *value = rxbin007_read_u32_at(reader->cursor);
    reader->cursor += 4;
    return 1;
}

static int rxbin007_reader_u64(rxbin007_reader_cursor *reader, uint64_t *value) {
    if (!reader || !value || reader->end - reader->cursor < 8) return 0;
    *value = rxbin007_read_u64_at(reader->cursor);
    reader->cursor += 8;
    return 1;
}

static int rxbin007_payload_done(const rxbin007_reader_cursor *reader) {
    return reader && reader->cursor == reader->end;
}

static int rxbin007_pool_read_grow(rxbin007_pool_read *pool, uint32_t needed) {
    rxbin007_record_view *records;
    uint32_t capacity;

    if (needed <= pool->record_capacity) return 1;
    capacity = pool->record_capacity ? pool->record_capacity : 32u;
    while (capacity < needed) {
        if (capacity > UINT32_MAX / 2u) {
            capacity = needed;
            break;
        }
        capacity *= 2u;
    }
    if ((size_t)capacity > SIZE_MAX / sizeof(*records)) return 0;
    records = (rxbin007_record_view *)realloc(pool->records,
                                              (size_t)capacity * sizeof(*records));
    if (!records) return 0;
    memset(records + pool->record_capacity,
           0,
           (size_t)(capacity - pool->record_capacity) * sizeof(*records));
    pool->records = records;
    pool->record_capacity = capacity;
    return 1;
}

static void rxbin007_free_pool_reads(rxbin007_pool_read *pools,
                                     uint32_t pool_count,
                                     int release_shared) {
    uint32_t i;

    if (!pools) return;
    for (i = 0u; i < pool_count; i++) {
        free(pools[i].records);
        free(pools[i].offsets);
        if (release_shared && pools[i].shared) {
            free(pools[i].shared->data);
            free(pools[i].shared);
        }
    }
    free(pools);
}

static int rxbin007_parse_record_section(const rxbin007_section_view *section,
                                         const char magic[4],
                                         int metadata,
                                         uint32_t maximum_pools,
                                         uint32_t maximum_records,
                                         rxbin007_pool_read **pools_ref,
                                         uint32_t *pool_count_ref) {
    rxbin007_reader_cursor reader;
    unsigned char actual_magic[4];
    uint32_t version;
    uint32_t pool_count;
    uint32_t record_count;
    uint32_t i;
    rxbin007_pool_read *pools;

    if (!section || section->size < 16u) return 0;
    reader.cursor = section->data;
    reader.end = section->data + section->size;
    if (!rxbin007_reader_take(&reader, actual_magic, sizeof(actual_magic)) ||
        memcmp(actual_magic, magic, 4u) != 0 ||
        !rxbin007_reader_u32(&reader, &version) || version != 1u ||
        !rxbin007_reader_u32(&reader, &pool_count) ||
        !rxbin007_reader_u32(&reader, &record_count) || !pool_count ||
        pool_count > maximum_pools || record_count > maximum_records) return 0;
    if (!*pools_ref) {
        pools = (rxbin007_pool_read *)calloc(pool_count, sizeof(*pools));
        if (!pools) return 0;
        *pools_ref = pools;
        *pool_count_ref = pool_count;
    } else if (*pool_count_ref != pool_count) {
        return 0;
    }
    pools = *pools_ref;
    for (i = 0u; i < record_count; i++) {
        uint32_t pool_index;
        uint32_t id;
        uint32_t type;
        uint32_t flags;
        uint64_t payload_size;
        size_t consumed;
        size_t padding;
        rxbin007_record_view *record;

        if (!rxbin007_reader_u32(&reader, &pool_index) ||
            !rxbin007_reader_u32(&reader, &id) ||
            !rxbin007_reader_u32(&reader, &type) ||
            !rxbin007_reader_u32(&reader, &flags) ||
            !rxbin007_reader_u64(&reader, &payload_size) ||
            pool_index >= pool_count || id == RXBIN007_NONE || id >= maximum_records ||
            type > META_AUTOLOAD || flags ||
            metadata != rxbin007_is_metadata((enum const_pool_type)type) ||
            payload_size > (uint64_t)(reader.end - reader.cursor) ||
            !rxbin007_pool_read_grow(&pools[pool_index], id + 1u)) return 0;
        record = &pools[pool_index].records[id];
        if (record->present) return 0;
        record->type = type;
        record->payload = reader.cursor;
        record->payload_size = (size_t)payload_size;
        record->present = 1u;
        if (pools[pool_index].record_count <= id) pools[pool_index].record_count = id + 1u;
        reader.cursor += (size_t)payload_size;
        consumed = (size_t)(reader.cursor - section->data);
        padding = (8u - (consumed & 7u)) & 7u;
        if (padding > (size_t)(reader.end - reader.cursor)) return 0;
        while (padding) {
            if (*reader.cursor++ != 0u) return 0;
            padding--;
        }
    }
    return reader.cursor == reader.end;
}

static int rxbin007_pool_reads_have_type(const rxbin007_pool_read *pools,
                                         uint32_t pool_count,
                                         enum const_pool_type type) {
    uint32_t pool_index;

    for (pool_index = 0u; pool_index < pool_count; pool_index++) {
        uint32_t record_index;
        for (record_index = 0u;
             record_index < pools[pool_index].record_count;
             record_index++) {
            const rxbin007_record_view *record =
                &pools[pool_index].records[record_index];
            if (record->present && record->type == (uint32_t)type) return 1;
        }
    }
    return 0;
}

static size_t rxbin007_native_size(const rxbin007_record_view *record) {
    rxbin007_reader_cursor reader;
    uint64_t byte_length;
    uint64_t ignored;
    uint32_t name_length;
    size_t base_size;
    size_t result;

    if (!record || !record->present) return 0u;
    reader.cursor = record->payload;
    reader.end = record->payload + record->payload_size;
    switch ((enum const_pool_type)record->type) {
        case STRING_CONST:
        case BINARY_CONST:
        case DECIMAL_CONST:
            if (!rxbin007_reader_u64(&reader, &byte_length) ||
                !rxbin007_reader_u64(&reader, &ignored) ||
                byte_length > (uint64_t)(reader.end - reader.cursor) ||
                byte_length != (uint64_t)(reader.end - reader.cursor) ||
                byte_length > SIZE_MAX - offsetof(string_constant, string) - 1u) return 0u;
            result = offsetof(string_constant, string) + (size_t)byte_length + 1u;
            break;
        case FLOAT_CONST:
            if (record->payload_size != 8u) return 0u;
            result = sizeof(float_constant);
            break;
        case PROC_CONST:
            if (record->payload_size < 24u) return 0u;
            reader.cursor += 20u;
            if (!rxbin007_reader_u32(&reader, &name_length) ||
                name_length != (uint32_t)(reader.end - reader.cursor) ||
                (size_t)name_length > SIZE_MAX - offsetof(proc_constant, name) - 1u) return 0u;
            result = offsetof(proc_constant, name) + (size_t)name_length + 1u;
            break;
        case EXPOSE_REG_CONST:
            if (record->payload_size < 12u) return 0u;
            reader.cursor += 8u;
            if (!rxbin007_reader_u32(&reader, &name_length) ||
                name_length != (uint32_t)(reader.end - reader.cursor) ||
                (size_t)name_length > SIZE_MAX - offsetof(expose_reg_constant, index) - 1u) return 0u;
            result = offsetof(expose_reg_constant, index) + (size_t)name_length + 1u;
            break;
        case EXPOSE_PROC_CONST:
            if (record->payload_size < 16u) return 0u;
            reader.cursor += 12u;
            if (!rxbin007_reader_u32(&reader, &name_length) ||
                name_length != (uint32_t)(reader.end - reader.cursor) ||
                (size_t)name_length > SIZE_MAX - offsetof(expose_proc_constant, index) - 1u) return 0u;
            result = offsetof(expose_proc_constant, index) + (size_t)name_length + 1u;
            break;
        case META_FUNC: base_size = sizeof(meta_func_constant); goto metadata;
        case META_REG: base_size = sizeof(meta_reg_constant); goto metadata;
        case META_CONST: base_size = sizeof(meta_const_constant); goto metadata;
        case META_CLEAR: base_size = sizeof(meta_clear_constant); goto metadata;
        case META_CLASS: base_size = sizeof(meta_class_constant); goto metadata;
        case META_ATTR: base_size = sizeof(meta_attr_constant); goto metadata;
        case META_INTERFACE: base_size = sizeof(meta_interface_constant); goto metadata;
        case META_IMPLEMENTS: base_size = sizeof(meta_implements_constant); goto metadata;
        case META_MEMBER: base_size = sizeof(meta_member_constant); goto metadata;
        case META_INLINE: base_size = sizeof(meta_inline_constant); goto metadata;
        case META_SOURCE_STEP: base_size = sizeof(meta_source_step_constant); goto metadata;
        case META_TRACE_EVENT: base_size = sizeof(meta_trace_event_constant); goto metadata;
        case META_TASK_TARGET: base_size = sizeof(meta_task_target_constant); goto metadata;
        case META_PROVIDER: base_size = sizeof(meta_provider_constant); goto metadata;
        case META_INITIALIZER: base_size = sizeof(meta_initializer_constant); goto metadata;
        case META_AUTOLOAD: base_size = sizeof(meta_autoload_constant); goto metadata;
        default:
            return 0u;
metadata:
            result = base_size;
            break;
    }
    if (result > SIZE_MAX - 7u) return 0u;
    return (result + 7u) & ~(size_t)7u;
}

static int rxbin007_record_matches_ref_kind(const rxbin007_record_view *record,
                                            rxbin007_ref_kind kind) {
    enum const_pool_type type;

    if (!record || !record->present) return 0;
    type = (enum const_pool_type)record->type;
    switch (kind) {
        case RXBIN007_REF_ANY:
            return 1;
        case RXBIN007_REF_STRING:
            return type == STRING_CONST;
        case RXBIN007_REF_BINARY:
            return type == BINARY_CONST;
        case RXBIN007_REF_VALUE:
            return type == STRING_CONST || type == BINARY_CONST ||
                   type == DECIMAL_CONST || type == FLOAT_CONST;
        case RXBIN007_REF_PROCEDURE:
            return type == PROC_CONST;
        case RXBIN007_REF_EXPOSE:
            return type == EXPOSE_REG_CONST || type == EXPOSE_PROC_CONST;
        case RXBIN007_REF_METADATA:
            return rxbin007_is_metadata(type);
        default:
            return 0;
    }
}

static int rxbin007_id_offset_kind(const rxbin007_pool_read *pool,
                                   uint32_t id,
                                   rxbin007_ref_kind kind,
                                   int allow_none,
                                   size_t *offset) {
    if (!offset) return 0;
    if (id == RXBIN007_NONE) {
        if (!allow_none) return 0;
        *offset = SIZE_MAX;
        return 1;
    }
    if (!pool || id >= pool->record_count ||
        !rxbin007_record_matches_ref_kind(&pool->records[id], kind)) return 0;
    *offset = pool->offsets[id];
    return 1;
}

static int rxbin007_id_int_offset_kind(const rxbin007_pool_read *pool,
                                       uint32_t id,
                                       rxbin007_ref_kind kind,
                                       int allow_none,
                                       int *offset) {
    size_t value;
    if (!rxbin007_id_offset_kind(pool, id, kind, allow_none, &value) ||
        (value != SIZE_MAX && value > INT_MAX)) return 0;
    *offset = value == SIZE_MAX ? -1 : (int)value;
    return 1;
}

static int rxbin007_reader_ref_kind(rxbin007_reader_cursor *reader,
                                    const rxbin007_pool_read *pool,
                                    rxbin007_ref_kind kind,
                                    int allow_none,
                                    size_t *offset) {
    uint32_t id;
    return rxbin007_reader_u32(reader, &id) &&
           rxbin007_id_offset_kind(pool, id, kind, allow_none, offset);
}

static int rxbin007_fill_meta_base(rxbin007_reader_cursor *reader,
                                   const rxbin007_pool_read *pool,
                                   meta_entry *entry) {
    uint32_t previous;
    uint32_t next;
    uint64_t address;

    if (!rxbin007_reader_u32(reader, &previous) ||
        !rxbin007_reader_u32(reader, &next) ||
        !rxbin007_reader_u64(reader, &address) ||
        address > SIZE_MAX ||
        !rxbin007_id_int_offset_kind(pool, previous, RXBIN007_REF_METADATA, 1,
                                     &entry->prev) ||
        !rxbin007_id_int_offset_kind(pool, next, RXBIN007_REF_METADATA, 1,
                                     &entry->next)) return 0;
    entry->address = (size_t)address;
    return 1;
}

static int rxbin007_fill_record(rxbin007_pool_read *pool, uint32_t id) {
    const rxbin007_record_view *record;
    rxbin007_reader_cursor reader;
    unsigned char *destination;
    chameleon_constant *base;
    size_t native_size;

    record = &pool->records[id];
    native_size = rxbin007_native_size(record);
    if (!native_size) return 0;
    destination = pool->shared->data + pool->offsets[id];
    base = (chameleon_constant *)destination;
    base->size_in_pool = native_size;
    base->type = (enum const_pool_type)record->type;
    reader.cursor = record->payload;
    reader.end = record->payload + record->payload_size;

    switch (base->type) {
        case STRING_CONST:
        case BINARY_CONST:
        case DECIMAL_CONST: {
            string_constant *entry = (string_constant *)base;
            uint64_t byte_length;
            uint64_t char_length;
            if (!rxbin007_reader_u64(&reader, &byte_length) ||
                !rxbin007_reader_u64(&reader, &char_length) ||
                byte_length > SIZE_MAX || char_length > SIZE_MAX ||
                byte_length != (uint64_t)(reader.end - reader.cursor)) return 0;
            entry->string_len = (size_t)byte_length;
#ifndef NUTF8
            entry->string_chars = (size_t)char_length;
#endif
            memcpy(entry->string, reader.cursor, (size_t)byte_length);
            entry->string[byte_length] = 0;
            reader.cursor += (size_t)byte_length;
            return rxbin007_payload_done(&reader);
        }
        case FLOAT_CONST: {
            uint64_t bits;
            if (!rxbin007_reader_u64(&reader, &bits) || !rxbin007_payload_done(&reader)) return 0;
            memcpy(&((float_constant *)base)->double_value, &bits, sizeof(bits));
            return 1;
        }
        case PROC_CONST: {
            proc_constant *entry = (proc_constant *)base;
            uint32_t next;
            uint32_t locals;
            uint64_t start;
            uint32_t exposed;
            uint32_t name_length;
            size_t exposed_offset;
            if (!rxbin007_reader_u32(&reader, &next) ||
                !rxbin007_reader_u32(&reader, &locals) ||
                !rxbin007_reader_u64(&reader, &start) || start > SIZE_MAX ||
                !rxbin007_reader_u32(&reader, &exposed) ||
                !rxbin007_reader_u32(&reader, &name_length) ||
                name_length != (uint32_t)(reader.end - reader.cursor) ||
                !rxbin007_id_int_offset_kind(pool, next, RXBIN007_REF_PROCEDURE, 1,
                                             &entry->next) ||
                !rxbin007_id_offset_kind(pool, exposed, RXBIN007_REF_EXPOSE, 1,
                                         &exposed_offset)) return 0;
            entry->locals = (int32_t)locals;
            entry->start = (size_t)start;
            entry->exposed = exposed_offset;
            memcpy(entry->name, reader.cursor, name_length);
            entry->name[name_length] = 0;
            return 1;
        }
        case EXPOSE_REG_CONST: {
            expose_reg_constant *entry = (expose_reg_constant *)base;
            uint32_t next;
            uint32_t global_reg;
            uint32_t name_length;
            if (!rxbin007_reader_u32(&reader, &next) ||
                !rxbin007_reader_u32(&reader, &global_reg) ||
                !rxbin007_reader_u32(&reader, &name_length) ||
                name_length != (uint32_t)(reader.end - reader.cursor) ||
                !rxbin007_id_int_offset_kind(pool, next, RXBIN007_REF_EXPOSE, 1,
                                             &entry->next)) return 0;
            entry->global_reg = (int32_t)global_reg;
            memcpy(entry->index, reader.cursor, name_length);
            entry->index[name_length] = 0;
            return 1;
        }
        case EXPOSE_PROC_CONST: {
            expose_proc_constant *entry = (expose_proc_constant *)base;
            uint32_t next;
            uint32_t procedure;
            uint32_t imported;
            uint32_t name_length;
            size_t procedure_offset;
            if (!rxbin007_reader_u32(&reader, &next) ||
                !rxbin007_reader_u32(&reader, &procedure) ||
                !rxbin007_reader_u32(&reader, &imported) || imported > 1u ||
                !rxbin007_reader_u32(&reader, &name_length) ||
                name_length != (uint32_t)(reader.end - reader.cursor) ||
                !rxbin007_id_int_offset_kind(pool, next, RXBIN007_REF_EXPOSE, 1,
                                             &entry->next) ||
                !rxbin007_id_offset_kind(pool, procedure, RXBIN007_REF_PROCEDURE, 0,
                                         &procedure_offset)) return 0;
            entry->procedure = procedure_offset;
            entry->imported = imported ? 1u : 0u;
            memcpy(entry->index, reader.cursor, name_length);
            entry->index[name_length] = 0;
            return 1;
        }
        default:
            break;
    }

    if (!rxbin007_fill_meta_base(&reader, pool, (meta_entry *)base)) return 0;
    switch (base->type) {
        case META_FUNC: {
            meta_func_constant *entry = (meta_func_constant *)base;
            return rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->symbol) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->option) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->type) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_PROCEDURE, 0,
                                            &entry->func) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->args) &&
                   rxbin007_payload_done(&reader);
        }
        case META_REG: {
            meta_reg_constant *entry = (meta_reg_constant *)base;
            uint64_t reg;
            if (!rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                          &entry->symbol) ||
                !rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                          &entry->option) ||
                !rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                          &entry->type) ||
                !rxbin007_reader_u64(&reader, &reg) || reg > SIZE_MAX ||
                !rxbin007_payload_done(&reader)) return 0;
            entry->reg = (size_t)reg;
            return 1;
        }
        case META_CONST: {
            meta_const_constant *entry = (meta_const_constant *)base;
            return rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->symbol) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->option) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->type) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_VALUE, 0,
                                            &entry->constant) &&
                   rxbin007_payload_done(&reader);
        }
        case META_CLEAR:
            return rxbin007_reader_ref_kind(&reader,
                                            pool,
                                            RXBIN007_REF_STRING,
                                            0,
                                            &((meta_clear_constant *)base)->symbol) &&
                   rxbin007_payload_done(&reader);
        case META_CLASS: {
            meta_class_constant *entry = (meta_class_constant *)base;
            return rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->symbol) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->option) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->type) &&
                   rxbin007_payload_done(&reader);
        }
        case META_ATTR: {
            meta_attr_constant *entry = (meta_attr_constant *)base;
            uint64_t reg;
            if (!rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                          &entry->symbol) ||
                !rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                          &entry->option) ||
                !rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                          &entry->type) ||
                !rxbin007_reader_u64(&reader, &reg) || reg > SIZE_MAX ||
                !rxbin007_payload_done(&reader)) return 0;
            entry->reg = (size_t)reg;
            return 1;
        }
        case META_INTERFACE: {
            meta_interface_constant *entry = (meta_interface_constant *)base;
            return rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->symbol) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->option) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->type) &&
                   rxbin007_payload_done(&reader);
        }
        case META_IMPLEMENTS: {
            meta_implements_constant *entry = (meta_implements_constant *)base;
            return rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->symbol) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->interface_symbol) &&
                   rxbin007_payload_done(&reader);
        }
        case META_MEMBER: {
            meta_member_constant *entry = (meta_member_constant *)base;
            return rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->owner) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->kind) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->member) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->type) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->args) &&
                   rxbin007_payload_done(&reader);
        }
        case META_INLINE: {
            meta_inline_constant *entry = (meta_inline_constant *)base;
            return rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->symbol) &&
                   rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                            &entry->payload) &&
                   rxbin007_payload_done(&reader);
        }
        case META_TASK_TARGET: {
            meta_task_target_constant *entry = (meta_task_target_constant *)base;
            if (!rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                          &entry->symbol) ||
                !rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_BINARY, 0,
                                          &entry->binding) ||
                !rxbin007_reader_u32(&reader, &entry->kind) ||
                entry->kind < 1u || entry->kind > 3u ||
                !rxbin007_payload_done(&reader)) return 0;
            return 1;
        }
        case META_PROVIDER: {
            meta_provider_constant *entry = (meta_provider_constant *)base;
            if (!rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                          &entry->symbol) ||
                !rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                          &entry->provider) ||
                !rxbin007_reader_u32(&reader, &entry->flags) ||
                (entry->flags & ~RXBIN_PROVIDER_KNOWN_FLAGS) != 0u ||
                !rxbin007_payload_done(&reader)) return 0;
            return 1;
        }
        case META_INITIALIZER: {
            meta_initializer_constant *entry =
                (meta_initializer_constant *)base;
            return rxbin007_reader_ref_kind(&reader, pool,
                                            RXBIN007_REF_STRING, 0,
                                            &entry->symbol) &&
                   rxbin007_reader_ref_kind(&reader, pool,
                                            RXBIN007_REF_PROCEDURE, 0,
                                            &entry->function) &&
                   rxbin007_payload_done(&reader);
        }
        case META_AUTOLOAD: {
            meta_autoload_constant *entry =
                (meta_autoload_constant *)base;
            return rxbin007_reader_ref_kind(&reader, pool,
                                            RXBIN007_REF_STRING, 0,
                                            &entry->symbol) &&
                   rxbin007_reader_ref_kind(&reader, pool,
                                            RXBIN007_REF_STRING, 0,
                                            &entry->artifact) &&
                   rxbin007_payload_done(&reader);
        }
        case META_SOURCE_STEP: {
            meta_source_step_constant *entry = (meta_source_step_constant *)base;
            uint64_t ignored;
            if (!rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                          &entry->file) ||
                !rxbin007_reader_ref_kind(&reader, pool, RXBIN007_REF_STRING, 0,
                                          &entry->source_line) ||
                !rxbin007_reader_u32(&reader, &entry->step_id) ||
                !rxbin007_reader_u32(&reader, &entry->clause_id) ||
                !rxbin007_reader_u32(&reader, &entry->line) ||
                !rxbin007_reader_u32(&reader, &entry->active_start_column) ||
                !rxbin007_reader_u32(&reader, &entry->active_end_column) ||
                !rxbin007_reader_u32(&reader, &entry->flags)) return 0;
            ignored = 0u;
            (void)ignored;
            return rxbin007_payload_done(&reader);
        }
        case META_TRACE_EVENT: {
            meta_trace_event_constant *entry = (meta_trace_event_constant *)base;
            uint32_t packed;
            uint64_t value_ref;
            uint32_t symbol;
            uint32_t resolved_name;
            size_t value_offset;
            if (!rxbin007_reader_u32(&reader, &packed) ||
                !rxbin007_reader_u32(&reader, &entry->mode_mask) ||
                !rxbin007_reader_u32(&reader, &entry->flags) ||
                !rxbin007_reader_u64(&reader, &value_ref) ||
                !rxbin007_reader_u32(&reader, &symbol) ||
                !rxbin007_reader_u32(&reader, &resolved_name) ||
                !rxbin007_reader_u32(&reader, &entry->source_step_id) ||
                !rxbin007_reader_u32(&reader, &entry->clause_id) ||
                !rxbin007_payload_done(&reader) ||
                !rxbin007_id_offset_kind(pool, symbol, RXBIN007_REF_STRING, 1,
                                         &entry->symbol) ||
                !rxbin007_id_offset_kind(pool, resolved_name, RXBIN007_REF_STRING, 1,
                                         &entry->resolved_name)) return 0;
            entry->kind = (uint8_t)(packed & 0xffu);
            entry->value_source = (uint8_t)((packed >> 8u) & 0xffu);
            entry->value_type = (uint8_t)((packed >> 16u) & 0xffu);
            entry->register_type = (uint8_t)((packed >> 24u) & 0xffu);
            if (value_ref == UINT64_MAX) {
                entry->value_ref = RXBIN_TRACE_REF_NONE;
            } else if (entry->value_source == RXBIN_TRACE_VALUE_CONSTANT) {
                if (value_ref > UINT32_MAX ||
                    !rxbin007_id_offset_kind(pool,
                                             (uint32_t)value_ref,
                                             RXBIN007_REF_VALUE,
                                             0,
                                             &value_offset)) return 0;
                entry->value_ref = value_offset;
            } else {
                if (value_ref > SIZE_MAX) return 0;
                entry->value_ref = (size_t)value_ref;
            }
            return 1;
        }
        default:
            return 0;
    }
}

static int rxbin007_materialize_pool(rxbin007_pool_read *pool) {
    size_t total_size;
    uint32_t id;

    if (!pool) return 0;
    for (id = 0u; id < pool->record_count; id++) {
        if (!pool->records[id].present) return 0;
    }
    pool->offsets = pool->record_count
        ? (size_t *)calloc(pool->record_count, sizeof(*pool->offsets)) : 0;
    if (pool->record_count && !pool->offsets) return 0;
    total_size = 0u;
    for (id = 0u; id < pool->record_count; id++) {
        size_t native_size;
        native_size = rxbin007_native_size(&pool->records[id]);
        if (!native_size || total_size > SIZE_MAX - native_size) return 0;
        pool->offsets[id] = total_size;
        total_size += native_size;
    }
    pool->shared = (rxbin_shared_constant_pool *)calloc(1u, sizeof(*pool->shared));
    if (!pool->shared) return 0;
    pool->shared->data = total_size ? (unsigned char *)calloc(total_size, 1u) : 0;
    if (total_size && !pool->shared->data) return 0;
    pool->shared->size = total_size;
    pool->shared->stored_size = total_size;
    pool->shared->refcount = 0u;
    for (id = 0u; id < pool->record_count; id++) {
        if (!rxbin007_fill_record(pool, id)) return 0;
    }
    return 1;
}

static int rxbin007_pool_id_has_type(const rxbin007_pool_read *pool,
                                     uint32_t id,
                                     enum const_pool_type type) {
    return pool && id < pool->record_count && pool->records[id].present &&
           pool->records[id].type == (uint32_t)type;
}

static int rxbin007_pool_operand_id_valid(const rxbin007_pool_read *pool,
                                          OperandType operand_type,
                                          uint32_t id) {
    enum const_pool_type expected;

    switch (operand_type) {
        case OP_FUNC: expected = PROC_CONST; break;
        case OP_FLOAT: expected = FLOAT_CONST; break;
        case OP_STRING: expected = STRING_CONST; break;
        case OP_DECIMAL: expected = DECIMAL_CONST; break;
        case OP_BINARY: expected = BINARY_CONST; break;
        default: return 0;
    }
    return rxbin007_pool_id_has_type(pool, id, expected);
}

static int rxbin007_graph_id_valid(const RxGraph *graph,
                                   int opcode,
                                   unsigned int operand_index,
                                   uint32_t id) {
    char *text;

    text = rx_graph_operand_text(graph, opcode, operand_index, id);
    if (!text) return 0;
    free(text);
    return 1;
}

static int rxbin007_decode_instructions(const unsigned char *data,
                                        size_t byte_size,
                                        size_t word_count,
                                        const rxbin007_pool_read *pool,
                                        const RxGraph *graph,
                                        uint32_t feature_flags,
                                        bin_code **instructions_out) {
    rxbin_var_reader reader;
    bin_code *instructions;
    size_t index;

    if (!instructions_out || (byte_size && !data) ||
        word_count > SIZE_MAX / sizeof(*instructions)) return 0;
    *instructions_out = 0;
    instructions = word_count
        ? (bin_code *)calloc(word_count, sizeof(*instructions)) : 0;
    if (word_count && !instructions) return 0;
    rxbin_var_reader_init(&reader, data, byte_size);
    index = 0u;
    while (index < word_count) {
        OpFormat format;
        uint64_t opcode_token;
        size_t operand_count;
        size_t operand_index;

        if (!rxbin_var_reader_read(&reader, &opcode_token) ||
            opcode_token >= (uint64_t)OP_MAX_INSTRUCTIONS) goto error;
        if ((rxbin007_opcode_features((int)opcode_token) & feature_flags) !=
            rxbin007_opcode_features((int)opcode_token)) {
            rxbin007_set_error(
                    "RXBIN 007 opcode %llu requires feature flag 0x%08x",
                    (unsigned long long)opcode_token,
                    (unsigned int)rxbin007_opcode_features((int)opcode_token));
            goto error;
        }
        format = rxbin_opcode_format((int)opcode_token);
        operand_count = rxop_format_operand_count(format);
        if (operand_count > INT_MAX || index + operand_count >= word_count) goto error;
        instructions[index].instruction.opcode = (int)opcode_token;
        instructions[index].instruction.no_ops = (int)operand_count;
        for (operand_index = 0; operand_index < operand_count; operand_index++) {
            bin_code *operand;
            RxGraphOperandKind graph_kind;
            uint64_t token;

            operand = &instructions[index + (size_t)operand_index + 1u];
            if (!rxbin_var_reader_read(&reader, &token)) goto error;
            graph_kind = rx_graph_operand_kind((int)opcode_token,
                                               (unsigned int)operand_index);
            if (graph_kind != RX_GRAPH_OPERAND_NONE) {
                if (token > UINT32_MAX ||
                    !rxbin007_graph_id_valid(graph,
                                             (int)opcode_token,
                                             (unsigned int)operand_index,
                                             (uint32_t)token)) goto error;
                operand->index = (size_t)token;
            } else if (rxbin007_pool_operand_type(rxop_format_operand_type(format, operand_index))) {
                if (token > UINT32_MAX ||
                    !rxbin007_pool_operand_id_valid(pool,
                                                    rxop_format_operand_type(format, operand_index),
                                                    (uint32_t)token)) goto error;
                operand->index = pool->offsets[(uint32_t)token];
            } else if (rxop_format_operand_type(format, operand_index) == OP_INT) {
                uint64_t magnitude;
                magnitude = (token >> 1u) ^ (UINT64_C(0) - (token & 1u));
                operand->iconst = (rxinteger)magnitude;
            } else if (rxop_format_operand_type(format, operand_index) == OP_CHAR) {
                if (token > UCHAR_MAX) goto error;
                operand->cconst = (char)(unsigned char)token;
            } else {
                if (token > SIZE_MAX) goto error;
                operand->index = (size_t)token;
            }
        }
        index += (size_t)operand_count + 1u;
    }
    if (reader.cursor != reader.end || reader.have_queued_value) goto error;
    *instructions_out = instructions;
    return 1;

error:
    free(instructions);
    return 0;
}

static int rxbin007_section_text(const rxbin007_section_view *section,
                                 uint64_t offset,
                                 uint64_t size,
                                 char **text) {
    const unsigned char *source;

    if (!section || !text || !size || size > (uint64_t)16u * 1024u * 1024u ||
        offset > section->size || size > (uint64_t)(section->size - (size_t)offset)) return 0;
    source = section->data + (size_t)offset;
    if (source[(size_t)size - 1u] != 0u ||
        memchr(source, 0, (size_t)size - 1u) != 0) return 0;
    *text = (char *)malloc((size_t)size);
    if (!*text) return 0;
    memcpy(*text, source, (size_t)size);
    return 1;
}

static int rxbin007_head_offset(const rxbin007_pool_read *pool,
                                uint32_t id,
                                rxbin007_ref_kind kind,
                                int *offset) {
    size_t native_offset;

    if (!offset) return 0;
    if (id == RXBIN007_NONE) {
        *offset = -1;
        return 1;
    }
    if (!rxbin007_id_offset_kind(pool, id, kind, 0, &native_offset)) return 0;
    if (native_offset > INT_MAX) return 0;
    *offset = (int)native_offset;
    return 1;
}

static int rxbin007_parse_modules(const rxbin007_section_view *module_section,
                                  const rxbin007_section_view *instruction_section,
                                  uint32_t expected_module_count,
                                  rxbin007_pool_read *pools,
                                  uint32_t pool_count,
                                  RxGraph *graph,
                                  uint32_t feature_flags,
                                  rxbin007_image_state **state_out) {
    rxbin007_image_state *state;
    uint32_t schema;
    uint32_t module_count;
    uint32_t declared_pool_count;
    uint32_t i;
    size_t record_bytes;

    if (!state_out || !module_section || module_section->size < 16u ||
        !instruction_section || instruction_section->size < 8u ||
        memcmp(module_section->data, "RXM7", 4u) != 0 ||
        memcmp(instruction_section->data, "RXQ7", 4u) != 0) return 0;
    schema = rxbin007_read_u32_at(module_section->data + 4u);
    module_count = rxbin007_read_u32_at(module_section->data + 8u);
    declared_pool_count = rxbin007_read_u32_at(module_section->data + 12u);
    if (schema != 1u || module_count != expected_module_count ||
        declared_pool_count != pool_count ||
        rxbin007_read_u32_at(instruction_section->data + 4u) != 1u ||
        (uint64_t)module_count * 88u > module_section->size - 16u) return 0;
    record_bytes = (size_t)module_count * 88u;
    if (module_section->size < 16u + record_bytes) return 0;
    state = (rxbin007_image_state *)calloc(1u, sizeof(*state));
    if (!state) return 0;
    state->modules = (module_file **)calloc(module_count, sizeof(*state->modules));
    if (!state->modules) {
        free(state);
        return 0;
    }
    state->module_count = module_count;
    for (i = 0u; i < module_count; i++) {
        const unsigned char *record;
        uint64_t name_offset;
        uint64_t name_size;
        uint64_t description_offset;
        uint64_t description_size;
        uint64_t instruction_offset;
        uint64_t instruction_size;
        uint64_t instruction_words;
        uint32_t pool_index;
        uint32_t flags;
        uint32_t globals;
        uint32_t proc_head;
        uint32_t expose_head;
        uint32_t meta_head;
        module_file *module;

        record = module_section->data + 16u + (size_t)i * 88u;
        name_offset = rxbin007_read_u64_at(record);
        name_size = rxbin007_read_u64_at(record + 8u);
        description_offset = rxbin007_read_u64_at(record + 16u);
        description_size = rxbin007_read_u64_at(record + 24u);
        instruction_offset = rxbin007_read_u64_at(record + 32u);
        instruction_size = rxbin007_read_u64_at(record + 40u);
        instruction_words = rxbin007_read_u64_at(record + 48u);
        pool_index = rxbin007_read_u32_at(record + 56u);
        flags = rxbin007_read_u32_at(record + 60u);
        globals = rxbin007_read_u32_at(record + 64u);
        proc_head = rxbin007_read_u32_at(record + 68u);
        expose_head = rxbin007_read_u32_at(record + 72u);
        meta_head = rxbin007_read_u32_at(record + 76u);
        if (rxbin007_read_u64_at(record + 80u) != 0u || flags & ~1u ||
            pool_index >= pool_count || instruction_offset < 8u ||
            instruction_offset > instruction_section->size ||
            instruction_size > (uint64_t)(instruction_section->size -
                                            (size_t)instruction_offset) ||
            instruction_words > SIZE_MAX) goto error;
        module = (module_file *)calloc(1u, sizeof(*module));
        if (!module) goto error;
        init_module(module);
        module->fromfile = 1u;
        module->native = (unsigned char)(flags & 1u);
        module->graph_operands = 1u;
        module->semantic_module_index = i;
        module->header.record_type = module_count > 1u
            ? RXBIN_RECORD_MODULE_SHARED : RXBIN_RECORD_MODULE_LOCAL;
        module->header.globals = (int)(int32_t)globals;
        module->header.instruction_size = (size_t)instruction_words;
        module->header.instruction_stored_size = (size_t)instruction_size;
        module->header.constant_size = pools[pool_index].shared->size;
        module->header.constant_stored_size = pools[pool_index].shared->stored_size;
        if (!rxbin007_section_text(module_section, name_offset, name_size, &module->name) ||
            !rxbin007_section_text(module_section,
                                   description_offset,
                                   description_size,
                                   &module->description) ||
            !rxbin007_head_offset(&pools[pool_index], proc_head,
                                  RXBIN007_REF_PROCEDURE,
                                  &module->header.proc_head) ||
            !rxbin007_head_offset(&pools[pool_index], expose_head,
                                  RXBIN007_REF_EXPOSE,
                                  &module->header.expose_head) ||
            !rxbin007_head_offset(&pools[pool_index], meta_head,
                                  RXBIN007_REF_METADATA,
                                  &module->header.meta_head) ||
            !rxbin007_decode_instructions(instruction_section->data +
                                              (size_t)instruction_offset,
                                          (size_t)instruction_size,
                                          (size_t)instruction_words,
                                          &pools[pool_index],
                                          graph,
                                          feature_flags,
                                          (bin_code **)&module->instructions)) {
            free_module(module);
            goto error;
        }
        module->header.name_size = (size_t)name_size;
        module->header.description_size = (size_t)description_size;
        module->constant = pools[pool_index].shared->data;
        module->shared_constant_pool = pools[pool_index].shared;
        module->shared_constant_pool->refcount++;
        module->semantic_graph = graph;
        rx_graph_retain(graph);
        state->modules[i] = module;
    }
    for (i = 0u; i < pool_count; i++) {
        if (!pools[i].shared || !pools[i].shared->refcount) goto error;
    }
    *state_out = state;
    return 1;

error:
    for (i = 0u; i < state->module_count; i++) free_module(state->modules[i]);
    free(state->modules);
    free(state);
    return 0;
}

static int rxbin007_zero_bytes(const unsigned char *data, size_t size) {
    while (size) {
        if (*data++) return 0;
        size--;
    }
    return 1;
}

static void rxbin007_free_image_state(rxbin007_image_state *state) {
    size_t i;

    if (!state) return;
    for (i = 0u; i < state->module_count; i++) free_module(state->modules[i]);
    free(state->modules);
    free(state);
}

static int rxbin007_parse_image(const unsigned char *image,
                                size_t image_size,
                                rxbin007_image_state **state_out) {
    rxbin007_section_view sections[RXBIN007_SECTION_COUNT];
    uint64_t declared_file_size;
    uint64_t directory_offset;
    uint32_t feature_flags;
    uint32_t section_count;
    uint32_t module_count;
    size_t previous_end;
    uint32_t i;
    uint32_t constant_records;
    uint32_t metadata_records;
    uint64_t total_records;
    rxbin007_pool_read *pools;
    uint32_t pool_count;
    RxGraph *graph;
    char *graph_error;
    rxbin007_image_state *state;

    if (state_out) *state_out = 0;
    if (!state_out || !image || image_size < RXBIN007_HEADER_SIZE ||
        memcmp(image, RXBIN007_MAGIC, 8u) != 0 ||
        rxbin007_read_u32_at(image + 8u) != RXBIN007_HEADER_SIZE ||
        !rxbin007_zero_bytes(image + 40u, 24u)) return 0;
    feature_flags = rxbin007_read_u32_at(image + 12u);
    if (feature_flags & ~RXBIN007_SUPPORTED_FEATURES) {
        rxbin007_set_error("RXBIN 007 has unsupported feature flags 0x%08x",
                           (unsigned int)(feature_flags &
                                          ~RXBIN007_SUPPORTED_FEATURES));
        return 0;
    }
    declared_file_size = rxbin007_read_u64_at(image + 16u);
    section_count = rxbin007_read_u32_at(image + 24u);
    module_count = rxbin007_read_u32_at(image + 28u);
    directory_offset = rxbin007_read_u64_at(image + 32u);
    if (declared_file_size != image_size || section_count != RXBIN007_SECTION_COUNT ||
        !module_count || directory_offset != RXBIN007_HEADER_SIZE ||
        directory_offset > image_size ||
        RXBIN007_SECTION_COUNT >
            (image_size - (size_t)directory_offset) / RXBIN007_DIRECTORY_ENTRY_SIZE) return 0;
    memset(sections, 0, sizeof(sections));
    pools = 0;
    pool_count = 0u;
    graph = 0;
    graph_error = 0;
    state = 0;
    previous_end = RXBIN007_HEADER_SIZE +
                   RXBIN007_SECTION_COUNT * RXBIN007_DIRECTORY_ENTRY_SIZE;
    for (i = 0u; i < RXBIN007_SECTION_COUNT; i++) {
        const unsigned char *row;
        uint32_t kind;
        uint32_t flags;
        uint32_t alignment;
        uint32_t reserved;
        uint64_t offset;
        uint64_t stored_size;
        uint64_t expanded_size;

        row = image + (size_t)directory_offset +
              (size_t)i * RXBIN007_DIRECTORY_ENTRY_SIZE;
        kind = rxbin007_read_u32_at(row);
        flags = rxbin007_read_u32_at(row + 4u);
        alignment = rxbin007_read_u32_at(row + 8u);
        reserved = rxbin007_read_u32_at(row + 12u);
        offset = rxbin007_read_u64_at(row + 16u);
        stored_size = rxbin007_read_u64_at(row + 24u);
        expanded_size = rxbin007_read_u64_at(row + 32u);
        if (kind != i + 1u ||
            (flags & ~RXBIN007_SECTION_LZSS) ||
            alignment != 8u || reserved ||
            offset > SIZE_MAX || stored_size > SIZE_MAX || expanded_size > SIZE_MAX ||
            (offset & 7u) ||
            offset < previous_end || offset > image_size ||
            stored_size > image_size - (size_t)offset ||
            !rxbin007_zero_bytes(image + previous_end,
                                 (size_t)offset - previous_end)) goto error;
        if (flags & RXBIN007_SECTION_LZSS) {
            if (!stored_size || stored_size >= expanded_size ||
                (stored_size <= UINT64_MAX / RXBIN007_LZSS_MAX_MATCH &&
                 expanded_size > stored_size * RXBIN007_LZSS_MAX_MATCH)) goto error;
            sections[i].owned_data = (unsigned char *)malloc((size_t)expanded_size);
            if (!sections[i].owned_data) {
                rxbin007_set_error("out of memory expanding RXBIN 007 section %lu",
                                   (unsigned long)kind);
                goto error;
            }
            if (!rxbin007_lzss_decompress(image + (size_t)offset,
                                          (size_t)stored_size,
                                          sections[i].owned_data,
                                          (size_t)expanded_size)) {
                rxbin007_set_error("RXBIN 007 section %lu has invalid compressed data",
                                   (unsigned long)kind);
                goto error;
            }
            sections[i].data = sections[i].owned_data;
            sections[i].size = (size_t)expanded_size;
        } else {
            if (stored_size != expanded_size) goto error;
            sections[i].data = image + (size_t)offset;
            sections[i].size = (size_t)stored_size;
        }
        previous_end = (size_t)offset + (size_t)stored_size;
    }
    if (previous_end != image_size || sections[RXBIN007_SECTION_CONSTANTS - 1u].size < 16u ||
        sections[RXBIN007_SECTION_METADATA - 1u].size < 16u) goto error;
    constant_records = rxbin007_read_u32_at(
        sections[RXBIN007_SECTION_CONSTANTS - 1u].data + 12u);
    metadata_records = rxbin007_read_u32_at(
        sections[RXBIN007_SECTION_METADATA - 1u].data + 12u);
    total_records = (uint64_t)constant_records + metadata_records;
    if (total_records > UINT32_MAX) goto error;

    graph = rx_graph_deserialize_sections(
        sections[RXBIN007_SECTION_GRAPH_FACTS - 1u].data,
        sections[RXBIN007_SECTION_GRAPH_FACTS - 1u].size,
        sections[RXBIN007_SECTION_GRAPH_INDEXES - 1u].data,
        sections[RXBIN007_SECTION_GRAPH_INDEXES - 1u].size,
        &graph_error);
    if (!graph) {
        rxbin007_set_error("RXBIN 007 semantic graph is invalid: %s",
                           graph_error ? graph_error : "unknown graph error");
        free(graph_error);
        graph_error = 0;
        goto error;
    }
    free(graph_error);
    graph_error = 0;
    if (!rxbin007_parse_record_section(
            &sections[RXBIN007_SECTION_CONSTANTS - 1u],
            "RXC7",
            0,
            module_count,
            (uint32_t)total_records,
            &pools,
            &pool_count) ||
        !rxbin007_parse_record_section(
            &sections[RXBIN007_SECTION_METADATA - 1u],
            "RXD7",
            1,
            module_count,
            (uint32_t)total_records,
            &pools,
            &pool_count)) goto error;
    if (rxbin007_pool_reads_have_type(pools, pool_count, META_PROVIDER) &&
        !(feature_flags & RXBIN007_FEATURE_NATIVE_PROVIDERS)) {
        rxbin007_set_error(
                "RXBIN 007 provider metadata requires feature flag 0x%08x",
                (unsigned int)RXBIN007_FEATURE_NATIVE_PROVIDERS);
        goto error;
    }
    if (rxbin007_pool_reads_have_type(pools, pool_count, META_INITIALIZER) &&
        !(feature_flags & RXBIN007_FEATURE_INITIALIZERS)) {
        rxbin007_set_error(
                "RXBIN 007 initializer metadata requires feature flag 0x%08x",
                (unsigned int)RXBIN007_FEATURE_INITIALIZERS);
        goto error;
    }
    if (rxbin007_pool_reads_have_type(pools, pool_count, META_AUTOLOAD) &&
        !(feature_flags & RXBIN007_FEATURE_AUTOLOAD_HINTS)) {
        rxbin007_set_error(
                "RXBIN 007 autoload metadata requires feature flag 0x%08x",
                (unsigned int)RXBIN007_FEATURE_AUTOLOAD_HINTS);
        goto error;
    }
    for (i = 0u; i < pool_count; i++) {
        if (!rxbin007_materialize_pool(&pools[i])) goto error;
    }
    if (!rxbin007_parse_modules(&sections[RXBIN007_SECTION_MODULES - 1u],
                                &sections[RXBIN007_SECTION_INSTRUCTIONS - 1u],
                                module_count,
                                pools,
                                pool_count,
                                graph,
                                feature_flags,
                                &state)) goto error;
    rxbin007_free_pool_reads(pools, pool_count, 0);
    rx_graph_release(&graph);
    rxbin007_free_section_views(sections);
    *state_out = state;
    return 1;

error:
    free(graph_error);
    rxbin007_free_image_state(state);
    rxbin007_free_pool_reads(pools, pool_count, 1);
    rx_graph_release(&graph);
    rxbin007_free_section_views(sections);
    if (!rxbin007_error[0]) rxbin007_set_error("RXBIN 007 container validation failed");
    return 0;
}

static int rxbin007_load_reader_image(rxbin_reader *reader) {
    unsigned char header[RXBIN007_HEADER_SIZE];
    unsigned char *image;
    uint64_t declared_size;
    size_t available;
    size_t read_size;

    if (!reader) return -1;
    rxbin007_clear_error();
    if (reader->from_memory) {
        if (!reader->buffer_cursor || !*reader->buffer_cursor ||
            !reader->buffer_end || *reader->buffer_cursor > reader->buffer_end) return -1;
        available = (size_t)(reader->buffer_end - *reader->buffer_cursor);
        if (!available) return 1;
        if (available < 8u) {
            rxbin007_set_error("RXBIN 007 memory image has a truncated magic");
            return -1;
        }
        if (memcmp(*reader->buffer_cursor, RXBIN007_MAGIC, 8u) != 0) return 2;
        if (available < RXBIN007_HEADER_SIZE) {
            rxbin007_set_error("RXBIN 007 memory image has a truncated header");
            return -1;
        }
        declared_size = rxbin007_read_u64_at(
            (const unsigned char *)*reader->buffer_cursor + 16u);
        if (declared_size < RXBIN007_HEADER_SIZE ||
            declared_size > available || declared_size > SIZE_MAX) {
            rxbin007_set_error("RXBIN 007 memory image has an invalid file size");
            return -1;
        }
        image = (unsigned char *)malloc((size_t)declared_size);
        if (!image) {
            rxbin007_set_error("out of memory reading RXBIN 007 image");
            return -1;
        }
        memcpy(image, *reader->buffer_cursor, (size_t)declared_size);
        *reader->buffer_cursor += (size_t)declared_size;
    } else {
        if (!reader->file) return -1;
        read_size = fread(header, 1u, sizeof(header), reader->file);
        if (!read_size && feof(reader->file)) return 1;
        if (read_size < sizeof(header)) {
            rxbin007_set_error("RXBIN 007 file has a truncated header");
            return -1;
        }
        if (memcmp(header, RXBIN007_MAGIC, 8u) != 0) return 2;
        declared_size = rxbin007_read_u64_at(header + 16u);
        if (declared_size < sizeof(header) || declared_size > SIZE_MAX) {
            rxbin007_set_error("RXBIN 007 file has an invalid file size");
            return -1;
        }
        {
            long current_position;
            current_position = ftell(reader->file);
            if (current_position >= 0 && fseek(reader->file, 0, SEEK_END) == 0) {
                long end_position;
                int restore_failed;
                end_position = ftell(reader->file);
                restore_failed = fseek(reader->file, current_position, SEEK_SET) != 0;
                if (restore_failed || end_position < current_position ||
                    declared_size - sizeof(header) >
                        (uint64_t)(end_position - current_position)) {
                    rxbin007_set_error("RXBIN 007 file is shorter than its declared size");
                    return -1;
                }
            }
        }
        image = (unsigned char *)malloc((size_t)declared_size);
        if (!image) {
            rxbin007_set_error("out of memory reading RXBIN 007 image");
            return -1;
        }
        memcpy(image, header, sizeof(header));
        read_size = (size_t)declared_size - sizeof(header);
        if (read_size && fread(image + sizeof(header), 1u, read_size, reader->file) != read_size) {
            free(image);
            rxbin007_set_error("RXBIN 007 file is shorter than its declared size");
            return -1;
        }
    }
    if (!rxbin007_parse_image(image,
                              (size_t)declared_size,
                              (rxbin007_image_state **)&reader->image_state)) {
        free(image);
        return -1;
    }
    free(image);
    reader->next_module = 0u;
    return 0;
}

void rxbin_reader_init_file(rxbin_reader *reader, FILE *inFile) {
    if (!reader) return;
    memset(reader, 0, sizeof(*reader));
    reader->file = inFile;
}

void rxbin_reader_init_mem(rxbin_reader *reader,
                           char **in_buffer,
                           const char *end_of_buffer) {
    if (!reader) return;
    memset(reader, 0, sizeof(*reader));
    reader->from_memory = 1u;
    reader->buffer_cursor = in_buffer;
    reader->buffer_end = end_of_buffer;
}

void rxbin_reader_close(rxbin_reader *reader) {
    if (!reader) return;
    rxbin007_free_image_state((rxbin007_image_state *)reader->image_state);
    memset(reader, 0, sizeof(*reader));
}

int rxbin_reader_next_module(rxbin_reader *reader, module_file **module) {
    rxbin007_image_state *state;
    int rc;

    if (!reader || !module) return -1;
    *module = 0;
    state = (rxbin007_image_state *)reader->image_state;
    if (state && reader->next_module >= state->module_count) {
        rxbin007_free_image_state(state);
        reader->image_state = 0;
        reader->next_module = 0u;
    }
    if (!reader->image_state) {
        rc = rxbin007_load_reader_image(reader);
        if (rc != 0) return rc;
    }
    state = (rxbin007_image_state *)reader->image_state;
    if (reader->next_module >= state->module_count) return 1;
    *module = state->modules[reader->next_module];
    state->modules[reader->next_module] = 0;
    reader->next_module++;
    return 0;
}

typedef struct rxbin007_file_reader_state {
    FILE *file;
    rxbin_reader reader;
    struct rxbin007_file_reader_state *next;
} rxbin007_file_reader_state;

typedef struct rxbin007_mem_reader_state {
    char **cursor_ref;
    rxbin_reader reader;
    struct rxbin007_mem_reader_state *next;
} rxbin007_mem_reader_state;

static rxbin007_file_reader_state *rxbin007_file_readers;
static rxbin007_mem_reader_state *rxbin007_mem_readers;

static void rxbin007_close_file_reader(FILE *file) {
    rxbin007_file_reader_state **link;

    link = &rxbin007_file_readers;
    while (*link) {
        if ((*link)->file == file) {
            rxbin007_file_reader_state *state;
            state = *link;
            *link = state->next;
            rxbin_reader_close(&state->reader);
            free(state);
            return;
        }
        link = &(*link)->next;
    }
}

static void rxbin007_close_mem_reader(char **cursor_ref) {
    rxbin007_mem_reader_state **link;

    link = &rxbin007_mem_readers;
    while (*link) {
        if ((*link)->cursor_ref == cursor_ref) {
            rxbin007_mem_reader_state *state;
            state = *link;
            *link = state->next;
            rxbin_reader_close(&state->reader);
            free(state);
            return;
        }
        link = &(*link)->next;
    }
}

int read_module(module_file **module, FILE *inFile) {
    rxbin007_file_reader_state *state;
    int rc;

    if (!module || !inFile) return -1;
    state = rxbin007_file_readers;
    while (state && state->file != inFile) state = state->next;
    if (!state) {
        state = (rxbin007_file_reader_state *)calloc(1u, sizeof(*state));
        if (!state) return -1;
        state->file = inFile;
        rxbin_reader_init_file(&state->reader, inFile);
        state->next = rxbin007_file_readers;
        rxbin007_file_readers = state;
    }
    rc = rxbin_reader_next_module(&state->reader, module);
    if (rc != 0) rxbin007_close_file_reader(inFile);
    return rc;
}

int read_module_mem(module_file **module,
                    char **in_buffer,
                    const char *end_of_buffer) {
    rxbin007_mem_reader_state *state;
    int rc;

    if (!module || !in_buffer) return -1;
    state = rxbin007_mem_readers;
    while (state && state->cursor_ref != in_buffer) state = state->next;
    if (!state) {
        state = (rxbin007_mem_reader_state *)calloc(1u, sizeof(*state));
        if (!state) return -1;
        state->cursor_ref = in_buffer;
        rxbin_reader_init_mem(&state->reader, in_buffer, end_of_buffer);
        state->next = rxbin007_mem_readers;
        rxbin007_mem_readers = state;
    }
    rc = rxbin_reader_next_module(&state->reader, module);
    if (rc != 0) rxbin007_close_mem_reader(in_buffer);
    return rc;
}
