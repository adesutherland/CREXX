/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CREXX_RXVMSTEM_H
#define CREXX_RXVMSTEM_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "rxvmvars.h"

#ifndef RXSTEM_PREP_BINARY_BUFFER
#define RXSTEM_PREP_BINARY_BUFFER prep_binary_buffer
#endif
#ifndef RXSTEM_RESERVE_BINARY_BUFFER
#define RXSTEM_RESERVE_BINARY_BUFFER reserve_binary_buffer
#endif
#ifndef RXSTEM_TRY_SET_NUM_ATTRIBUTES
#define RXSTEM_TRY_SET_NUM_ATTRIBUTES try_set_num_attributes
#endif
#ifndef RXSTEM_MALLOC
#define RXSTEM_MALLOC RXVM_VALUE_MALLOC
#endif
#ifndef RXSTEM_FREE
#define RXSTEM_FREE RXVM_VALUE_FREE
#endif

/*
 * Private mutable stem representation.
 *
 * The receiver's ordinary binary slot owns the fixed-width metadata below.
 * It is process-local value state and is never serialized into RXBIN. Keys,
 * values, and the current default remain ordinary VM-owned values:
 *
 *   attribute 0: insertion-ordered key array
 *   attribute 1: insertion-ordered value array
 *   attribute 2: current default string
 *
 * Bucket and next indexes are one-based; zero is the chain sentinel. Entry
 * records stay in insertion order. All integers in the binary are explicitly
 * little-endian so copy/move behavior is independent of host layout.
 */

enum rxstem_result {
    RXSTEM_OK = 0,
    RXSTEM_OUT_OF_MEMORY = -1,
    RXSTEM_CORRUPT = -2,
    RXSTEM_OVERFLOW = -3,
    RXSTEM_INVALID_INDEX = -4
};

enum {
    RXSTEM_HEADER_SIZE = 64,
    RXSTEM_BUCKET_COUNT = 256,
    RXSTEM_BUCKET_BYTES = RXSTEM_BUCKET_COUNT * 4,
    RXSTEM_ENTRY_SIZE = 16,
    RXSTEM_INITIAL_CAPACITY = 16,
    RXSTEM_KEYS_ATTRIBUTE = 0,
    RXSTEM_VALUES_ATTRIBUTE = 1,
    RXSTEM_DEFAULT_ATTRIBUTE = 2,
    RXSTEM_ATTRIBUTE_COUNT = 3
};

#define RXSTEM_MAGIC UINT32_C(0x31485453) /* "STH1" */
#define RXSTEM_VERSION UINT32_C(1)

typedef struct rxstem_header {
    uint64_t generation;
    uint32_t count;
    uint32_t capacity;
} rxstem_header;

typedef struct rxstem_entry {
    uint32_t hash;
    uint32_t next;
    uint64_t generation;
} rxstem_entry;

typedef struct rxstem_key_parts {
    const char *left;
    size_t left_length;
    const char *right;
    size_t right_length;
    int segmented;
} rxstem_key_parts;

RX_INLINE uint32_t rxstem_load_u32(const unsigned char *p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

RX_INLINE uint64_t rxstem_load_u64(const unsigned char *p) {
    return (uint64_t)rxstem_load_u32(p) |
           ((uint64_t)rxstem_load_u32(p + 4) << 32);
}

RX_INLINE void rxstem_store_u32(unsigned char *p, uint32_t v) {
    p[0] = (unsigned char)v;
    p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16);
    p[3] = (unsigned char)(v >> 24);
}

RX_INLINE void rxstem_store_u64(unsigned char *p, uint64_t v) {
    rxstem_store_u32(p, (uint32_t)v);
    rxstem_store_u32(p + 4, (uint32_t)(v >> 32));
}

RX_INLINE size_t rxstem_entries_base(void) {
    return RXSTEM_HEADER_SIZE + RXSTEM_BUCKET_BYTES;
}

RX_INLINE int rxstem_layout_size(uint32_t capacity, size_t *total) {
    size_t base = rxstem_entries_base();
    if ((size_t)capacity > (SIZE_MAX - base) / RXSTEM_ENTRY_SIZE)
        return RXSTEM_OVERFLOW;
    *total = base + (size_t)capacity * RXSTEM_ENTRY_SIZE;
    return RXSTEM_OK;
}

RX_INLINE void rxstem_store_header(value *stem, const rxstem_header *header) {
    unsigned char *p = (unsigned char *)stem->binary_value;
    rxstem_store_u32(p + 0, RXSTEM_MAGIC);
    rxstem_store_u32(p + 4, RXSTEM_VERSION);
    rxstem_store_u64(p + 8, header->generation);
    rxstem_store_u32(p + 16, header->count);
    rxstem_store_u32(p + 20, header->capacity);
    rxstem_store_u32(p + 24, RXSTEM_BUCKET_COUNT);
    rxstem_store_u32(p + 28, RXSTEM_ENTRY_SIZE);
}

RX_INLINE void rxstem_load_header_fields(value *stem, rxstem_header *header) {
    const unsigned char *p = (const unsigned char *)stem->binary_value;
    header->generation = rxstem_load_u64(p + 8);
    header->count = rxstem_load_u32(p + 16);
    header->capacity = rxstem_load_u32(p + 20);
}

RX_INLINE uint32_t rxstem_bucket_get(value *stem, uint32_t bucket) {
    return rxstem_load_u32((const unsigned char *)stem->binary_value +
                           RXSTEM_HEADER_SIZE + (size_t)bucket * 4);
}

RX_INLINE void rxstem_bucket_set(value *stem, uint32_t bucket, uint32_t raw) {
    rxstem_store_u32((unsigned char *)stem->binary_value +
                     RXSTEM_HEADER_SIZE + (size_t)bucket * 4, raw);
}

RX_INLINE void rxstem_entry_load(value *stem, uint32_t index,
                                 rxstem_entry *entry) {
    const unsigned char *p = (const unsigned char *)stem->binary_value +
                             rxstem_entries_base() +
                             (size_t)index * RXSTEM_ENTRY_SIZE;
    entry->hash = rxstem_load_u32(p + 0);
    entry->next = rxstem_load_u32(p + 4);
    entry->generation = rxstem_load_u64(p + 8);
}

RX_INLINE void rxstem_entry_store(value *stem, uint32_t index,
                                  const rxstem_entry *entry) {
    unsigned char *p = (unsigned char *)stem->binary_value +
                       rxstem_entries_base() +
                       (size_t)index * RXSTEM_ENTRY_SIZE;
    rxstem_store_u32(p + 0, entry->hash);
    rxstem_store_u32(p + 4, entry->next);
    rxstem_store_u64(p + 8, entry->generation);
}

RX_INLINE rxstem_key_parts rxstem_one_part(const value *key) {
    rxstem_key_parts parts;
    parts.left = key->string_value;
    parts.left_length = key->string_length;
    parts.right = 0;
    parts.right_length = 0;
    parts.segmented = 0;
    return parts;
}

RX_INLINE rxstem_key_parts rxstem_two_parts(const value *left,
                                            const value *right) {
    rxstem_key_parts parts;
    parts.left = left->string_value;
    parts.left_length = left->string_length;
    parts.right = right->string_value;
    parts.right_length = right->string_length;
    parts.segmented = 1;
    return parts;
}

RX_INLINE int rxstem_parts_length(const rxstem_key_parts *parts,
                                  size_t *length) {
    size_t separator = parts->segmented ? 1u : 0u;
    if (parts->left_length > SIZE_MAX - separator) return RXSTEM_OVERFLOW;
    if (parts->right_length >
        SIZE_MAX - parts->left_length - separator) return RXSTEM_OVERFLOW;
    *length = parts->left_length + separator + parts->right_length;
    return RXSTEM_OK;
}

RX_INLINE uint32_t rxstem_hash_parts(const rxstem_key_parts *parts) {
    uint32_t hash = UINT32_C(2166136261);
    size_t i;

    for (i = 0; i < parts->left_length; i++) {
        hash ^= (uint8_t)parts->left[i];
        hash *= UINT32_C(16777619);
    }
    if (parts->segmented) {
        hash ^= (uint8_t)'.';
        hash *= UINT32_C(16777619);
        for (i = 0; i < parts->right_length; i++) {
            hash ^= (uint8_t)parts->right[i];
            hash *= UINT32_C(16777619);
        }
    }
    return hash;
}

RX_INLINE int rxstem_parts_equal(const rxstem_key_parts *parts,
                                 const char *stored,
                                 size_t stored_length) {
    size_t length;
    if (rxstem_parts_length(parts, &length) != RXSTEM_OK ||
        length != stored_length) return 0;
    if (parts->left_length &&
        memcmp(parts->left, stored, parts->left_length) != 0) return 0;
    if (!parts->segmented) return 1;
    if (stored[parts->left_length] != '.') return 0;
    return !parts->right_length ||
           memcmp(parts->right,
                  stored + parts->left_length + 1,
                  parts->right_length) == 0;
}

RX_INLINE int rxstem_set_parts_string(value *dest,
                                      const rxstem_key_parts *parts) {
    size_t length;
    size_t buffer_length = RXVM_VALUE_STRING_CAPACITY(dest);
    char *buffer = dest->string_value;
    char *new_buffer = 0;
    int result = rxstem_parts_length(parts, &length);
    if (result != RXSTEM_OK) return result;
    if (!rxvm_value_string_metric_fits(length)) return RXSTEM_OVERFLOW;

    if (length > buffer_length) {
        buffer_length = buffer_size(length);
        if (buffer_length < length) return RXSTEM_OVERFLOW;
        new_buffer = RXSTEM_MALLOC(buffer_length);
        if (!new_buffer) return RXSTEM_OUT_OF_MEMORY;
        buffer = new_buffer;
    }

    if (parts->left_length)
        memcpy(buffer, parts->left, parts->left_length);
    if (parts->segmented) {
        buffer[parts->left_length] = '.';
        if (parts->right_length)
            memcpy(buffer + parts->left_length + 1,
                   parts->right,
                   parts->right_length);
    }

    if (dest->reference_payload) rxvm_reference_value_release_payload(dest);
    if (rxvm_value_native_ops(dest)) clear_binary_payload(dest);
    if (new_buffer) {
        if (RXVM_VALUE_STRING_IS_ALLOCATED(dest))
            RXSTEM_FREE(dest->string_value);
        dest->string_value = new_buffer;
        RXVM_VALUE_SET_STRING_CAPACITY(dest, buffer_length);
    }
    rxvm_value_set_string_length_known(dest, length);
    string_cache_reset(dest);
#ifndef NUTF8
    refresh_utf8_flags(dest);
#else
    clear_vm_private_flags(dest);
#endif
    return RXSTEM_OK;
}

RX_INLINE int rxstem_set_value_string(value *dest, const value *source) {
    size_t length;
    size_t buffer_length;
    char *new_buffer = 0;

    if (dest == source) return RXSTEM_OK;
    length = source->string_length;
    buffer_length = RXVM_VALUE_STRING_CAPACITY(dest);
    if (length > buffer_length) {
        buffer_length = buffer_size(length);
        if (buffer_length < length) return RXSTEM_OVERFLOW;
        new_buffer = RXSTEM_MALLOC(buffer_length);
        if (!new_buffer) return RXSTEM_OUT_OF_MEMORY;
        if (length) memcpy(new_buffer, source->string_value, length);
    }

    if (dest->reference_payload) rxvm_reference_value_release_payload(dest);
    if (rxvm_value_native_ops(dest)) clear_binary_payload(dest);
    if (new_buffer) {
        if (RXVM_VALUE_STRING_IS_ALLOCATED(dest))
            RXSTEM_FREE(dest->string_value);
        dest->string_value = new_buffer;
        RXVM_VALUE_SET_STRING_CAPACITY(dest, buffer_length);
    }
    else if (length) {
        memmove(dest->string_value, source->string_value, length);
    }
    rxvm_value_set_string_length_known(dest, length);
    string_cache_reset(dest);
#ifndef NUTF8
    dest->string_chars = source->string_chars;
    copy_vm_private_flags(dest, source);
#else
    clear_vm_private_flags(dest);
#endif
    return RXSTEM_OK;
}

RX_INLINE int rxstem_init(value *stem) {
    rxstem_header header;
    size_t total;
    int result;

    if (!stem || stem->binary_length) return RXSTEM_CORRUPT;
    result = rxstem_layout_size(RXSTEM_INITIAL_CAPACITY, &total);
    if (result != RXSTEM_OK) return result;
    if (RXSTEM_PREP_BINARY_BUFFER(stem, total) != 0)
        return RXSTEM_OUT_OF_MEMORY;

    memset(stem->binary_value, 0, total);
    if (RXSTEM_TRY_SET_NUM_ATTRIBUTES(stem, RXSTEM_ATTRIBUTE_COUNT) != 0) {
        stem->binary_length = 0;
        return RXSTEM_OUT_OF_MEMORY;
    }
    header.generation = 0;
    header.count = 0;
    header.capacity = RXSTEM_INITIAL_CAPACITY;
    rxstem_store_header(stem, &header);
    return RXSTEM_OK;
}

RX_INLINE int rxstem_load(value *stem, rxstem_header *header) {
    const unsigned char *p;
    size_t total;

    if (!stem || !header) return RXSTEM_CORRUPT;
    if (!stem->binary_length) {
        int result = rxstem_init(stem);
        if (result != RXSTEM_OK) return result;
    }
    if (stem->binary_length < RXSTEM_HEADER_SIZE ||
        stem->num_attributes != RXSTEM_ATTRIBUTE_COUNT) return RXSTEM_CORRUPT;
    p = (const unsigned char *)stem->binary_value;
    if (rxstem_load_u32(p + 0) != RXSTEM_MAGIC ||
        rxstem_load_u32(p + 4) != RXSTEM_VERSION ||
        rxstem_load_u32(p + 24) != RXSTEM_BUCKET_COUNT ||
        rxstem_load_u32(p + 28) != RXSTEM_ENTRY_SIZE)
        return RXSTEM_CORRUPT;

    rxstem_load_header_fields(stem, header);
    if (header->capacity < RXSTEM_INITIAL_CAPACITY ||
        header->count > header->capacity ||
        rxstem_layout_size(header->capacity, &total) != RXSTEM_OK ||
        total != stem->binary_length ||
        stem->attributes[RXSTEM_KEYS_ATTRIBUTE]->num_attributes < header->count ||
        stem->attributes[RXSTEM_VALUES_ATTRIBUTE]->num_attributes < header->count)
        return RXSTEM_CORRUPT;
    return RXSTEM_OK;
}

RX_INLINE int rxstem_ensure_capacity(value *stem,
                                     rxstem_header *header,
                                     uint32_t needed) {
    uint32_t old_capacity = header->capacity;
    uint32_t new_capacity = old_capacity;
    size_t total;

    while (new_capacity < needed) {
        if (new_capacity > UINT32_MAX / 2) return RXSTEM_OVERFLOW;
        new_capacity *= 2;
    }
    if (new_capacity == old_capacity) return RXSTEM_OK;
    if (rxstem_layout_size(new_capacity, &total) != RXSTEM_OK)
        return RXSTEM_OVERFLOW;
    if (RXSTEM_RESERVE_BINARY_BUFFER(stem, total) != 0)
        return RXSTEM_OUT_OF_MEMORY;

    memset(stem->binary_value + rxstem_entries_base() +
               (size_t)old_capacity * RXSTEM_ENTRY_SIZE,
           0,
           (size_t)(new_capacity - old_capacity) * RXSTEM_ENTRY_SIZE);
    stem->binary_length = total;
    header->capacity = new_capacity;
    rxstem_store_header(stem, header);
    return RXSTEM_OK;
}

RX_INLINE int rxstem_find(value *stem,
                          const rxstem_header *header,
                          const rxstem_key_parts *parts,
                          uint32_t hash,
                          uint32_t *found) {
    uint32_t raw = rxstem_bucket_get(stem, hash & 255u);
    uint32_t traversed = 0;
    value *keys = stem->attributes[RXSTEM_KEYS_ATTRIBUTE];

    *found = UINT32_MAX;
    while (raw) {
        uint32_t index = raw - 1;
        rxstem_entry entry;
        value *key;

        if (++traversed > header->count ||
            index >= header->count || index >= keys->num_attributes)
            return RXSTEM_CORRUPT;
        rxstem_entry_load(stem, index, &entry);
        key = keys->attributes[index];
        if (entry.hash == hash &&
            rxstem_parts_equal(parts, key->string_value, key->string_length)) {
            *found = index;
            return RXSTEM_OK;
        }
        raw = entry.next;
    }
    return RXSTEM_OK;
}

RX_INLINE int rxstem_get_parts(value *out,
                               value *stem,
                               const rxstem_key_parts *parts) {
    rxstem_header header;
    rxstem_entry entry;
    uint32_t hash;
    uint32_t found;
    value *source;
    int result = rxstem_load(stem, &header);

    if (result != RXSTEM_OK) return result;
    hash = rxstem_hash_parts(parts);
    result = rxstem_find(stem, &header, parts, hash, &found);
    if (result != RXSTEM_OK) return result;

    source = stem->attributes[RXSTEM_DEFAULT_ATTRIBUTE];
    if (found != UINT32_MAX) {
        rxstem_entry_load(stem, found, &entry);
        if (entry.generation == header.generation)
            source = stem->attributes[RXSTEM_VALUES_ATTRIBUTE]->attributes[found];
    }
    return rxstem_set_value_string(out, source);
}

RX_INLINE int rxstem_set_parts(value *stem,
                               const rxstem_key_parts *parts,
                               value *new_value) {
    rxstem_header header;
    rxstem_entry entry;
    uint32_t hash;
    uint32_t found;
    size_t key_length;
    int result = rxstem_load(stem, &header);

    if (result != RXSTEM_OK) return result;
    result = rxstem_parts_length(parts, &key_length);
    if (result != RXSTEM_OK || key_length > UINT32_MAX)
        return RXSTEM_OVERFLOW;

    hash = rxstem_hash_parts(parts);
    result = rxstem_find(stem, &header, parts, hash, &found);
    if (result != RXSTEM_OK) return result;

    if (found == UINT32_MAX) {
        value *keys;
        value *values;
        if (header.count == UINT32_MAX) return RXSTEM_OVERFLOW;
        result = rxstem_ensure_capacity(stem, &header, header.count + 1);
        if (result != RXSTEM_OK) return result;

        keys = stem->attributes[RXSTEM_KEYS_ATTRIBUTE];
        values = stem->attributes[RXSTEM_VALUES_ATTRIBUTE];
        if (RXSTEM_TRY_SET_NUM_ATTRIBUTES(
                    keys, (size_t)header.count + 1) != 0)
            return RXSTEM_OUT_OF_MEMORY;
        if (RXSTEM_TRY_SET_NUM_ATTRIBUTES(
                    values, (size_t)header.count + 1) != 0) {
            set_num_attributes(keys, header.count);
            return RXSTEM_OUT_OF_MEMORY;
        }
        result = rxstem_set_parts_string(keys->attributes[header.count], parts);
        if (result != RXSTEM_OK) {
            set_num_attributes(values, header.count);
            set_num_attributes(keys, header.count);
            return result;
        }
        result = rxstem_set_value_string(
                values->attributes[header.count], new_value);
        if (result != RXSTEM_OK) {
            set_num_attributes(values, header.count);
            set_num_attributes(keys, header.count);
            return result;
        }

        entry.hash = hash;
        entry.next = rxstem_bucket_get(stem, hash & 255u);
        entry.generation = header.generation;
        found = header.count;
        rxstem_entry_store(stem, found, &entry);
        rxstem_bucket_set(stem, hash & 255u, found + 1);
        header.count++;
        rxstem_store_header(stem, &header);
        return RXSTEM_OK;
    }

    rxstem_entry_load(stem, found, &entry);
    result = rxstem_set_value_string(
            stem->attributes[RXSTEM_VALUES_ATTRIBUTE]->attributes[found],
            new_value);
    if (result != RXSTEM_OK) return result;
    entry.generation = header.generation;
    rxstem_entry_store(stem, found, &entry);
    return RXSTEM_OK;
}

RX_INLINE int rxstem_reset(value *stem, value *new_default) {
    rxstem_header header;
    int result = rxstem_load(stem, &header);
    if (result != RXSTEM_OK) return result;
    if (header.generation == UINT64_MAX) return RXSTEM_OVERFLOW;

    result = rxstem_set_value_string(
            stem->attributes[RXSTEM_DEFAULT_ATTRIBUTE], new_default);
    if (result != RXSTEM_OK) return result;
    header.generation++;
    rxstem_store_header(stem, &header);
    return RXSTEM_OK;
}

RX_INLINE int rxstem_size(rxinteger *size, value *stem) {
    rxstem_header header;
    int result = rxstem_load(stem, &header);
    if (result != RXSTEM_OK) return result;
    *size = (rxinteger)header.count;
    return RXSTEM_OK;
}

RX_INLINE int rxstem_key_at(value *out, value *stem, rxinteger one_based) {
    rxstem_header header;
    uint32_t index;
    int result = rxstem_load(stem, &header);
    if (result != RXSTEM_OK) return result;
    if (one_based < 1 || (uint64_t)one_based > header.count)
        return RXSTEM_INVALID_INDEX;

    index = (uint32_t)(one_based - 1);
    return rxstem_set_value_string(
            out, stem->attributes[RXSTEM_KEYS_ATTRIBUTE]->attributes[index]);
}

RX_INLINE int rxstem_value_at(value *out, value *stem, rxinteger one_based) {
    rxstem_header header;
    rxstem_entry entry;
    uint32_t index;
    value *source;
    int result = rxstem_load(stem, &header);
    if (result != RXSTEM_OK) return result;
    if (one_based < 1 || (uint64_t)one_based > header.count)
        return RXSTEM_INVALID_INDEX;

    index = (uint32_t)(one_based - 1);
    rxstem_entry_load(stem, index, &entry);
    source = stem->attributes[RXSTEM_DEFAULT_ATTRIBUTE];
    if (entry.generation == header.generation)
        source = stem->attributes[RXSTEM_VALUES_ATTRIBUTE]->attributes[index];
    return rxstem_set_value_string(out, source);
}

#endif
