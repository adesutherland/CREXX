//
// Created by Adrian Sutherland on 25/05/2024.
//

#ifndef CREXX_JSNEMIT_H
#define CREXX_JSNEMIT_H

#include "crexxsaa.h"

// CREXXSAA JSON  emitter functionS

// Define the emit function signature
// This has three parameters:
// 1. An enum indicating the type of the action (open, emit, finished_emit, close)
// 2. const char* data - The JSON data to emit (for emit action), or a parameter to the open/close logic (for open/close actions)
// 3. void** context - A handle to a context object that can be used by the emit function.
//                     It is a handle rather than a pointer to allow the emit function to
//                     change the context object if necessary (to allocate memory, for example).

// The emit_action enum definition
typedef enum {
    ACTION_OPEN,
    ACTION_EMIT,
    ACTION_FINISHED_EMIT,
    ACTION_CLOSE
} emit_action;

// The emit function signature
typedef int (*emit_func)(emit_action action, const char* data, void** context);

// The JSON emitter function - for writing to stdout - this is used for logging and debugging
// On success, returns 0, on error -1. Note in the case of an error the
// state of the context object is undefined as is the number of
// characters writen.
// Error conditions currently are only raised by socket errors (likely socket closing)
int emit_to_stdout(emit_action action, const char* data, __attribute__((unused)) void** context);

// The JSON emitter functions - for writing to a memory buffer
// This is used for capturing the JSON output in memory for further processing
// The memory buffer is dynamically allocated and grows as needed
typedef struct {
    char *data;          // Pointer to the dynamically allocated buffer
    size_t capacity;     // Current allocated size of the buffer
    size_t length;       // Current used length within the buffer
} MemoryBuffer;

// The JSON emitter function - for writing to a memory buffer - this automatically creates, grows, right sizes, and closes the buffer as needed
// depending on the action code
int emit_to_memory_buffer(emit_action action, const char* data, void** context);

// emit_func - The emit function that a block as JSON to the specified emit function
// Returns 0 on success, -1 on error
int jsonEMIT(SHVBLOCK* block, emit_func emit, void** context);

#endif //CREXX_JSNEMIT_H
