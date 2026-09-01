/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxbin.h"

void init_module(module_file *module) {
    memset(module, 0, sizeof(module_file)); /* Zero module file (valgrind complains otherwise) */
    memcpy(module->header.FILE_HEADER, BIN_HEADER, sizeof(BIN_HEADER));
    memcpy(module->header.FILE_VERSION, BIN_VERSION, sizeof(BIN_VERSION));
    module->header.record_type = RXBIN_RECORD_MODULE_LOCAL;
    module->fromfile = 0;
}

OpFormat rxbin_opcode_format(int opcode) {
    static const OpFormat opcode_formats[OP_MAX_INSTRUCTIONS] = {
#define X(NAME, OPCODE, FMT, FLOW, FLAGS, DESC) [OPCODE] = FMT,
#include "rxops.h"
#undef X
    };

    if (opcode < 0 || opcode >= OP_MAX_INSTRUCTIONS) return FMT_EMPTY;
    return opcode_formats[opcode];
}

void rxbin_byte_buffer_init(rxbin_byte_buffer *buffer) {
    buffer->data = 0;
    buffer->size = 0;
    buffer->capacity = 0;
}

void rxbin_byte_buffer_free(rxbin_byte_buffer *buffer) {
    if (buffer->data) free(buffer->data);
    buffer->data = 0;
    buffer->size = 0;
    buffer->capacity = 0;
}

int rxbin_byte_buffer_reserve(rxbin_byte_buffer *buffer, size_t extra) {
    size_t required;
    size_t new_capacity;
    unsigned char *new_data;

    if (extra > SIZE_MAX - buffer->size) return 0;
    required = buffer->size + extra;
    if (required <= buffer->capacity) return 1;

    new_capacity = buffer->capacity ? buffer->capacity : 64;
    while (new_capacity < required) {
        if (new_capacity > SIZE_MAX / 2) {
            new_capacity = required;
            break;
        }
        new_capacity *= 2;
    }

    new_data = realloc(buffer->data, new_capacity);
    if (!new_data) return 0;

    buffer->data = new_data;
    buffer->capacity = new_capacity;
    return 1;
}

static int rxbin_byte_buffer_append_byte(rxbin_byte_buffer *buffer, unsigned char value) {
    if (!rxbin_byte_buffer_reserve(buffer, 1)) return 0;
    buffer->data[buffer->size++] = value;
    return 1;
}

int rxbin_byte_buffer_append_bytes(rxbin_byte_buffer *buffer, const unsigned char *data, size_t size) {
    if (!size) return 1;
    if (!rxbin_byte_buffer_reserve(buffer, size)) return 0;
    memcpy(buffer->data + buffer->size, data, size);
    buffer->size += size;
    return 1;
}

static int rxbin_append_varuint_direct(rxbin_byte_buffer *buffer, uint64_t value) {
    unsigned char bytes[9];
    size_t count = 0;

    if (value <= UINT64_C(0x7F)) {
        bytes[count++] = (unsigned char)value;
    } else if (value <= UINT64_C(0x1FFF)) {
        bytes[count++] = (unsigned char)(0xC0u | ((value >> 8) & 0x1Fu));
        bytes[count++] = (unsigned char)(value & 0xFFu);
    } else if (value <= UINT64_C(0xFFFFF)) {
        bytes[count++] = (unsigned char)(0xE0u | ((value >> 16) & 0x0Fu));
        bytes[count++] = (unsigned char)((value >> 8) & 0xFFu);
        bytes[count++] = (unsigned char)(value & 0xFFu);
    } else if (value <= UINT64_C(0x7FFFFFF)) {
        bytes[count++] = (unsigned char)(0xF0u | ((value >> 24) & 0x07u));
        bytes[count++] = (unsigned char)((value >> 16) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 8) & 0xFFu);
        bytes[count++] = (unsigned char)(value & 0xFFu);
    } else if (value <= UINT64_C(0x3FFFFFFFF)) {
        bytes[count++] = (unsigned char)(0xF8u | ((value >> 32) & 0x03u));
        bytes[count++] = (unsigned char)((value >> 24) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 16) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 8) & 0xFFu);
        bytes[count++] = (unsigned char)(value & 0xFFu);
    } else if (value <= UINT64_C(0x1FFFFFFFFFF)) {
        bytes[count++] = (unsigned char)(0xFCu | ((value >> 40) & 0x01u));
        bytes[count++] = (unsigned char)((value >> 32) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 24) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 16) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 8) & 0xFFu);
        bytes[count++] = (unsigned char)(value & 0xFFu);
    } else if (value <= UINT64_C(0xFFFFFFFFFFFF)) {
        bytes[count++] = 0xFEu;
        bytes[count++] = (unsigned char)((value >> 40) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 32) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 24) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 16) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 8) & 0xFFu);
        bytes[count++] = (unsigned char)(value & 0xFFu);
    } else {
        bytes[count++] = 0xFFu;
        bytes[count++] = (unsigned char)((value >> 56) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 48) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 40) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 32) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 24) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 16) & 0xFFu);
        bytes[count++] = (unsigned char)((value >> 8) & 0xFFu);
        bytes[count++] = (unsigned char)(value & 0xFFu);
    }

    return rxbin_byte_buffer_append_bytes(buffer, bytes, count);
}

void rxbin_var_writer_init(rxbin_var_writer *writer, rxbin_byte_buffer *buffer) {
    writer->buffer = buffer;
    writer->pending_tiny = 0;
    writer->have_pending_tiny = 0;
}

int rxbin_var_writer_write(rxbin_var_writer *writer, uint64_t value) {
    if (value <= UINT64_C(7)) {
        if (writer->have_pending_tiny) {
            unsigned char pair_byte = (unsigned char)(0x80u | (writer->pending_tiny << 3) | (unsigned char)value);
            writer->have_pending_tiny = 0;
            return rxbin_byte_buffer_append_byte(writer->buffer, pair_byte);
        }
        writer->pending_tiny = (unsigned char)value;
        writer->have_pending_tiny = 1;
        return 1;
    }

    if (writer->have_pending_tiny) {
        if (!rxbin_append_varuint_direct(writer->buffer, writer->pending_tiny)) return 0;
        writer->have_pending_tiny = 0;
    }

    return rxbin_append_varuint_direct(writer->buffer, value);
}

int rxbin_var_writer_flush(rxbin_var_writer *writer) {
    if (!writer->have_pending_tiny) return 1;
    writer->have_pending_tiny = 0;
    return rxbin_append_varuint_direct(writer->buffer, writer->pending_tiny);
}

void rxbin_var_reader_init(rxbin_var_reader *reader, const unsigned char *data, size_t size) {
    reader->cursor = data;
    reader->end = data + size;
    reader->queued_value = 0;
    reader->have_queued_value = 0;
}

int rxbin_var_reader_read(rxbin_var_reader *reader, uint64_t *value) {
    unsigned char first;

    if (reader->have_queued_value) {
        reader->have_queued_value = 0;
        *value = reader->queued_value;
        return 1;
    }

    if (reader->cursor >= reader->end) return 0;
    first = *(reader->cursor++);

    if ((first & 0x80u) == 0) {
        *value = first;
        return 1;
    }

    if ((first & 0xC0u) == 0x80u) {
        *value = (uint64_t)((first >> 3) & 0x07u);
        reader->queued_value = (uint64_t)(first & 0x07u);
        reader->have_queued_value = 1;
        return 1;
    }

    if ((first & 0xE0u) == 0xC0u) {
        if (reader->end - reader->cursor < 1) return 0;
        *value = ((uint64_t)(first & 0x1Fu) << 8) |
                 (uint64_t)*(reader->cursor++);
        return 1;
    }

    if ((first & 0xF0u) == 0xE0u) {
        if (reader->end - reader->cursor < 2) return 0;
        *value = ((uint64_t)(first & 0x0Fu) << 16) |
                 ((uint64_t)reader->cursor[0] << 8) |
                 (uint64_t)reader->cursor[1];
        reader->cursor += 2;
        return 1;
    }

    if ((first & 0xF8u) == 0xF0u) {
        if (reader->end - reader->cursor < 3) return 0;
        *value = ((uint64_t)(first & 0x07u) << 24) |
                 ((uint64_t)reader->cursor[0] << 16) |
                 ((uint64_t)reader->cursor[1] << 8) |
                 (uint64_t)reader->cursor[2];
        reader->cursor += 3;
        return 1;
    }

    if ((first & 0xFCu) == 0xF8u) {
        if (reader->end - reader->cursor < 4) return 0;
        *value = ((uint64_t)(first & 0x03u) << 32) |
                 ((uint64_t)reader->cursor[0] << 24) |
                 ((uint64_t)reader->cursor[1] << 16) |
                 ((uint64_t)reader->cursor[2] << 8) |
                 (uint64_t)reader->cursor[3];
        reader->cursor += 4;
        return 1;
    }

    if ((first & 0xFEu) == 0xFCu) {
        if (reader->end - reader->cursor < 5) return 0;
        *value = ((uint64_t)(first & 0x01u) << 40) |
                 ((uint64_t)reader->cursor[0] << 32) |
                 ((uint64_t)reader->cursor[1] << 24) |
                 ((uint64_t)reader->cursor[2] << 16) |
                 ((uint64_t)reader->cursor[3] << 8) |
                 (uint64_t)reader->cursor[4];
        reader->cursor += 5;
        return 1;
    }

    if (first == 0xFEu) {
        if (reader->end - reader->cursor < 6) return 0;
        *value = ((uint64_t)reader->cursor[0] << 40) |
                 ((uint64_t)reader->cursor[1] << 32) |
                 ((uint64_t)reader->cursor[2] << 24) |
                 ((uint64_t)reader->cursor[3] << 16) |
                 ((uint64_t)reader->cursor[4] << 8) |
                 (uint64_t)reader->cursor[5];
        reader->cursor += 6;
        return 1;
    }

    if (reader->end - reader->cursor < 8) return 0;
    *value = ((uint64_t)reader->cursor[0] << 56) |
             ((uint64_t)reader->cursor[1] << 48) |
             ((uint64_t)reader->cursor[2] << 40) |
             ((uint64_t)reader->cursor[3] << 32) |
             ((uint64_t)reader->cursor[4] << 24) |
             ((uint64_t)reader->cursor[5] << 16) |
             ((uint64_t)reader->cursor[6] << 8) |
             (uint64_t)reader->cursor[7];
    reader->cursor += 8;
    return 1;
}


static void rxbin_shared_pool_release(rxbin_shared_constant_pool **pool_ref) {
    rxbin_shared_constant_pool *pool;

    if (!pool_ref || !*pool_ref) return;
    pool = *pool_ref;
    if (pool->refcount) pool->refcount--;
    if (!pool->refcount) {
        free(pool->data);
        free(pool);
    }
    *pool_ref = 0;
}

void free_module(module_file *module) {
    if (!module) return;
    if (module->semantic_graph) rx_graph_release(&module->semantic_graph);
    if (module->shared_constant_pool) {
        rxbin_shared_pool_release(&module->shared_constant_pool);
        module->constant = 0;
    }
    if (module->fromfile || module->native) {
        free(module->name);
        free(module->description);
        free(module->instructions);
        free(module->constant);
    }
    free(module);
}
