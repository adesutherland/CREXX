//
// Common HTTP functions used by both the client and server
// Created by Adrian Sutherland on 11/05/2024.
//

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "httpcommon.h"

// Reads a response from the socket
// Handles chunked and non-chunked encoding but the response is read into an in-memory buffer, so
// it is not suitable for huge / streamed responses
// Returns true if the response was read successfully
bool read_response(int sock, HTTPResponse *response) {
    // Initialize the response
    response->reading_error_code = 0;
    response->reading_phase = READING_STATUS_LINE;
    response->cursor = 0;
    response->status_code = 0;
    response->chunked = false;
    response->expecting_trailers = false;
    response->last_chunk_start = 0;
    response->expected_body_size = 0;
    response->size = 0;
    response->status_line_start = 0;
    response->status_line_length = 0;
    response->header_start = 0;
    response->header_length = 0;
    response->body_start = 0;
    response->body_length = 0;
    response->trailer_start = 0;
    response->trailer_length = 0;

    // Read the response into the buffer
    if (read_into_buffer(sock, response)) return false;

    // Process the status line
    while (response->reading_phase == READING_STATUS_LINE) {
        char *crlf = strstr(response->buffer, "\r\n");
        if (crlf) {
            *crlf = '\0';
            response->status_line_start = 0;
            response->status_line_length = crlf - response->buffer;

            // Fine the second word in the status line (the status code)
            char *space = strchr(STATUS_LINE(response), ' ');
            if (space) {
                // Get the status code
                response->status_code = (int)strtol(space, NULL, 10);
                // Check if the status code is valid
                if (response->status_code < 100 || response->status_code > 599) {
                    response->reading_phase = READING_COMPLETE;
                    response->reading_error_code = READ_ERROR_INVALID_STATUS_CODE;
                    return false;
                }
            }
            else {
                response->reading_phase = READING_COMPLETE;
                response->reading_error_code = READ_ERROR_MALFORMED_STATUS_LINE;
                return false;
            }

            // Set the start of the header buffer
            response->header_start = crlf - response->buffer + 2;

            // We are now ready to read the headers
            response->reading_phase = READING_HEADERS;
        }
        else {
            // Read from the socket
            if (read_into_buffer(sock, response)) return false;
        }
    }

    // Process the headers
    while (response->reading_phase == READING_HEADERS) {
        // Check if we have a double newline
        char *crlf_crlf = strstr(response->buffer + response->header_start, "\r\n\r\n");
        if (crlf_crlf) {
            // We have a double newline
            crlf_crlf += 2; // The first newline is part of the headers
            *crlf_crlf = '\0';

            response->header_length = crlf_crlf - response->buffer - response->header_start;

            // Process the headers to find the expected body size
            if (!process_headers(response)) return false;

            // We are ready to read the body
            response->reading_phase = READING_BODY;
            response->body_start = crlf_crlf - response->buffer + 2;
        }

        // If we don't have a double newline we need to read more headers
        else {
            // Read more headers from the socket
            if (read_into_buffer(sock, response)) return false;
        }
    }

    // Process the body
    while (response->reading_phase == READING_BODY) {
        if (is_body_complete(response)) {
            if (!process_body(response)) return false;
        }
        else {
            // Check if there was an error reading the body
            if (response->reading_error_code) return false;

            // Read the body from the socket
            if (read_into_buffer(sock, response)) return false;
        }
    }

    // Process the trailers
    while (response->reading_phase == READING_TRAILERS) {
        // Check if we have a double newline
        char *crlf_crlf = strstr(response->buffer + response->trailer_start, "\r\n\r\n");
        if (crlf_crlf) {
            // We have a double newline
            crlf_crlf += 2; // The first newline is part of the trailers
            *crlf_crlf = '\0';

            response->trailer_length = crlf_crlf - response->buffer - response->trailer_start;

            // Process the headers to find the expected body size
            if (!process_trailers(response)) return false;

            // We are complete
            response->reading_phase = READING_COMPLETE;
        }

        // If we don't have a double newline we need to read more headers
        else {
            // Read more headers from the socket
            if (read_into_buffer(sock, response)) return false;
        }
    }

    // Check if there is any unprocessed data in the buffer
    if (response->size > response->trailer_start + response->trailer_length) {
        // Respond with an error
        response->reading_error_code = READ_ERROR_EXTRANEOUS_DATA;
        return false;
    }

    // All done
    return true;
}

// Makes sure the buffer is at least of  size (doubling the buffer size if need be)
void ensure_buffer_size(HTTPResponse *response, size_t size) {
    char *new_buffer;
    if (size < INITIAL_BUFFER_SIZE) size = INITIAL_BUFFER_SIZE;
    if (response->capacity < size) {
        size_t new_capacity = response->capacity * 2;
        if (new_capacity < size) new_capacity = size;

        // Use realloc to resize the buffer
        if (response->buffer == NULL) {
            new_buffer = malloc(new_capacity);
        }
        else {
            new_buffer = realloc(response->buffer, new_capacity);
        }
        if (new_buffer) {
            response->buffer = new_buffer;
            response->capacity = new_capacity;
        }
        else {
            // realloc failed - we just panic and exit on memory allocation failures
            // Print an error message to stderr
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
}

// Read data from the socket into the buffer
// Returns READ_ERROR_NONE on success, <0 (READ_ERROR_...) > on an error or if the socket is closed (setting reading_error_code and reading_phase)
int read_into_buffer(int sock, HTTPResponse *response) {
    ssize_t bytes_read;

    // Ensure we have at least INITIAL_BUFFER_SIZE bytes spare in the buffer for the read
    if (response->capacity - response->size < INITIAL_BUFFER_SIZE) {
        ensure_buffer_size(response, response->capacity + INITIAL_BUFFER_SIZE);
    }

    // Read from the socket
    bytes_read = read(sock, response->buffer + response->size, response->capacity - response->size);
    if (bytes_read < 0) {
        response->reading_phase = READING_COMPLETE;
        response->reading_error_code = READ_ERROR_SOCKET_ERROR;
        return READ_ERROR_SOCKET_ERROR;
    }

    // Socket Closed
    if (bytes_read == 0) {
        response->reading_phase = READING_COMPLETE;
        response->reading_error_code = READ_ERROR_SOCKET_CLOSED;
        return READ_ERROR_SOCKET_CLOSED;
    }

    // Update the buffer size (data in the buffer)
    response->size += bytes_read;
    return READ_ERROR_NONE;
}

// Parses headers to ensure the correct format, and to find Content-Length or Transfer-Encoding: chunked
// This updates the response struct with the expected body size and if the body is chunked
// It records that trailers are expected if the body is chunked, but we do not handle trailer headers specifically
// Returns true if the headers were parsed successfully
// If the headers are invalid it sets the reading_phase and response_error_code and returns false
bool process_headers(HTTPResponse *response) {
    size_t i;
    char *line = HEADERS(response); // Start of the headers
    char *next_line;
    char header_name[MAX_HEADER_NAME]; // We assume header names are less than 128 characters - and will ignore longer names
    char header_value[MAX_HEADER_VALUE]; // We assume header values are less than 128 characters - and will ignore longer values

    while (line && strlen(line)) {
        next_line = strstr(line, "\r\n");
        if (next_line) {
            next_line += 2;
        }
        else {
            next_line = NULL;
        }

        // Get the header name and value
        if (parse_header_line(line, header_name, header_value)) {
            // Check for Content-Length
            if (strcmp(header_name, "Content-Length") == 0) {
                // Validate that the value is a number
                for (i = 0; i < strlen(header_value); i++) {
                    if (header_value[i] < '0' || header_value[i] > '9') {
                        return false;
                    }
                }
                response->expected_body_size = atoi(header_value); // NOLINT(*-err34-c) - atoi is used here as the value has been validated
            }
            // Check for Transfer-Encoding: chunked
            else if (strcmp(header_name, "Transfer-Encoding") == 0 && strcmp(header_value, "chunked") == 0) {
                response->chunked = true;
                response->expecting_trailers = true;
            }

            // Check for trailer header
            else if (strcmp(header_name, "Trailer") == 0) {
                // We are not handling trailers specifically - but we could check for specific headers here
                response->expecting_trailers = true;
            }

        } else {
            response->reading_phase = READING_COMPLETE;
            response->reading_error_code = READ_ERROR_MALFORMED_HEADER;
            return false;
        }

        line = next_line;
    }

    return true;
}

// Function to check if the body is complete
// If chunked it reads the chunked response body buffer and determine if the response is complete
// Otherwise it checks if the body buffer is complete
// If it is complete it also splits the buffer into the trailer buffer
// Returns true if the response is complete
bool is_body_complete(HTTPResponse *response) {
    if (response->chunked) {
        // We have processed all the chunks and have not found the end of the chunks
        // We need to find the end of the chunks
        size_t i;
        size_t next_chunk_start = response->last_chunk_start;
        char *next_chunk_size = BODY(response) + response->last_chunk_start;
        while (next_chunk_start < response->body_length) {
            // Find the next chunk size
            char *crlf = strstr(next_chunk_size, "\r\n");
            if (crlf) {
                // Get the chunk size
                // Check we have a number
                for (i = 0; i < crlf - next_chunk_size; i++) {
                    if ((next_chunk_size[i] < '0' || next_chunk_size[i] > '9')
                        && (next_chunk_size[i] < 'a' || next_chunk_size[i] > 'f')
                        && (next_chunk_size[i] < 'A' || next_chunk_size[i] > 'F')) {
                        response->reading_error_code = READ_ERROR_MALFORMED_CHUNK;
                        return false;
                    }
                }
                // Get the chunk size from the hex string
                size_t chunk_size = strtol(next_chunk_size, NULL, 16);
                if (chunk_size == 0) {
                    // We have reached the end of the chunks
                    // Null terminate the buffer
                    *(BODY(response) + next_chunk_start) = '\0';
                    response->body_length = next_chunk_start;

                    // Set the trailer buffer start
                    response->trailer_start = crlf + 2 - response->buffer;
                    return true;
                }

                // Check we have the full chunk size
                if (crlf + 2 + chunk_size > response->buffer + response->size) {
                    // No so we just return false - the caller will read more data
                    return false;
                }

                // Move the chunk and everything in the buffer after it back overwriting the chunk size field using the chunk_size_field_size
                memmove(BODY(response) + next_chunk_start, crlf + 2, response->size - (crlf + 2 - response->buffer));
                response->size -=
                        crlf - (BODY(response) + next_chunk_start) + 2; // 2 for the crlf and 2 for the chunk size field

                // Set the next chunk start
                next_chunk_start += chunk_size;
                next_chunk_size = BODY(response) + next_chunk_start + 2; // Skip the crlf at the end of the chunk
                response->last_chunk_start = next_chunk_start;
            } else {
                // No crlf so we just return false - the caller will read more data
                return false;
            }
        }
        // We have processed all the chunks and have not found the end of the chunks - so we return false
        return false;
    }
    else {
        // Non-Chunked logic
        if (response->expected_body_size > 0) {
            // Have an expected body size - so we check if we have read enough data
            if (response->size < response->expected_body_size + 2 ) {
                // No so we just return false - the caller will read more data
                return false;
            }
            // We have the expected body date
            if (response->buffer[response->body_start + response->expected_body_size] == '\r' &&
                response->buffer[response->body_start + response->expected_body_size + 1] == '\n') {
                // We have a well-formed body
                // Null terminate the buffer
                response->buffer[response->body_start + response->expected_body_size] = '\0';
                response->body_length = response->expected_body_size;

                // Set the trailer buffer start
                response->trailer_start = response->body_start + response->body_length + 2;
                return true;
            }
            else {
                // No end newline so a malformed body
                response->reading_error_code = READ_ERROR_MALFORMED_BODY;
                return false;
            }
        }
        else {
            // We have no expected body size - so we check if there is an end newline
            // If there is we remove it, if not it is an error
            if (response->size >= 2 && response->buffer[response->size - 2] == '\r' && response->buffer[response->size - 1] == '\n') {
                // We have a complete body
                // Null terminate the buffer
                response->buffer[response->size - 2] = '\0';
                response->body_length = response->size - 2;

                // Set the trailer buffer start
                response->trailer_start = response->size;
                return true;
            }
            else {
                // No end newline so we just return false - the caller will read more data
                response->reading_error_code = READ_ERROR_MALFORMED_BODY;
                return false;
            }
        }
    }
}

// Function to parse and clean the body buffer
// This is mostly nop as is_body_complete() should have already cleaned the body buffer
// It does set if a TRAILER is expected by setting reading_phase to READING_TRAILERS or READING_COMPLETE
// returns true
bool process_body(HTTPResponse *response) {
    // Check that the body is null terminated (otherwise something has gone wrong)
    if (response->buffer[response->body_start + response->body_length] != '\0') {
        response->reading_error_code = READ_ERROR_MALFORMED_BODY;
        response->reading_phase = READING_COMPLETE;
        return false;
    }
    if (response->chunked) {
        response->reading_phase = READING_TRAILERS; // There may be trailers
    }
    else {
        response->reading_phase = READING_COMPLETE; // Not chunked so no trailers
    }
    return true;
}

// Parses trailers to find specific headers (to be defined)
// Returns true if the trailers were parsed successfully
bool process_trailers(HTTPResponse *response) {
    char *line = TRAILERS(response); // Start of the headers
    char *next_line;
    char header_name[MAX_HEADER_NAME]; // We assume trailer names are less than 128 characters - and will ignore longer names
    char header_value[MAX_HEADER_VALUE]; // We assume trailer values are less than 128 characters - and will ignore longer values

    while (line && strlen(line)) {
        next_line = strstr(line, "\r\n");
        if (next_line) {
            next_line += 2;
        }
        else {
            next_line = NULL;
        }

        // Get the trailer name and value - parse_header_line() is used to get the name and value
        if (parse_header_line(line, header_name, header_value)) {
            // This is where we can check for specific trailer headers e.g.  if (strcmp(header_name, "XXX") == 0) { ... }
        } else {
            response->reading_phase = READING_COMPLETE;
            response->reading_error_code = READ_ERROR_MALFORMED_TRAILER;
            return false;
        }

        line = next_line;
    }

    return true;
}

// Frees the memory used by the response
void free_response(HTTPResponse *response) {
    if (response->buffer) {
        free(response->buffer);
    }
    // Zero the whole struct
    memset(response, 0, sizeof(HTTPResponse));
}

// Strip a string of leading and trailing whitespace characters in place
void strip_whitespace(char *str) {
    char *start = str;
    char *end = str + strlen(str) - 1;

    // Strip leading whitespace
    while (*start && (*start == ' ' || *start == '\t')) start++;
    // Strip trailing whitespace
    while (end > start && (*end == ' ' || *end == '\t')) end--;
    // Null terminate the string
    *(end + 1) = '\0';
    // Move the string to the start
    if (start != str) {
        memmove(str, start, end - start + 2);
    }
}

// Parse a header line to get the header name and value
// Max length of header name and value is MAX_HEADER_NAME / MAX_HEADER_VALUE characters.
// Characters beyond this length are ignored (but assumed correctly formatted)
// The line is terminated by a newline character
// Returns true if the header was parsed successfully
bool parse_header_line(char *line, char *header_name, char *header_value) {
    char *colon = strchr(line, ':');
    if (colon) {
        // Copy the header name
        size_t name_length = colon - line;
        if (name_length > MAX_HEADER_NAME) name_length = MAX_HEADER_NAME;
        strncpy(header_name, line, name_length);
        header_name[name_length] = '\0';
        strip_whitespace(header_name);

        // Copy the header value - ending is a newline
        char *crlf = strstr(colon, "\r\n");
        if (crlf) {
            size_t value_length = crlf - colon - 1;
            if (value_length > MAX_HEADER_VALUE) value_length = MAX_HEADER_VALUE;
            strncpy(header_value, colon + 1, value_length);
            header_value[value_length] = '\0';
            strip_whitespace(header_value);
            return true;
        }
    }
    return false;
}
