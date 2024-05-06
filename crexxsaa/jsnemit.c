// CREXXSAA JASON EMITTER
// JSON emitter for CREXXSAA
// Reads a SHVBLOCK and emits JSON
//
// Created by Adrian Sutherland on 01/05/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include "httpclient.h"

// The JSON emitter function - for writing to stdout - this is used for logging and debugging
__attribute__((unused)) void emit_to_stdout(emit_action action, const char* data, __attribute__((unused)) void** context) {
    switch (action) {
        case ACTION_OPEN:
            printf("JSON START\n");
            break;
        case ACTION_EMIT:
            // Output the JSON data with no additional formatting or newline
            printf("%s", data);
            break;
        case ACTION_FINISHED_EMIT:
            printf("\nJSON END\n");
            break;
        default:
            break;
    }
}

// Function to create a memory buffer
MemoryBuffer *create_memory_buffer(size_t initial_capacity) {
    MemoryBuffer *buffer = malloc(sizeof(MemoryBuffer));
    if (buffer == NULL) {
        // Memory allocation error handling - panic and exit
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    buffer->data = malloc(initial_capacity);
    if (buffer->data == NULL) {
        // Memory allocation error handling - panic and exit
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    buffer->capacity = initial_capacity;
    buffer->length = 0;
    return buffer;
}

// Function to give memory back from the buffer, so it just has enough for the length
void shrink_memory_buffer(MemoryBuffer *buffer) {
    char *new_data = realloc(buffer->data, buffer->length);
    if (new_data != NULL) {
        buffer->data = new_data;
        buffer->capacity = buffer->length;
    }
    // If realloc fails, the original buffer is still valid
}
// Function to write data to a memory buffer
void write_to_memory_buffer(MemoryBuffer *buffer, const char *data, size_t data_length) {
    // Ensure enough capacity
    if (buffer->length + data_length > buffer->capacity) {
        size_t new_capacity = buffer->capacity * 2;  // Doubling strategy
        char *new_data = realloc(buffer->data, new_capacity);
        if (new_data == NULL) {
            // Handle memory allocation errors
            // For simplicity just panic and exit
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }

    // Copy data
    memcpy(buffer->data + buffer->length, data, data_length);
    buffer->length += data_length;
}

// Function to free a memory buffer
void free_memory_buffer(MemoryBuffer *buffer) {
    free(buffer->data);
    free(buffer);
}

// The JSON emitter function - for writing to a memory buffer - this automatically creates, grows, and right sizes the buffer as needed
void emit_to_memory_buffer(emit_action action, const char* data, void** context) {
    MemoryBuffer *buffer = (MemoryBuffer *) *context;

    switch (action) {
        case ACTION_OPEN:
            buffer = create_memory_buffer(512);
            *context = buffer; // Store the context back
            break;

        case ACTION_EMIT:
            write_to_memory_buffer(buffer, data, strlen(data));
            break;

        case ACTION_FINISHED_EMIT:
            // Reduce the memory buffer to the exact size needed
            shrink_memory_buffer(buffer);
            break;

        case ACTION_CLOSE:
            free_memory_buffer(buffer);
            *context = NULL;  // Reset context
            break;
    }
}

// Forward declarations
void objblockEMITasArray(OBJBLOCK *object, emit_func emit, void** context); // Emit the OBJBLOCK as an array
void objblockEMITasObject(OBJBLOCK *object, emit_func emit, void** context); // Emit the OBJBLOCK as an object
void objblockEMIT(OBJBLOCK *shvobject, emit_func emit, void** context); // Emit the OBJBLOCK and members
void shvblockEMIT(SHVBLOCK *shvblock, emit_func emit, void** context); // Emit the SHVBLOCK
const char *shvcode_to_string(unsigned long shvcode); // Convert shvcode to a string
const char *shvret_to_string(unsigned long shvret); // Convert shvret to a string
char* base64_encode(const char *binary_data, size_t length); // Base64 encode function

// Emit the full SHVBLOCK structure as JSON
// This is the top-level function that calls the other emit functions
// example JSON:
//             {
//                serviceBlocks": [
//                     ... comma delimited service blocks ...
//                ]
//             }
void jsonEMIT(SHVBLOCK* block, emit_func emit, void** context) {
    // Start
    emit(ACTION_OPEN, NULL, context);
    emit(ACTION_EMIT, "{\"serviceBlocks\":[", context);

    /* Process the SHVBLOCK linked list */
    while (block) {
        shvblockEMIT(block, emit, context);
        if (block->shvnext)
            emit(ACTION_EMIT, ",", context);
        block = block->shvnext;
    }

    // End
    emit(ACTION_EMIT, "]}", context);
    emit(ACTION_FINISHED_EMIT, NULL, context);
}

// Emit the SHVBLOCK, e.g.
//                     {
//                        "name": "test",
//                        "request": "set",
//                        "result": "ok",
//                        "value": ... value ...
//                    }
void shvblockEMIT(SHVBLOCK *shvblock, emit_func emit, void** context) {
    // Start
    emit(ACTION_EMIT, "{", context);

    // Emit the name
    emit(ACTION_EMIT, "\"name\":\"", context);
    emit(ACTION_EMIT, shvblock->shvname, context);
    emit(ACTION_EMIT, "\",", context);

    // Emit the request
    emit(ACTION_EMIT, "\"request\":\"", context);
    emit(ACTION_EMIT, shvcode_to_string(shvblock->shvcode), context);
    emit(ACTION_EMIT, "\",", context);

    // Emit the result
    emit(ACTION_EMIT, "\"result\":\"", context);
    emit(ACTION_EMIT, shvret_to_string(shvblock->shvret), context);
    emit(ACTION_EMIT, "\",", context);

    // Emit the value
    emit(ACTION_EMIT, "\"value\":", context);
    objblockEMIT(shvblock->shvobject, emit, context);

    // End
    emit(ACTION_EMIT, "}", context);
}

// Emit the OBJBLOCK and members
void objblockEMIT(OBJBLOCK *shvobject, emit_func emit, void** context) { // NOLINT(misc-no-recursion) - suppress the clang-tidy warning about recursion
    char num_str[32]; // Buffer for number conversion
    char *base64_data;

    switch (shvobject->type) {
        case VALUE_STRING:
            emit(ACTION_EMIT, "\"", context);
            emit(ACTION_EMIT, shvobject->value.string, context);
            emit(ACTION_EMIT, "\"", context);
            break;
        case VALUE_NULL:
            emit(ACTION_EMIT, "null", context);
            break;
        case VALUE_BINARY: // Emit the binary data as base64
            base64_data = base64_encode(shvobject->value.binary.data, shvobject->value.binary.length);
            emit(ACTION_EMIT, "{\"base64\":", context);
            emit(ACTION_EMIT, "\"", context);
            emit(ACTION_EMIT, base64_data, context);
            emit(ACTION_EMIT, "\"}", context);
            free(base64_data);
            break;
        case VALUE_INT:
            // Convert the integer to a string
            snprintf(num_str, sizeof(num_str), "%ld", shvobject->value.integer);
            emit(ACTION_EMIT, num_str, context);
            break;
        case VALUE_FLOAT:
            // Convert the float to a string - with no trailing zeros
            snprintf(num_str, sizeof(num_str), "%g", shvobject->value.real);
            emit(ACTION_EMIT, num_str, context);
            break;
        case VALUE_BOOL:
            emit(ACTION_EMIT, shvobject->value.boolean ? "true" : "false", context);
            break;
        case VALUE_ARRAY:
            objblockEMITasArray(shvobject, emit, context);
            break;
        case VALUE_OBJECT:
            objblockEMITasObject(shvobject, emit, context);
            break;

    }
}

// Convert shvcode to a string - the reverse of parseRequestCode() in jsnparse.c
// The result is one of "set", "fetch", "drop", "nextv", "priv", "syset", "syfet", "sydro", "sydel" (or "unknown")
const char *shvcode_to_string(unsigned long shvcode) {
    switch (shvcode) {
        case RXSHV_SET:
            return "set";
        case RXSHV_FETCH:
            return "fetch";
        case RXSHV_DROP:
            return "drop";
        case RXSHV_NEXTV:
            return "nextv";
        case RXSHV_PRIV:
            return "priv";
        case RXSHV_SYSET:
            return "syset";
        case RXSHV_SYFET:
            return "syfet";
        case RXSHV_SYDRO:
            return "sydro";
        case RXSHV_SYDEL:
            return "sydel";
        default:
            return "unknown";
    }
}

// Convert shvret to a string
// The result  is one of "ok", "newv", "lvar", "trunc", "badn", "memfl", "badf", "noavl", "notex", (or "unknown")
const char *shvret_to_string(unsigned long shvret) {
    switch (shvret) {
        case RXSHV_OK:
            return "ok";
        case RXSHV_NEWV:
            return "newv";
        case RXSHV_LVAR:
            return "lvar";
        case RXSHV_TRUNC:
            return "trunc";
        case RXSHV_BADN:
            return "badn";
        case RXSHV_MEMFL:
            return "memfl";
        case RXSHV_BADF:
            return "badf";
        case RXSHV_NOAVL:
            return "noavl";
        case RXSHV_NOTEX:
            return "notex";
        default:
            return "unknown";
    }
}

// Emit the OBJBLOCK as an array
void objblockEMITasArray(OBJBLOCK *object, emit_func emit, void** context) { // NOLINT(misc-no-recursion) - suppress the clang-tidy warning about recursion
    MEMBLOCK *memblock = object->value.members;
    emit(ACTION_EMIT, "[", context);

    while (memblock) {
        emit(ACTION_EMIT, "{\"", context);
        emit(ACTION_EMIT, memblock->membername, context);
        emit(ACTION_EMIT, "\":", context);
        objblockEMIT(memblock->memberobject, emit, context);
        emit(ACTION_EMIT, "}", context);

        if (memblock->membernext)
            emit(ACTION_EMIT, ",", context);

        memblock = memblock->membernext;
    }

    emit(ACTION_EMIT, "]", context);
}

// Emit the OBJBLOCK as an object
void objblockEMITasObject(OBJBLOCK *object, emit_func emit, void** context) { // NOLINT(misc-no-recursion) - suppress the clang-tidy warning about recursion
    MEMBLOCK *memblock = object->value.members;
    emit(ACTION_EMIT, "{", context);
    // Emit the typename
    emit(ACTION_EMIT, "\"class\":\"", context);
    emit(ACTION_EMIT, object->typename, context);
    emit(ACTION_EMIT, "\",", context);
    // Emit the members
    emit(ACTION_EMIT, "\"members\":{", context);
    while (memblock) {
        emit(ACTION_EMIT, "\"", context);
        emit(ACTION_EMIT, memblock->membername, context);
        emit(ACTION_EMIT, "\":", context);
        objblockEMIT(memblock->memberobject, emit, context);

        if (memblock->membernext)
            emit(ACTION_EMIT, ",", context);

        memblock = memblock->membernext;
    }

    emit(ACTION_EMIT, "}", context); // End members
    emit(ACTION_EMIT, "}", context); // End object
}

// Base64 encode function - encoded output is returned in a malloced buffer that needs to be freed by the caller
// The function panics and exits on internal (malloc) errors
char* base64_encode(const char *binary_data, size_t length) {
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t output_length = 4 * ((length + 2) / 3);
    char* encoded_data = malloc(output_length + 1); // +1 for the null-terminator

    if (encoded_data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    size_t i, j;
    for (i = 0, j = 0; i < length;) {
        uint32_t octet_a = i < length ? (unsigned char)binary_data[i++] : 0;
        uint32_t octet_b = i < length ? (unsigned char)binary_data[i++] : 0;
        uint32_t octet_c = i < length ? (unsigned char)binary_data[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }

    // Add required padding '=' characters replacing 'A's
    for (i = 0; i < 3 - length % 3; i++) {
        if (i < 2 || length % 3 != 0) { // Don't add '=' if length is a multiple of 3
            encoded_data[output_length - 1 - i] = '=';
        }
    }
    // And now remove padding '=' characters - probably not efficient, but simple
    while (encoded_data[j - 1] == '=') {
        j--;
    }

    // Null-terminate the string
    encoded_data[j] = '\0';

    return encoded_data;
}
