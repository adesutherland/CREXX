/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXVMPROCESSPROTOCOL_H
#define CREXX_RXVMPROCESSPROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define RXVM_PROCESS_PROTOCOL_VERSION 1u
#define RXVM_PROCESS_PROTOCOL_HEADER_SIZE 24u
#define RXVM_PROCESS_PROTOCOL_MAX_PAYLOAD (16u * 1024u * 1024u)

enum rxvm_process_frame_type {
    RXVM_PROCESS_FRAME_READY = 1,
    RXVM_PROCESS_FRAME_INVOKE = 2,
    RXVM_PROCESS_FRAME_STARTED = 3,
    RXVM_PROCESS_FRAME_RESULT = 4,
    RXVM_PROCESS_FRAME_CANCEL = 5,
    RXVM_PROCESS_FRAME_SHUTDOWN = 6
};

typedef struct rxvm_process_frame {
    uint16_t type;
    uint64_t request_id;
    unsigned char *payload;
    size_t payload_length;
} rxvm_process_frame;

void rxvm_process_frame_header(
        unsigned char header[RXVM_PROCESS_PROTOCOL_HEADER_SIZE],
        uint16_t type,
        uint64_t request_id,
        size_t payload_length);
int rxvm_process_frame_header_parse(
        const unsigned char header[RXVM_PROCESS_PROTOCOL_HEADER_SIZE],
        uint16_t *type_out,
        uint64_t *request_id_out,
        size_t *payload_length_out);
void rxvm_process_frame_free(rxvm_process_frame *frame);

#endif
