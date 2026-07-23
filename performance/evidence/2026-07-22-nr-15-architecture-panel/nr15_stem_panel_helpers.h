/*
 * NR-15 architecture-panel helpers.
 *
 * This is deliberately retained with the evidence, not installed as a VM
 * interface.  During the panel rxvmintp.c temporarily includes this file;
 * that include and the provisional opcodes must be reverted after capture.
 */
#ifndef NR15_STEM_PANEL_HELPERS_H
#define NR15_STEM_PANEL_HELPERS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

enum nr15_result {
    NR15_OK = 0,
    NR15_OOM = -1,
    NR15_CORRUPT = -2,
    NR15_OVERFLOW = -3
};

typedef struct nr15_key_parts {
    const char *left;
    size_t left_length;
    const char *right;
    size_t right_length;
    int segmented;
} nr15_key_parts;

static nr15_key_parts nr15_one_part(const value *key) {
    nr15_key_parts parts;
    parts.left = key->string_value;
    parts.left_length = key->string_length;
    parts.right = 0;
    parts.right_length = 0;
    parts.segmented = 0;
    return parts;
}

static nr15_key_parts nr15_two_parts(const value *left, const value *right) {
    nr15_key_parts parts;
    parts.left = left->string_value;
    parts.left_length = left->string_length;
    parts.right = right->string_value;
    parts.right_length = right->string_length;
    parts.segmented = 1;
    return parts;
}

static int nr15_parts_length(const nr15_key_parts *parts, size_t *length) {
    size_t separator = parts->segmented ? 1u : 0u;
    if (parts->left_length > SIZE_MAX - separator) return NR15_OVERFLOW;
    if (parts->right_length > SIZE_MAX - parts->left_length - separator)
        return NR15_OVERFLOW;
    *length = parts->left_length + separator + parts->right_length;
    return NR15_OK;
}

static uint32_t nr15_hash_parts(const nr15_key_parts *parts) {
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

static int nr15_parts_equal(const nr15_key_parts *parts,
                            const char *stored, size_t stored_length) {
    size_t length;
    if (nr15_parts_length(parts, &length) != NR15_OK || length != stored_length)
        return 0;
    if (parts->left_length &&
        memcmp(parts->left, stored, parts->left_length) != 0) return 0;
    if (!parts->segmented) return 1;
    if (stored[parts->left_length] != '.') return 0;
    return !parts->right_length ||
           memcmp(parts->right, stored + parts->left_length + 1,
                  parts->right_length) == 0;
}

static int nr15_set_parts_string(value *dest, const nr15_key_parts *parts) {
    size_t length;
    if (nr15_parts_length(parts, &length) != NR15_OK) return NR15_OVERFLOW;
    if (dest->reference_payload) rxvm_reference_value_release_payload(dest);
    if (dest->native_payload_ops) clear_binary_payload(dest);
    prep_string_buffer(dest, length);
    if (parts->left_length)
        memcpy(dest->string_value, parts->left, parts->left_length);
    if (parts->segmented) {
        dest->string_value[parts->left_length] = '.';
        if (parts->right_length)
            memcpy(dest->string_value + parts->left_length + 1,
                   parts->right, parts->right_length);
    }
    dest->string_pos = 0;
#ifndef NUTF8
    dest->string_char_pos = 0;
    refresh_utf8_flags(dest);
#else
    clear_vm_private_flags(dest);
#endif
    return NR15_OK;
}

/* -------------------------------------------------------------------------
 * D1: native operations over the current stem's nine attribute slots.
 * ------------------------------------------------------------------------- */

enum {
    NR15_D1_BUCKETS = 0,
    NR15_D1_KEYS = 4,
    NR15_D1_VALUES = 7,
    NR15_D1_VALUE_GENERATIONS = 8,
    NR15_D1_NEXT = 5,
    NR15_D1_COUNT = 1,
    NR15_D1_DEFAULT = 3,
    NR15_D1_GENERATION = 2,
    NR15_D1_ATTRIBUTE_COUNT = 9
};

static int nr15_d1_shape(value *stem) {
    return stem && stem->num_attributes >= NR15_D1_ATTRIBUTE_COUNT;
}

static int nr15_d1_find(value *stem, const nr15_key_parts *parts,
                        uint32_t hash, size_t *found, size_t *last) {
    value *buckets;
    value *keys;
    value *next;
    rxinteger raw;
    size_t count;
    if (!nr15_d1_shape(stem)) return NR15_CORRUPT;
    buckets = stem->attributes[NR15_D1_BUCKETS];
    if (buckets->num_attributes < 256) set_num_attributes(buckets, 256);
    keys = stem->attributes[NR15_D1_KEYS];
    next = stem->attributes[NR15_D1_NEXT];
    count = (size_t)stem->attributes[NR15_D1_COUNT]->int_value;
    raw = buckets->attributes[hash & 255u]->int_value;
    *found = SIZE_MAX;
    *last = SIZE_MAX;
    while (raw > 0) {
        size_t index = (size_t)(raw - 1);
        if (index >= count || index >= keys->num_attributes ||
            index >= next->num_attributes) return NR15_CORRUPT;
        if (nr15_parts_equal(parts, keys->attributes[index]->string_value,
                             keys->attributes[index]->string_length)) {
            *found = index;
            return NR15_OK;
        }
        *last = index;
        raw = next->attributes[index]->int_value;
    }
    return NR15_OK;
}

static int nr15_d1_get_parts(value *out, value *stem,
                             const nr15_key_parts *parts) {
    size_t found, last;
    int rc = nr15_d1_find(stem, parts, nr15_hash_parts(parts), &found, &last);
    value *source;
    if (rc != NR15_OK) return rc;
    source = stem->attributes[NR15_D1_DEFAULT];
    if (found != SIZE_MAX) {
        value *generations = stem->attributes[NR15_D1_VALUE_GENERATIONS];
        value *values = stem->attributes[NR15_D1_VALUES];
        if (found >= generations->num_attributes || found >= values->num_attributes)
            return NR15_CORRUPT;
        if (generations->attributes[found]->int_value ==
            stem->attributes[NR15_D1_GENERATION]->int_value)
            source = values->attributes[found];
    }
    set_value_string(out, source);
    return NR15_OK;
}

static int nr15_d1_set_parts(value *stem, const nr15_key_parts *parts,
                             value *new_value) {
    uint32_t hash = nr15_hash_parts(parts);
    size_t found, last, count;
    value *keys, *values, *generations, *next, *buckets;
    int rc = nr15_d1_find(stem, parts, hash, &found, &last);
    if (rc != NR15_OK) return rc;
    keys = stem->attributes[NR15_D1_KEYS];
    values = stem->attributes[NR15_D1_VALUES];
    generations = stem->attributes[NR15_D1_VALUE_GENERATIONS];
    next = stem->attributes[NR15_D1_NEXT];
    buckets = stem->attributes[NR15_D1_BUCKETS];
    if (found != SIZE_MAX) {
        set_value_string(values->attributes[found], new_value);
        set_int(generations->attributes[found],
                stem->attributes[NR15_D1_GENERATION]->int_value);
        return NR15_OK;
    }
    count = (size_t)stem->attributes[NR15_D1_COUNT]->int_value;
    if (count == SIZE_MAX) return NR15_OVERFLOW;
    set_num_attributes(keys, count + 1);
    set_num_attributes(values, count + 1);
    set_num_attributes(generations, count + 1);
    set_num_attributes(next, count + 1);
    rc = nr15_set_parts_string(keys->attributes[count], parts);
    if (rc != NR15_OK) return rc;
    set_value_string(values->attributes[count], new_value);
    set_int(generations->attributes[count],
            stem->attributes[NR15_D1_GENERATION]->int_value);
    set_int(next->attributes[count], 0);
    if (last == SIZE_MAX)
        set_int(buckets->attributes[hash & 255u], (rxinteger)count + 1);
    else
        set_int(next->attributes[last], (rxinteger)count + 1);
    set_int(stem->attributes[NR15_D1_COUNT], (rxinteger)count + 1);
    return NR15_OK;
}

static int nr15_d1_reset(value *stem, value *new_default) {
    if (!nr15_d1_shape(stem)) return NR15_CORRUPT;
    set_value_string(stem->attributes[NR15_D1_DEFAULT], new_default);
    stem->attributes[NR15_D1_GENERATION]->int_value++;
    return NR15_OK;
}

/* -------------------------------------------------------------------------
 * Fixed-width helpers for the two binary-backed candidates.
 * The layout is an ephemeral VM representation, not an RXBIN serialization.
 * Explicit little-endian access nevertheless makes the evidence portable.
 * ------------------------------------------------------------------------- */

enum {
    NR15_HEADER_SIZE = 64,
    NR15_BUCKET_COUNT = 256,
    NR15_BUCKET_BYTES = NR15_BUCKET_COUNT * 4,
    NR15_D2_ENTRY_SIZE = 36,
    NR15_D2H_ENTRY_SIZE = 16,
    NR15_INITIAL_ENTRY_CAPACITY = 16,
    NR15_INITIAL_ARENA_CAPACITY = 1024
};

#define NR15_D2_MAGIC UINT32_C(0x3244314e)  /* N1D2 */
#define NR15_D2H_MAGIC UINT32_C(0x4832314e) /* N12H */

typedef struct nr15_header {
    uint32_t magic;
    uint32_t version;
    uint64_t generation;
    uint32_t count;
    uint32_t capacity;
    uint32_t arena_used;
    uint32_t arena_capacity;
    uint32_t default_offset;
    uint32_t default_length;
    uint32_t default_capacity;
} nr15_header;

typedef struct nr15_d2_entry {
    uint32_t hash;
    uint32_t next;
    uint64_t generation;
    uint32_t key_offset;
    uint32_t key_length;
    uint32_t value_offset;
    uint32_t value_length;
    uint32_t value_capacity;
} nr15_d2_entry;

typedef struct nr15_d2h_entry {
    uint32_t hash;
    uint32_t next;
    uint64_t generation;
} nr15_d2h_entry;

static uint32_t nr15_load_u32(const unsigned char *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint64_t nr15_load_u64(const unsigned char *p) {
    return (uint64_t)nr15_load_u32(p) |
           ((uint64_t)nr15_load_u32(p + 4) << 32);
}

static void nr15_store_u32(unsigned char *p, uint32_t v) {
    p[0] = (unsigned char)v;
    p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16);
    p[3] = (unsigned char)(v >> 24);
}

static void nr15_store_u64(unsigned char *p, uint64_t v) {
    nr15_store_u32(p, (uint32_t)v);
    nr15_store_u32(p + 4, (uint32_t)(v >> 32));
}

static void nr15_store_header(value *stem, const nr15_header *h) {
    unsigned char *p = (unsigned char *)stem->binary_value;
    nr15_store_u32(p + 0, h->magic);
    nr15_store_u32(p + 4, h->version);
    nr15_store_u64(p + 8, h->generation);
    nr15_store_u32(p + 16, h->count);
    nr15_store_u32(p + 20, h->capacity);
    nr15_store_u32(p + 24, h->arena_used);
    nr15_store_u32(p + 28, h->arena_capacity);
    nr15_store_u32(p + 32, h->default_offset);
    nr15_store_u32(p + 36, h->default_length);
    nr15_store_u32(p + 40, h->default_capacity);
}

static void nr15_load_header(value *stem, nr15_header *h) {
    const unsigned char *p = (const unsigned char *)stem->binary_value;
    h->magic = nr15_load_u32(p + 0);
    h->version = nr15_load_u32(p + 4);
    h->generation = nr15_load_u64(p + 8);
    h->count = nr15_load_u32(p + 16);
    h->capacity = nr15_load_u32(p + 20);
    h->arena_used = nr15_load_u32(p + 24);
    h->arena_capacity = nr15_load_u32(p + 28);
    h->default_offset = nr15_load_u32(p + 32);
    h->default_length = nr15_load_u32(p + 36);
    h->default_capacity = nr15_load_u32(p + 40);
}

static size_t nr15_entries_base(void) {
    return NR15_HEADER_SIZE + NR15_BUCKET_BYTES;
}

static int nr15_layout_size(uint32_t capacity, size_t entry_size,
                            uint32_t arena_capacity, size_t *total) {
    size_t base = nr15_entries_base();
    size_t entries;
    if ((size_t)capacity > (SIZE_MAX - base) / entry_size) return NR15_OVERFLOW;
    entries = (size_t)capacity * entry_size;
    if ((size_t)arena_capacity > SIZE_MAX - base - entries) return NR15_OVERFLOW;
    *total = base + entries + arena_capacity;
    return NR15_OK;
}

static size_t nr15_arena_base(uint32_t capacity) {
    return nr15_entries_base() + (size_t)capacity * NR15_D2_ENTRY_SIZE;
}

static uint32_t nr15_bucket_get(value *stem, uint32_t bucket) {
    return nr15_load_u32((unsigned char *)stem->binary_value +
                         NR15_HEADER_SIZE + (size_t)bucket * 4);
}

static void nr15_bucket_set(value *stem, uint32_t bucket, uint32_t value_) {
    nr15_store_u32((unsigned char *)stem->binary_value +
                   NR15_HEADER_SIZE + (size_t)bucket * 4, value_);
}

static void nr15_d2_entry_load(value *stem, uint32_t index,
                               nr15_d2_entry *entry) {
    const unsigned char *p = (unsigned char *)stem->binary_value +
                             nr15_entries_base() +
                             (size_t)index * NR15_D2_ENTRY_SIZE;
    entry->hash = nr15_load_u32(p + 0);
    entry->next = nr15_load_u32(p + 4);
    entry->generation = nr15_load_u64(p + 8);
    entry->key_offset = nr15_load_u32(p + 16);
    entry->key_length = nr15_load_u32(p + 20);
    entry->value_offset = nr15_load_u32(p + 24);
    entry->value_length = nr15_load_u32(p + 28);
    entry->value_capacity = nr15_load_u32(p + 32);
}

static void nr15_d2_entry_store(value *stem, uint32_t index,
                                const nr15_d2_entry *entry) {
    unsigned char *p = (unsigned char *)stem->binary_value +
                       nr15_entries_base() +
                       (size_t)index * NR15_D2_ENTRY_SIZE;
    nr15_store_u32(p + 0, entry->hash);
    nr15_store_u32(p + 4, entry->next);
    nr15_store_u64(p + 8, entry->generation);
    nr15_store_u32(p + 16, entry->key_offset);
    nr15_store_u32(p + 20, entry->key_length);
    nr15_store_u32(p + 24, entry->value_offset);
    nr15_store_u32(p + 28, entry->value_length);
    nr15_store_u32(p + 32, entry->value_capacity);
}

static int nr15_d2_init(value *stem, nr15_header *h) {
    size_t total;
    if (nr15_layout_size(NR15_INITIAL_ENTRY_CAPACITY, NR15_D2_ENTRY_SIZE,
                         NR15_INITIAL_ARENA_CAPACITY, &total) != NR15_OK)
        return NR15_OVERFLOW;
    if (prep_binary_buffer(stem, total) != 0) return NR15_OOM;
    memset(stem->binary_value, 0, total);
    h->magic = NR15_D2_MAGIC;
    h->version = 1;
    h->generation = 0;
    h->count = 0;
    h->capacity = NR15_INITIAL_ENTRY_CAPACITY;
    h->arena_used = 0;
    h->arena_capacity = NR15_INITIAL_ARENA_CAPACITY;
    h->default_offset = 0;
    h->default_length = 0;
    h->default_capacity = 0;
    nr15_store_header(stem, h);
    return NR15_OK;
}

static int nr15_d2_load(value *stem, nr15_header *h) {
    size_t total;
    if (!stem->binary_length) return nr15_d2_init(stem, h);
    if (stem->binary_length < NR15_HEADER_SIZE) return NR15_CORRUPT;
    nr15_load_header(stem, h);
    if (h->magic != NR15_D2_MAGIC || h->version != 1 ||
        h->count > h->capacity || h->arena_used > h->arena_capacity)
        return NR15_CORRUPT;
    if (nr15_layout_size(h->capacity, NR15_D2_ENTRY_SIZE,
                         h->arena_capacity, &total) != NR15_OK ||
        total > stem->binary_length) return NR15_CORRUPT;
    if (h->default_offset > h->arena_used ||
        h->default_capacity > h->arena_used - h->default_offset ||
        h->default_length > h->default_capacity)
        return NR15_CORRUPT;
    return NR15_OK;
}

static int nr15_d2_ensure(value *stem, nr15_header *h,
                          uint32_t entries_needed, uint32_t arena_needed) {
    uint32_t old_capacity = h->capacity;
    uint32_t new_capacity = h->capacity;
    uint32_t new_arena = h->arena_capacity;
    size_t old_arena_base, new_arena_base, total;
    while (new_capacity < entries_needed) {
        if (new_capacity > UINT32_MAX / 2) return NR15_OVERFLOW;
        new_capacity *= 2;
    }
    while (new_arena < arena_needed) {
        if (new_arena > UINT32_MAX / 2) return NR15_OVERFLOW;
        new_arena *= 2;
    }
    if (nr15_layout_size(new_capacity, NR15_D2_ENTRY_SIZE,
                         new_arena, &total) != NR15_OK) return NR15_OVERFLOW;
    old_arena_base = nr15_arena_base(old_capacity);
    new_arena_base = nr15_arena_base(new_capacity);
    if (reserve_binary_buffer(stem, total) != 0) return NR15_OOM;
    if (new_arena_base != old_arena_base) {
        memmove(stem->binary_value + new_arena_base,
                stem->binary_value + old_arena_base, h->arena_used);
        memset(stem->binary_value + nr15_entries_base() +
                   (size_t)old_capacity * NR15_D2_ENTRY_SIZE,
               0, (size_t)(new_capacity - old_capacity) * NR15_D2_ENTRY_SIZE);
    }
    h->capacity = new_capacity;
    h->arena_capacity = new_arena;
    stem->binary_length = total;
    nr15_store_header(stem, h);
    return NR15_OK;
}

static int nr15_d2_append(value *stem, nr15_header *h,
                          const char *bytes, size_t length, uint32_t *offset) {
    size_t base;
    if (length > UINT32_MAX || h->arena_used > UINT32_MAX - (uint32_t)length)
        return NR15_OVERFLOW;
    *offset = h->arena_used;
    base = nr15_arena_base(h->capacity) + h->arena_used;
    if (length) memcpy(stem->binary_value + base, bytes, length);
    h->arena_used += (uint32_t)length;
    return NR15_OK;
}

static int nr15_d2_storage_capacity(size_t length, uint32_t *capacity) {
    uint32_t result = 32;
    if (length > UINT32_MAX) return NR15_OVERFLOW;
    while (result < length) {
        if (result > UINT32_MAX / 2) return NR15_OVERFLOW;
        result *= 2;
    }
    *capacity = result;
    return NR15_OK;
}

static int nr15_d2_append_reserved(value *stem, nr15_header *h,
                                   const char *bytes, size_t length,
                                   uint32_t capacity, uint32_t *offset) {
    size_t base;
    if (length > capacity || h->arena_used > UINT32_MAX - capacity)
        return NR15_OVERFLOW;
    *offset = h->arena_used;
    base = nr15_arena_base(h->capacity) + h->arena_used;
    if (length) memcpy(stem->binary_value + base, bytes, length);
    h->arena_used += capacity;
    return NR15_OK;
}

static int nr15_d2_append_parts(value *stem, nr15_header *h,
                                const nr15_key_parts *parts, uint32_t *offset,
                                uint32_t *length_out) {
    size_t length;
    unsigned char *dest;
    int rc = nr15_parts_length(parts, &length);
    if (rc != NR15_OK || length > UINT32_MAX) return NR15_OVERFLOW;
    *offset = h->arena_used;
    *length_out = (uint32_t)length;
    dest = (unsigned char *)stem->binary_value + nr15_arena_base(h->capacity) +
           h->arena_used;
    if (parts->left_length) memcpy(dest, parts->left, parts->left_length);
    if (parts->segmented) {
        dest[parts->left_length] = '.';
        if (parts->right_length)
            memcpy(dest + parts->left_length + 1, parts->right,
                   parts->right_length);
    }
    h->arena_used += (uint32_t)length;
    return NR15_OK;
}

static int nr15_d2_find(value *stem, const nr15_header *h,
                        const nr15_key_parts *parts, uint32_t hash,
                        uint32_t *found) {
    uint32_t raw = nr15_bucket_get(stem, hash & 255u);
    size_t arena = nr15_arena_base(h->capacity);
    *found = UINT32_MAX;
    while (raw) {
        uint32_t index = raw - 1;
        nr15_d2_entry entry;
        if (index >= h->count) return NR15_CORRUPT;
        nr15_d2_entry_load(stem, index, &entry);
        if (entry.key_offset > h->arena_used ||
            entry.key_length > h->arena_used - entry.key_offset)
            return NR15_CORRUPT;
        if (entry.value_offset > h->arena_used ||
            entry.value_capacity > h->arena_used - entry.value_offset ||
            entry.value_length > entry.value_capacity) return NR15_CORRUPT;
        if (entry.hash == hash &&
            nr15_parts_equal(parts, stem->binary_value + arena + entry.key_offset,
                             entry.key_length)) {
            *found = index;
            return NR15_OK;
        }
        raw = entry.next;
    }
    return NR15_OK;
}

static int nr15_d2_get_parts(value *out, value *stem,
                             const nr15_key_parts *parts) {
    nr15_header h;
    nr15_d2_entry entry;
    uint32_t found, offset, length;
    size_t arena;
    int rc = nr15_d2_load(stem, &h);
    if (rc != NR15_OK) return rc;
    rc = nr15_d2_find(stem, &h, parts, nr15_hash_parts(parts), &found);
    if (rc != NR15_OK) return rc;
    offset = h.default_offset;
    length = h.default_length;
    if (found != UINT32_MAX) {
        nr15_d2_entry_load(stem, found, &entry);
        if (entry.generation == h.generation) {
            offset = entry.value_offset;
            length = entry.value_length;
        }
    }
    if (offset > h.arena_used || length > h.arena_used - offset)
        return NR15_CORRUPT;
    arena = nr15_arena_base(h.capacity);
    set_string(out, stem->binary_value + arena + offset, length);
    return NR15_OK;
}

static int nr15_d2_set_parts(value *stem, const nr15_key_parts *parts,
                             value *new_value) {
    nr15_header h;
    nr15_d2_entry entry;
    uint32_t found, key_length, needed_arena, value_capacity;
    size_t parts_length;
    int rc = nr15_d2_load(stem, &h);
    if (rc != NR15_OK) return rc;
    rc = nr15_d2_find(stem, &h, parts, nr15_hash_parts(parts), &found);
    if (rc != NR15_OK) return rc;
    if (nr15_parts_length(parts, &parts_length) != NR15_OK ||
        parts_length > UINT32_MAX || new_value->string_length > UINT32_MAX)
        return NR15_OVERFLOW;
    if (found != UINT32_MAX) nr15_d2_entry_load(stem, found, &entry);
    rc = nr15_d2_storage_capacity(new_value->string_length, &value_capacity);
    if (rc != NR15_OK) return rc;
    needed_arena = h.arena_used;
    if (found == UINT32_MAX) {
        if (needed_arena > UINT32_MAX - (uint32_t)parts_length)
            return NR15_OVERFLOW;
        needed_arena += (uint32_t)parts_length;
        if (needed_arena > UINT32_MAX - value_capacity) return NR15_OVERFLOW;
        needed_arena += value_capacity;
    }
    else if (new_value->string_length > entry.value_capacity) {
        if (needed_arena > UINT32_MAX - value_capacity) return NR15_OVERFLOW;
        needed_arena += value_capacity;
    }
    rc = nr15_d2_ensure(stem, &h,
                        h.count + (found == UINT32_MAX), needed_arena);
    if (rc != NR15_OK) return rc;
    if (found == UINT32_MAX) {
        memset(&entry, 0, sizeof(entry));
        entry.hash = nr15_hash_parts(parts);
        entry.next = nr15_bucket_get(stem, entry.hash & 255u);
        rc = nr15_d2_append_parts(stem, &h, parts, &entry.key_offset,
                                  &key_length);
        if (rc != NR15_OK) return rc;
        entry.key_length = key_length;
        found = h.count++;
        nr15_bucket_set(stem, entry.hash & 255u, found + 1);
    }
    if (new_value->string_length <= entry.value_capacity) {
        if (new_value->string_length)
            memcpy(stem->binary_value + nr15_arena_base(h.capacity) +
                       entry.value_offset,
                   new_value->string_value, new_value->string_length);
    }
    else {
        rc = nr15_d2_append_reserved(stem, &h, new_value->string_value,
                                     new_value->string_length, value_capacity,
                                     &entry.value_offset);
        if (rc != NR15_OK) return rc;
        entry.value_capacity = value_capacity;
    }
    entry.value_length = (uint32_t)new_value->string_length;
    entry.generation = h.generation;
    nr15_d2_entry_store(stem, found, &entry);
    nr15_store_header(stem, &h);
    return NR15_OK;
}

static int nr15_d2_reset(value *stem, value *new_default) {
    nr15_header h;
    uint32_t needed, capacity;
    int rc = nr15_d2_load(stem, &h);
    if (rc != NR15_OK) return rc;
    rc = nr15_d2_storage_capacity(new_default->string_length, &capacity);
    if (rc != NR15_OK) return rc;
    needed = h.arena_used;
    if (new_default->string_length > h.default_capacity) {
        if (needed > UINT32_MAX - capacity) return NR15_OVERFLOW;
        needed += capacity;
    }
    rc = nr15_d2_ensure(stem, &h, h.count, needed);
    if (rc != NR15_OK) return rc;
    if (new_default->string_length <= h.default_capacity) {
        if (new_default->string_length)
            memcpy(stem->binary_value + nr15_arena_base(h.capacity) +
                       h.default_offset,
                   new_default->string_value, new_default->string_length);
    }
    else {
        rc = nr15_d2_append_reserved(stem, &h, new_default->string_value,
                                     new_default->string_length, capacity,
                                     &h.default_offset);
        if (rc != NR15_OK) return rc;
        h.default_capacity = capacity;
    }
    h.default_length = (uint32_t)new_default->string_length;
    h.generation++;
    nr15_store_header(stem, &h);
    return NR15_OK;
}

/* -------------------------------------------------------------------------
 * D2-hybrid: binary header/buckets/entries, VM strings in attributes.
 * attr 0 = keys array, attr 1 = values array, attr 2 = default string.
 * ------------------------------------------------------------------------- */

static void nr15_d2h_entry_load(value *stem, uint32_t index,
                                nr15_d2h_entry *entry) {
    const unsigned char *p = (unsigned char *)stem->binary_value +
                             nr15_entries_base() +
                             (size_t)index * NR15_D2H_ENTRY_SIZE;
    entry->hash = nr15_load_u32(p + 0);
    entry->next = nr15_load_u32(p + 4);
    entry->generation = nr15_load_u64(p + 8);
}

static void nr15_d2h_entry_store(value *stem, uint32_t index,
                                 const nr15_d2h_entry *entry) {
    unsigned char *p = (unsigned char *)stem->binary_value +
                       nr15_entries_base() +
                       (size_t)index * NR15_D2H_ENTRY_SIZE;
    nr15_store_u32(p + 0, entry->hash);
    nr15_store_u32(p + 4, entry->next);
    nr15_store_u64(p + 8, entry->generation);
}

static int nr15_d2h_init(value *stem, nr15_header *h) {
    size_t total;
    if (nr15_layout_size(NR15_INITIAL_ENTRY_CAPACITY, NR15_D2H_ENTRY_SIZE,
                         0, &total) != NR15_OK) return NR15_OVERFLOW;
    if (prep_binary_buffer(stem, total) != 0) return NR15_OOM;
    memset(stem->binary_value, 0, total);
    set_num_attributes(stem, 3);
    h->magic = NR15_D2H_MAGIC;
    h->version = 1;
    h->generation = 0;
    h->count = 0;
    h->capacity = NR15_INITIAL_ENTRY_CAPACITY;
    h->arena_used = 0;
    h->arena_capacity = 0;
    h->default_offset = 0;
    h->default_length = 0;
    h->default_capacity = 0;
    nr15_store_header(stem, h);
    return NR15_OK;
}

static int nr15_d2h_load(value *stem, nr15_header *h) {
    size_t total;
    if (!stem->binary_length) return nr15_d2h_init(stem, h);
    if (stem->binary_length < NR15_HEADER_SIZE || stem->num_attributes < 3)
        return NR15_CORRUPT;
    nr15_load_header(stem, h);
    if (h->magic != NR15_D2H_MAGIC || h->version != 1 ||
        h->count > h->capacity) return NR15_CORRUPT;
    if (nr15_layout_size(h->capacity, NR15_D2H_ENTRY_SIZE, 0, &total) != NR15_OK ||
        total > stem->binary_length) return NR15_CORRUPT;
    if (stem->attributes[0]->num_attributes < h->count ||
        stem->attributes[1]->num_attributes < h->count) return NR15_CORRUPT;
    return NR15_OK;
}

static int nr15_d2h_ensure(value *stem, nr15_header *h, uint32_t needed) {
    uint32_t old_capacity = h->capacity;
    uint32_t new_capacity = h->capacity;
    size_t total;
    while (new_capacity < needed) {
        if (new_capacity > UINT32_MAX / 2) return NR15_OVERFLOW;
        new_capacity *= 2;
    }
    if (nr15_layout_size(new_capacity, NR15_D2H_ENTRY_SIZE, 0, &total) != NR15_OK)
        return NR15_OVERFLOW;
    if (reserve_binary_buffer(stem, total) != 0) return NR15_OOM;
    if (new_capacity != old_capacity)
        memset(stem->binary_value + nr15_entries_base() +
                   (size_t)old_capacity * NR15_D2H_ENTRY_SIZE,
               0, (size_t)(new_capacity - old_capacity) * NR15_D2H_ENTRY_SIZE);
    stem->binary_length = total;
    h->capacity = new_capacity;
    nr15_store_header(stem, h);
    return NR15_OK;
}

static int nr15_d2h_find(value *stem, const nr15_header *h,
                         const nr15_key_parts *parts, uint32_t hash,
                         uint32_t *found) {
    uint32_t raw = nr15_bucket_get(stem, hash & 255u);
    value *keys = stem->attributes[0];
    *found = UINT32_MAX;
    while (raw) {
        uint32_t index = raw - 1;
        nr15_d2h_entry entry;
        value *key;
        if (index >= h->count || index >= keys->num_attributes)
            return NR15_CORRUPT;
        nr15_d2h_entry_load(stem, index, &entry);
        key = keys->attributes[index];
        if (entry.hash == hash &&
            nr15_parts_equal(parts, key->string_value, key->string_length)) {
            *found = index;
            return NR15_OK;
        }
        raw = entry.next;
    }
    return NR15_OK;
}

static int nr15_d2h_get_parts(value *out, value *stem,
                              const nr15_key_parts *parts) {
    nr15_header h;
    nr15_d2h_entry entry;
    uint32_t found;
    value *source;
    int rc = nr15_d2h_load(stem, &h);
    if (rc != NR15_OK) return rc;
    rc = nr15_d2h_find(stem, &h, parts, nr15_hash_parts(parts), &found);
    if (rc != NR15_OK) return rc;
    source = stem->attributes[2];
    if (found != UINT32_MAX) {
        nr15_d2h_entry_load(stem, found, &entry);
        if (entry.generation == h.generation)
            source = stem->attributes[1]->attributes[found];
    }
    set_value_string(out, source);
    return NR15_OK;
}

static int nr15_d2h_set_parts(value *stem, const nr15_key_parts *parts,
                              value *new_value) {
    nr15_header h;
    nr15_d2h_entry entry;
    uint32_t found;
    int rc = nr15_d2h_load(stem, &h);
    if (rc != NR15_OK) return rc;
    rc = nr15_d2h_find(stem, &h, parts, nr15_hash_parts(parts), &found);
    if (rc != NR15_OK) return rc;
    if (found == UINT32_MAX) {
        value *keys = stem->attributes[0];
        value *values = stem->attributes[1];
        rc = nr15_d2h_ensure(stem, &h, h.count + 1);
        if (rc != NR15_OK) return rc;
        set_num_attributes(keys, h.count + 1);
        set_num_attributes(values, h.count + 1);
        rc = nr15_set_parts_string(keys->attributes[h.count], parts);
        if (rc != NR15_OK) return rc;
        memset(&entry, 0, sizeof(entry));
        entry.hash = nr15_hash_parts(parts);
        entry.next = nr15_bucket_get(stem, entry.hash & 255u);
        found = h.count++;
        nr15_bucket_set(stem, entry.hash & 255u, found + 1);
    }
    else {
        nr15_d2h_entry_load(stem, found, &entry);
    }
    set_value_string(stem->attributes[1]->attributes[found], new_value);
    entry.generation = h.generation;
    nr15_d2h_entry_store(stem, found, &entry);
    nr15_store_header(stem, &h);
    return NR15_OK;
}

static int nr15_d2h_reset(value *stem, value *new_default) {
    nr15_header h;
    int rc = nr15_d2h_load(stem, &h);
    if (rc != NR15_OK) return rc;
    set_value_string(stem->attributes[2], new_default);
    h.generation++;
    nr15_store_header(stem, &h);
    return NR15_OK;
}

static int nr15_d1_key_at(value *out, value *stem, rxinteger one_based) {
    size_t index, count;
    value *keys;
    if (!nr15_d1_shape(stem) || one_based < 1) return NR15_CORRUPT;
    index = (size_t)(one_based - 1);
    count = (size_t)stem->attributes[NR15_D1_COUNT]->int_value;
    keys = stem->attributes[NR15_D1_KEYS];
    if (index >= count || index >= keys->num_attributes) return NR15_CORRUPT;
    set_value_string(out, keys->attributes[index]);
    return NR15_OK;
}

static int nr15_d1_value_at(value *out, value *stem, rxinteger one_based) {
    size_t index, count;
    value *source;
    if (!nr15_d1_shape(stem) || one_based < 1) return NR15_CORRUPT;
    index = (size_t)(one_based - 1);
    count = (size_t)stem->attributes[NR15_D1_COUNT]->int_value;
    if (index >= count ||
        index >= stem->attributes[NR15_D1_VALUES]->num_attributes ||
        index >= stem->attributes[NR15_D1_VALUE_GENERATIONS]->num_attributes)
        return NR15_CORRUPT;
    source = stem->attributes[NR15_D1_DEFAULT];
    if (stem->attributes[NR15_D1_VALUE_GENERATIONS]->attributes[index]->int_value ==
        stem->attributes[NR15_D1_GENERATION]->int_value)
        source = stem->attributes[NR15_D1_VALUES]->attributes[index];
    set_value_string(out, source);
    return NR15_OK;
}

static int nr15_d2_key_at(value *out, value *stem, rxinteger one_based) {
    nr15_header h;
    nr15_d2_entry entry;
    uint32_t index;
    int rc = nr15_d2_load(stem, &h);
    if (rc != NR15_OK || one_based < 1 || (uint64_t)one_based > h.count)
        return NR15_CORRUPT;
    index = (uint32_t)(one_based - 1);
    nr15_d2_entry_load(stem, index, &entry);
    if (entry.key_offset > h.arena_used ||
        entry.key_length > h.arena_used - entry.key_offset) return NR15_CORRUPT;
    set_string(out, stem->binary_value + nr15_arena_base(h.capacity) +
                    entry.key_offset, entry.key_length);
    return NR15_OK;
}

static int nr15_d2_value_at(value *out, value *stem, rxinteger one_based) {
    nr15_header h;
    nr15_d2_entry entry;
    uint32_t offset, length, index;
    int rc = nr15_d2_load(stem, &h);
    if (rc != NR15_OK || one_based < 1 || (uint64_t)one_based > h.count)
        return NR15_CORRUPT;
    index = (uint32_t)(one_based - 1);
    nr15_d2_entry_load(stem, index, &entry);
    offset = h.default_offset;
    length = h.default_length;
    if (entry.generation == h.generation) {
        offset = entry.value_offset;
        length = entry.value_length;
    }
    if (offset > h.arena_used || length > h.arena_used - offset)
        return NR15_CORRUPT;
    set_string(out, stem->binary_value + nr15_arena_base(h.capacity) + offset,
               length);
    return NR15_OK;
}

static int nr15_d2h_key_at(value *out, value *stem, rxinteger one_based) {
    nr15_header h;
    uint32_t index;
    int rc = nr15_d2h_load(stem, &h);
    if (rc != NR15_OK || one_based < 1 || (uint64_t)one_based > h.count)
        return NR15_CORRUPT;
    index = (uint32_t)(one_based - 1);
    set_value_string(out, stem->attributes[0]->attributes[index]);
    return NR15_OK;
}

static int nr15_d2h_value_at(value *out, value *stem, rxinteger one_based) {
    nr15_header h;
    nr15_d2h_entry entry;
    uint32_t index;
    value *source;
    int rc = nr15_d2h_load(stem, &h);
    if (rc != NR15_OK || one_based < 1 || (uint64_t)one_based > h.count)
        return NR15_CORRUPT;
    index = (uint32_t)(one_based - 1);
    nr15_d2h_entry_load(stem, index, &entry);
    source = stem->attributes[2];
    if (entry.generation == h.generation)
        source = stem->attributes[1]->attributes[index];
    set_value_string(out, source);
    return NR15_OK;
}

static const char *nr15_error_message(int result) {
    switch (result) {
        case NR15_OOM: return "NR-15 panel native stem allocation failed";
        case NR15_OVERFLOW: return "NR-15 panel native stem size overflow";
        default: return "NR-15 panel native stem representation is corrupt";
    }
}

#endif
