//
// Common HTTP functions used by both the client and server
// Created by Adrian Sutherland on 11/05/2024.
//

#ifndef CREXX_HTTPCOMMON_H
#define CREXX_HTTPCOMMON_H

#include <stddef.h>
#include <stdbool.h>

#define INITIAL_BUFFER_SIZE 1024 // Also the minimum read size for the socket
#define MAX_STATUS_LINE_SIZE 100
#define MAX_HEADER_NAME 128
#define MAX_HEADER_VALUE 128

typedef struct DynamicBuffer {
    char *buffer;
    size_t size;
    size_t capacity;
} DynamicBuffer;

enum READING_PHASE {
    READING_STATUS_LINE,
    READING_HEADERS,
    READING_BODY,
    READING_TRAILERS,
    READING_COMPLETE
};

typedef struct HTTPResponse {
    // Parsing state
    enum READING_PHASE reading_phase;
    bool split_buffer; // Used to signal that we have split data into the next buffer (which should be processed next)
    int reading_error_code;
    bool chunked;
    char* last_chunk_start; // Position of the last partial (or unread) chunk that was processed
    size_t expected_body_size;

    // Response data
    int status_code;
    char status_line[MAX_STATUS_LINE_SIZE + 1];
    DynamicBuffer header_buffer;
    DynamicBuffer body_buffer;
    DynamicBuffer trailer_buffer;
} HTTPResponse;

// Reads a response from the socket
// Handles chunked and non-chunked encoding but the response is read into an in-memory buffer, so
// it is not suitable for huge / streamed responses
// Returns true if the response was read successfully
bool read_response(int sock, HTTPResponse *response);

// Makes sure the buffer is at least of  size (doubling the buffer size if need be)
void ensure_buffer_size(DynamicBuffer *buffer, size_t size);

// Checks for \r\n\r\n in the header buffer and if so returns true and splits the buffer to body buffer
bool split_header_buffer(HTTPResponse *response);

// Parses headers to find Content-Length or Transfer-Encoding: chunked
// This updates the response struct with the expected body size and if the body is chunked
// Returns true if the headers were parsed successfully
bool process_headers(HTTPResponse *response);

// Function to parse and clean the body buffer (if not chunked remove the end newline, if chunked remove the chunked encoding)
// returns true if the body was cleaned successfully
bool clean_body_buffer(HTTPResponse *response);

// Function to read a chunked response body buffer and determine if the response is complete
// If it is complete it also splits the buffer into the trailer buffer
// Returns true if the response is complete
bool is_chunking_complete(HTTPResponse *response);

// Frees the memory used by the buffer
void free_buffer(DynamicBuffer *buffer);

// Frees the memory used by the response
void free_response(HTTPResponse *response);

// Strip a string of leading and trailing whitespace characters in place
void strip_whitespace(char *str);

// Parse a header line to get the header name and value
// Max length of header name and value is MAX_HEADER_NAME / MAX_HEADER_VALUE characters.
// Characters beyond this length are ignored (but assumed correctly formatted)
// The line is terminated by a newline character
// Returns true if the header was parsed successfully
bool parse_header_line(char *line, char *header_name, char *header_value);

#endif //CREXX_HTTPCOMMON_H
