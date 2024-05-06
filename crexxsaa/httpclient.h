//
// Created by Adrian Sutherland on 26/03/2024.
//

#ifndef REXXSAAPOC_HTTPCLIENT_H
#define REXXSAAPOC_HTTPCLIENT_H

#include "crexxsaa.h"

// Parse Error Structure
typedef struct {
    int error_code;
    size_t position; // Note the position is approximate and may not be accurate
    char* message;
} PARSE_ERROR;

// parseJSON error codes
#define PARSE_ERROR_OK 0
#define PARSE_ERROR_EXPECTING_OPEN_CURLY 1
#define PARSE_ERROR_EXPECTING_SERVICE_BLOCKS 2
#define PARSE_ERROR_EXPECTING_STRING 3
#define PARSE_ERROR_NO_CLOSING_QUOTE 4
#define PARSE_ERROR_EXPECTING_COLON 5
#define PARSE_ERROR_EXPECTING_ARRAY 6
#define PARSE_ERROR_EXPECTING_CLOSE_CURLY 7
#define PARSE_ERROR_CREATE_SHVBLOCK_FAILED 8
#define PARSE_ERROR_DUPLICATE_NAME 9
#define PARSE_ERROR_DUPLICATE_REQUEST 10
#define PARSE_ERROR_INVALID_REQUEST 11
#define PARSE_ERROR_DUPLICATE_RESULT 12
#define PARSE_ERROR_INVALID_RESULT 13
#define PARSE_ERROR_DUPLICATE_VALUE 14
#define PARSE_ERROR_INVALID_ATTRIBUTE 15
#define PARSE_ERROR_MISSING_NAME 16
#define PARSE_ERROR_DUPLICATE_CLASS 17
#define PARSE_ERROR_DUPLICATE_MEMBERS 18
#define PARSE_ERROR_MEMORY_ALLOCATION 19
#define PARSE_ERROR_MISSING_CLASS 20
#define PARSE_ERROR_MISSING_MEMBERS 21
#define PARSE_ERROR_INVALID_TYPE 22
#define PARSE_ERROR_UNEXPECTED_TOKEN 23
#define PARSE_ERROR_EXPECTING_BASE64 25
#define PARSE_ERROR_INVALID_BASE64 26
#define PARSE_ERROR_INVALID_STRING 27
#define PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_CURLY 28
#define PARSE_ERROR_EXPECTED_COMMA_OR_CLOSE_ARRAY 29

// parseJSON - Parse JSON string and return SHVBLOCK structure
// json - JSON string to parse
// shvblock_handle - Pointer to SHVBLOCK handle
// error - Pointer to PARSE_ERROR structure (if this is NULL, the function will not return error details)
// Returns 0 if successful, otherwise returns an error code (see PARSE_ERROR structure for details
int parseJSON(char* json, SHVBLOCK** shvblock_handle, PARSE_ERROR* error);

// CREXXSAA JSON emitter functionS

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
typedef void (*emit_func)(emit_action action, const char* data, void** context);

// The JSON emitter function - for writing to stdout - this is used for logging and debugging
void emit_to_stdout(emit_action action, const char* data, __attribute__((unused)) void** context);

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
void emit_to_memory_buffer(emit_action action, const char* data, void** context);

// emit_func - The emit function signature
void jsonEMIT(SHVBLOCK* block, emit_func emit, void** context);

#endif //REXXSAAPOC_HTTPCLIENT_H
