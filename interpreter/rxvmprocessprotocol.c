/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmprocessprotocol.h"

#include <stdlib.h>
#include <string.h>

static void process_put_u16(unsigned char *data, uint16_t value) {
    data[0] = (unsigned char)value;
    data[1] = (unsigned char)(value >> 8u);
}

static void process_put_u64(unsigned char *data, uint64_t value) {
    unsigned int index;
    for (index = 0u; index < 8u; index++) {
        data[index] = (unsigned char)(value >> (index * 8u));
    }
}

static uint16_t process_u16(const unsigned char *data) {
    return (uint16_t)data[0] | (uint16_t)((uint16_t)data[1] << 8u);
}

static uint64_t process_u64(const unsigned char *data) {
    uint64_t value = 0u;
    unsigned int index;
    for (index = 0u; index < 8u; index++) {
        value |= (uint64_t)data[index] << (index * 8u);
    }
    return value;
}

void rxvm_process_frame_header(
        unsigned char header[RXVM_PROCESS_PROTOCOL_HEADER_SIZE],
        uint16_t type,
        uint64_t request_id,
        size_t payload_length) {
    memset(header, 0, RXVM_PROCESS_PROTOCOL_HEADER_SIZE);
    memcpy(header, "RXPW", 4u);
    process_put_u16(header + 4u, RXVM_PROCESS_PROTOCOL_VERSION);
    process_put_u16(header + 6u, type);
    process_put_u64(header + 8u, request_id);
    process_put_u64(header + 16u, (uint64_t)payload_length);
}

int rxvm_process_frame_header_parse(
        const unsigned char header[RXVM_PROCESS_PROTOCOL_HEADER_SIZE],
        uint16_t *type_out,
        uint64_t *request_id_out,
        size_t *payload_length_out) {
    uint64_t payload_length;
    uint16_t type;
    if (!header || memcmp(header, "RXPW", 4u) ||
        process_u16(header + 4u) != RXVM_PROCESS_PROTOCOL_VERSION) return 0;
    type = process_u16(header + 6u);
    payload_length = process_u64(header + 16u);
    if (type < RXVM_PROCESS_FRAME_READY ||
        type > RXVM_PROCESS_FRAME_SHUTDOWN ||
        payload_length > RXVM_PROCESS_PROTOCOL_MAX_PAYLOAD ||
        payload_length > SIZE_MAX) return 0;
    if (type_out) *type_out = type;
    if (request_id_out) *request_id_out = process_u64(header + 8u);
    if (payload_length_out) *payload_length_out = (size_t)payload_length;
    return 1;
}

void rxvm_process_frame_free(rxvm_process_frame *frame) {
    if (!frame) return;
    free(frame->payload);
    memset(frame, 0, sizeof(*frame));
}
