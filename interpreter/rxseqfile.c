/* Stable identity for an expanded RXBIN module. */

#include "rxseqfile.h"

#include <string.h>

#define RXSEQ_FNV_OFFSET UINT64_C(14695981039346656037)
#define RXSEQ_FNV_PRIME UINT64_C(1099511628211)

const unsigned char rxseq_file_magic[8] = {
        'R', 'X', 'S', 'E', 'Q', 'B', 'I', 'N'
};

int rxseq_write_bytes(FILE *file, const void *data, size_t size) {
    return !size || fwrite(data, 1, size, file) == size;
}

int rxseq_write_u32(FILE *file, uint32_t value) {
    unsigned char bytes[4];
    unsigned int i;
    for (i = 0; i < sizeof(bytes); i++) {
        bytes[i] = (unsigned char)(value & UINT32_C(0xff));
        value >>= 8u;
    }
    return rxseq_write_bytes(file, bytes, sizeof(bytes));
}

int rxseq_write_u64(FILE *file, uint64_t value) {
    unsigned char bytes[8];
    unsigned int i;
    for (i = 0; i < sizeof(bytes); i++) {
        bytes[i] = (unsigned char)(value & UINT64_C(0xff));
        value >>= 8u;
    }
    return rxseq_write_bytes(file, bytes, sizeof(bytes));
}

int rxseq_write_varuint(FILE *file, uint64_t value) {
    unsigned char bytes[10];
    size_t used = 0;
    do {
        unsigned char byte = (unsigned char)(value & UINT64_C(0x7f));
        value >>= 7u;
        if (value) byte |= 0x80u;
        bytes[used++] = byte;
    } while (value);
    return rxseq_write_bytes(file, bytes, used);
}

int rxseq_read_bytes(FILE *file, void *data, size_t size) {
    return !size || fread(data, 1, size, file) == size;
}

int rxseq_read_u32(FILE *file, uint32_t *value) {
    unsigned char bytes[4];
    unsigned int i;
    uint32_t result = 0;
    if (!rxseq_read_bytes(file, bytes, sizeof(bytes))) return 0;
    for (i = 0; i < sizeof(bytes); i++)
        result |= (uint32_t)bytes[i] << (i * 8u);
    *value = result;
    return 1;
}

int rxseq_read_u64(FILE *file, uint64_t *value) {
    unsigned char bytes[8];
    unsigned int i;
    uint64_t result = 0;
    if (!rxseq_read_bytes(file, bytes, sizeof(bytes))) return 0;
    for (i = 0; i < sizeof(bytes); i++)
        result |= (uint64_t)bytes[i] << (i * 8u);
    *value = result;
    return 1;
}

int rxseq_read_varuint(FILE *file, uint64_t *value) {
    uint64_t result = 0;
    unsigned int i;
    for (i = 0; i < 10; i++) {
        unsigned char byte;
        if (!rxseq_read_bytes(file, &byte, 1)) return 0;
        if (i == 9 && (byte & 0xfeu)) return 0;
        result |= (uint64_t)(byte & 0x7fu) << (i * 7u);
        if (!(byte & 0x80u)) {
            if (i && !(byte & 0x7fu)) return 0;
            *value = result;
            return 1;
        }
    }
    return 0;
}

static uint64_t rxseq_hash_bytes(uint64_t hash, const void *data, size_t size) {
    const unsigned char *bytes = (const unsigned char *)data;
    size_t i;
    for (i = 0; i < size; i++) {
        hash ^= bytes[i];
        hash *= RXSEQ_FNV_PRIME;
    }
    return hash;
}

static uint64_t rxseq_hash_size(uint64_t hash, size_t value) {
    unsigned char bytes[8];
    unsigned int i;
    for (i = 0; i < sizeof(bytes); i++) {
        bytes[i] = (unsigned char)(value & 0xffu);
        value >>= 8u;
    }
    return rxseq_hash_bytes(hash, bytes, sizeof(bytes));
}

static uint64_t rxseq_hash_int(uint64_t hash, int value) {
    uint32_t bits = (uint32_t)value;
    unsigned char bytes[4];
    unsigned int i;
    for (i = 0; i < sizeof(bytes); i++) {
        bytes[i] = (unsigned char)(bits & 0xffu);
        bits >>= 8u;
    }
    return rxseq_hash_bytes(hash, bytes, sizeof(bytes));
}

uint64_t rxseq_hash_module_file(const module_file *file) {
    uint64_t hash = RXSEQ_FNV_OFFSET;
    size_t name_size;
    size_t description_size;
    if (!file) return 0;

    name_size = file->name ? strlen(file->name) + 1 : 0;
    description_size = file->description ? strlen(file->description) + 1 : 0;
    hash = rxseq_hash_size(hash, name_size);
    if (name_size) hash = rxseq_hash_bytes(hash, file->name, name_size);
    hash = rxseq_hash_size(hash, description_size);
    if (description_size)
        hash = rxseq_hash_bytes(hash, file->description, description_size);
    hash = rxseq_hash_size(hash, file->header.instruction_size);
    hash = rxseq_hash_size(hash, file->header.constant_size);
    hash = rxseq_hash_int(hash, file->header.globals);
    hash = rxseq_hash_int(hash, file->header.proc_head);
    hash = rxseq_hash_int(hash, file->header.expose_head);
    hash = rxseq_hash_int(hash, file->header.meta_head);
    hash = rxseq_hash_int(hash, file->native ? 1 : 0);
    if (file->header.instruction_size && file->instructions) {
        hash = rxseq_hash_bytes(hash, file->instructions,
                file->header.instruction_size * sizeof(bin_code));
    }
    if (file->header.constant_size && file->constant) {
        hash = rxseq_hash_bytes(hash, file->constant,
                file->header.constant_size);
    }
    return hash;
}
