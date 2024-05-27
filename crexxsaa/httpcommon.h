//
// Common HTTP functions used by both the client and server
// Created by Adrian Sutherland on 11/05/2024.
//

#ifndef CREXX_HTTPCOMMON_H
#define CREXX_HTTPCOMMON_H

#include <stddef.h>
#include <stdbool.h>
#include "jsnemit.h"

#define INITIAL_BUFFER_SIZE 1024 // Also the minimum read size for the socket
#define MAX_HEADER_NAME 128
#define MAX_HEADER_VALUE 128

enum READING_PHASE {
    READING_STATUS_LINE,
    READING_HEADERS,
    READING_BODY,
    READING_TRAILERS,
    READING_COMPLETE
};

typedef struct HTTPResponse {
    // Common buffer
    char *buffer;
    size_t size; // Current size of data in the buffer
    size_t capacity; // Buffer capacity
    size_t status_line_start; // Position of the start of the status line in the buffer (always 0?)
    size_t status_line_length; // Length of the status line (excluding a null terminator)
    size_t header_start; // Position of the start of the headers in the buffer
    size_t header_length; // Length of the headers (excluding a null terminator)
    size_t body_start; // Position of the start of the body in the buffer
    size_t body_length; // Length of the body (excluding a null terminator)
    size_t trailer_start; // Position of the start of the trailers in the buffer
    size_t trailer_length; // Length of the trailers (excluding a null terminator)

    // Parsing state
    enum READING_PHASE reading_phase;
    size_t cursor; // Current parse position in the buffer
    int reading_error_code;
    bool chunked;
    bool expecting_trailers;
    size_t last_chunk_start; // Position of the last partial (or unread) chunk that was processed
    size_t expected_body_size;

    // Extracted data
    int status_code;
} HTTPResponse;

#define READ_ERROR_NONE (0)
#define READ_ERROR_SOCKET_CLOSED (-1)
#define READ_ERROR_SOCKET_ERROR (-2)
#define READ_ERROR_INVALID_STATUS_CODE (-3)
#define READ_ERROR_MALFORMED_STATUS_LINE (-4)
#define READ_ERROR_MALFORMED_HEADER (-5)
#define READ_ERROR_MALFORMED_CHUNK (-6)
#define READ_ERROR_MALFORMED_BODY (-7)
#define READ_ERROR_MALFORMED_TRAILER (-8)
#define READ_ERROR_EXTRANEOUS_DATA (-10)


// macros to access the buffer (status line, headers, body, trailers)
#define STATUS_LINE(response) ((response)->buffer + (response)->status_line_start)
#define HEADERS(response) ((response)->buffer + (response)->header_start)
#define BODY(response) ((response)->buffer + (response)->body_start)
#define TRAILERS(response) ((response)->buffer + (response)->trailer_start)


// Reads a response from the socket and parses it
// *** This is the function clients use ***
// Handles chunked and non-chunked encoding but the response is read into an in-memory buffer, so
// it is not suitable for huge / streamed responses
// Returns true if the response was read successfully
bool read_response(int sock, HTTPResponse *response);

// Read data from the socket into the buffer
// Returns READ_ERROR_NONE on success, <0 (READ_ERROR_...) > on an error or if the socket is closed (setting reading_error_code and reading_phase)
int read_into_buffer(int sock, HTTPResponse *response);

// Makes sure the response buffer is at least of  size (doubling the buffer size if need be)
void ensure_buffer_size(HTTPResponse *response, size_t size);

// Parses headers to find Content-Length or Transfer-Encoding: chunked
// This updates the response struct with the expected body size and if the body is chunked
// Returns true if the headers were parsed successfully
bool process_headers(HTTPResponse *response);

// Function to check if the body is complete
// If chunked it reads the chunked response body buffer and determine if the response is complete
// Otherwise it checks if the body buffer is complete
// If it is complete it also splits the buffer into the trailer buffer
// Returns true if the response is complete
bool is_body_complete(HTTPResponse *response);

// Function to parse and clean the body buffer
// returns true if the body was cleaned successfully
bool process_body(HTTPResponse *response);

// Parses trailers to find specific headers (to be defined)
// Returns true if the trailers were parsed successfully
bool process_trailers(HTTPResponse *response);

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

// The JSON emitter functions for writing to socket using the HTTP chunked protocole
// This is used for sending JSON data over a socket connection
// The function buffers output into chuck sizes and sends the chunks over the socket
// context structure holding the buffer for the currect chunk, size and socket
#define SOCKET_BUFFER_SIZE 512
typedef struct {
    char *buffer;         // Pointer to the buffer
    size_t capacity;    // Current allocated size of the buffer
    size_t length;       // Current used length within the buffer
    int socket;            // The socket to write the buffer to
    char *secret_id;  // The secret id to use in the HTTP header
    char *http_request; // The HTTP request to use (POST, PUT, etc.)
    char *http_path;    // The HTTP path to use
    char *http_headers; // Additional HTTP headers to use
    char *http_host;    // The HTTP host to use
} SocketBuffer;

// Function to flush a SocketBuffer buffer to the socket
// This function is used by the emit_to_socket() function
// Returns 0 on success, -1 on error
int flush_socket_buffer(SocketBuffer *buffer);

// Function to write data to a SocketBuffer buffer. When the buffer is full it is flushed to the socket
// This function is used by the emit_to_socket() function
// If the length is bigger than the buffer capacity the buffer is flushed and the data is written directly to the socket
// Returns 0 on success, -1 on error
int write_to_socket_buffer(SocketBuffer *buffer, const char *data, size_t length);

// Function to flush a SocketBuffer buffer to the socket as a chunk
// This function is used by the emit_to_socket() function
// Returns 0 on success, -1 on error
int flush_chunked_socket_buffer(SocketBuffer *buffer);

// Function to write data to a SocketBuffer buffer. When the buffer is full it is flushed to the socket
// This function generates HTTP body chunked encoding
// This function is used by the emit_to_socket() function
// If the length is bigger than the buffer capacity then it is added to the buffer and flushed in chunks
// Returns 0 on success, -1 on error
int write_to_chunked_socket_buffer(SocketBuffer *buffer, const char *data, size_t length);

// The JSON emitter functions for writing to socket using the HTTP chunked protocol
// This is used for sending JSON data over a socket connection
// The function buffers output into chuck sizes and sends the chunks over the socket
// context structure holding the buffer for the current chunk, size and socket
// Returns 0 on success, -1 on error
int emit_to_socket(emit_action action, const char* data, void** context);

#endif //CREXX_HTTPCOMMON_H
