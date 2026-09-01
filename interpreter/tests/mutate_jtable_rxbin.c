/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rxbin.h"
#include "rxjtable.h"

static uint16_t read_u16le(const unsigned char *bytes) {
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8u));
}

static uint32_t read_u32le(const unsigned char *bytes) {
    return (uint32_t)bytes[0] |
           ((uint32_t)bytes[1] << 8u) |
           ((uint32_t)bytes[2] << 16u) |
           ((uint32_t)bytes[3] << 24u);
}

static void write_u32le(unsigned char *bytes, uint32_t value) {
    bytes[0] = (unsigned char)(value & UINT32_C(0xff));
    bytes[1] = (unsigned char)((value >> 8u) & UINT32_C(0xff));
    bytes[2] = (unsigned char)((value >> 16u) & UINT32_C(0xff));
    bytes[3] = (unsigned char)((value >> 24u) & UINT32_C(0xff));
}

static unsigned char *find_jump_table(module_file *module, size_t *length_out) {
    size_t offset = 0;

    while (offset < module->header.constant_size) {
        chameleon_constant *entry =
                (chameleon_constant *)((unsigned char *)module->constant + offset);
        if (entry->size_in_pool == 0 ||
            entry->size_in_pool > module->header.constant_size - offset) return NULL;
        if (entry->type == BINARY_CONST) {
            string_constant *binary = (string_constant *)entry;
            unsigned char *payload = (unsigned char *)binary->string;
            if (binary->string_len >= RX_JTABLE_HEADER_SIZE &&
                payload[0] >= RX_JTABLE_ALG_LINEAR &&
                payload[0] <= RX_JTABLE_ALG_ACPH) {
                *length_out = binary->string_len;
                return payload;
            }
        }
        offset += entry->size_in_pool;
    }
    return NULL;
}

static int key_is_beta(const unsigned char *payload, size_t length,
                       uint32_t key_offset, uint32_t key_length) {
    static const unsigned char beta[] = {'b', 'e', 't', 'a'};
    return key_length == sizeof(beta) &&
           key_offset <= length &&
           sizeof(beta) <= length - key_offset &&
           memcmp(payload + key_offset, beta, sizeof(beta)) == 0;
}

static unsigned char *find_linear_beta_entry(unsigned char *payload, size_t length) {
    uint32_t case_count;
    size_t i;

    if (length < RX_JTABLE_HEADER_SIZE) return NULL;
    case_count = read_u32le(payload + 8u);
    if ((size_t)case_count >
        (length - RX_JTABLE_HEADER_SIZE) / RX_JTABLE_LINEAR_ENTRY_SIZE) return NULL;
    for (i = 0; i < case_count; i++) {
        unsigned char *entry = payload + RX_JTABLE_HEADER_SIZE +
                               i * RX_JTABLE_LINEAR_ENTRY_SIZE;
        if (key_is_beta(payload, length, read_u32le(entry), read_u32le(entry + 4u)))
            return entry;
    }
    return NULL;
}

static unsigned char *find_open_beta_slot(unsigned char *payload, size_t length) {
    uint32_t slot_count;
    size_t i;

    if (length < RX_JTABLE_OPEN_HEADER_SIZE) return NULL;
    slot_count = read_u32le(payload + 12u);
    if (slot_count == 0 ||
        (size_t)slot_count > (length - RX_JTABLE_OPEN_HEADER_SIZE) / RX_JTABLE_OPEN_SLOT_SIZE)
        return NULL;
    for (i = 0; i < slot_count; i++) {
        unsigned char *slot = payload + RX_JTABLE_OPEN_HEADER_SIZE +
                              i * RX_JTABLE_OPEN_SLOT_SIZE;
        uint32_t key_offset = read_u32le(slot + 4u);
        if (key_offset != RX_JTABLE_OPEN_EMPTY &&
            key_is_beta(payload, length, key_offset, read_u32le(slot + 8u)))
            return slot;
    }
    return NULL;
}

static unsigned char *find_acph_beta_slot(unsigned char *payload, size_t length) {
    uint32_t root_offset;
    uint16_t slot_count;
    size_t i;

    if (length < RX_JTABLE_ACPH_HEADER_SIZE) return NULL;
    root_offset = read_u32le(payload + 12u);
    if (root_offset > length ||
        RX_JTABLE_ACPH_NODE_SIZE > length - root_offset) return NULL;
    slot_count = read_u16le(payload + root_offset + 4u);
    if (slot_count == 0 ||
        (size_t)slot_count >
        (length - root_offset - RX_JTABLE_ACPH_NODE_SIZE) / RX_JTABLE_ACPH_SLOT_SIZE)
        return NULL;
    for (i = 0; i < slot_count; i++) {
        unsigned char *slot = payload + root_offset + RX_JTABLE_ACPH_NODE_SIZE +
                              i * RX_JTABLE_ACPH_SLOT_SIZE;
        if (slot[2] == RX_JTABLE_ACPH_SLOT_LEAF) {
            uint32_t leaf_offset = read_u32le(slot + 4u);
            if (leaf_offset <= length &&
                RX_JTABLE_ACPH_LEAF_SIZE <= length - leaf_offset &&
                key_is_beta(payload, length,
                            read_u32le(payload + leaf_offset),
                            read_u32le(payload + leaf_offset + 4u)))
                return slot;
        }
    }
    return NULL;
}

static int mutate_payload(unsigned char *payload, size_t length, const char *mutation) {
    unsigned char *slot;

    if (strcmp(mutation, "algorithm") == 0) {
        payload[0] = 0xffu;
        return 1;
    }
    if (strcmp(mutation, "header") == 0) {
        payload[2] = 0;
        payload[3] = 0;
        return 1;
    }
    if (strcmp(mutation, "linear-key-offset") == 0 &&
        payload[0] == RX_JTABLE_ALG_LINEAR &&
        length >= RX_JTABLE_HEADER_SIZE + RX_JTABLE_LINEAR_ENTRY_SIZE) {
        write_u32le(payload + RX_JTABLE_HEADER_SIZE, (uint32_t)length + 1u);
        return 1;
    }
    if (strcmp(mutation, "linear-target") == 0 &&
        payload[0] == RX_JTABLE_ALG_LINEAR) {
        slot = find_linear_beta_entry(payload, length);
        if (!slot) return 0;
        write_u32le(slot + 8u, UINT32_MAX);
        return 1;
    }
    if (strcmp(mutation, "open-slot-offset") == 0 &&
        payload[0] == RX_JTABLE_ALG_OPENHASH) {
        slot = find_open_beta_slot(payload, length);
        if (!slot) return 0;
        write_u32le(slot + 4u, (uint32_t)length + 1u);
        return 1;
    }
    if (strcmp(mutation, "acph-slot-kind") == 0 &&
        payload[0] == RX_JTABLE_ALG_ACPH) {
        slot = find_acph_beta_slot(payload, length);
        if (!slot) return 0;
        slot[2] = 0xffu;
        return 1;
    }
    if (strcmp(mutation, "acph-target") == 0 &&
        payload[0] == RX_JTABLE_ALG_ACPH) {
        uint32_t leaf_offset;
        slot = find_acph_beta_slot(payload, length);
        if (!slot) return 0;
        leaf_offset = read_u32le(slot + 4u);
        if (leaf_offset > length || RX_JTABLE_ACPH_LEAF_SIZE > length - leaf_offset)
            return 0;
        write_u32le(payload + leaf_offset + 8u, UINT32_MAX);
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    FILE *input;
    FILE *output;
    module_file *module = NULL;
    unsigned char *payload;
    size_t length = 0;
    int rc = 1;

    if (argc != 4) {
        fprintf(stderr, "usage: %s input.rxbin output.rxbin mutation\n", argv[0]);
        return 2;
    }
    input = fopen(argv[1], "rb");
    if (!input) {
        perror(argv[1]);
        return 1;
    }
    if (read_module(&module, input) != 0) {
        fprintf(stderr, "cannot read RXBIN module\n");
        fclose(input);
        return 1;
    }
    fclose(input);
    payload = find_jump_table(module, &length);
    if (!payload || !mutate_payload(payload, length, argv[3])) {
        fprintf(stderr, "cannot apply jump-table mutation %s\n", argv[3]);
        goto done;
    }
    output = fopen(argv[2], "wb");
    if (!output) {
        perror(argv[2]);
        goto done;
    }
    if (write_module(module, output) != 0) {
        fprintf(stderr, "cannot write mutated RXBIN module\n");
        fclose(output);
        goto done;
    }
    if (fclose(output) != 0) goto done;
    rc = 0;

done:
    free_module(module);
    return rc;
}
