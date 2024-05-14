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
    ssize_t bytes_read;
    size_t status_line_size;


    // Initialize the response
    response->reading_error_code = 0;
    response->reading_phase = READING_STATUS_LINE;
    response->split_buffer = false;
    response->status_code = 0;
    response->status_line[0] = '\0';
    response->chunked = false;
    response->last_chunk_start = response->body_buffer.buffer;
    response->expected_body_size = 0;
    // Clean the buffers - however we might be reusing the malloced buffer
    response->header_buffer.size = 0;
    response->body_buffer.size = 0;
    response->trailer_buffer.size = 0;

    // Read the response
    while (true) {
        switch (response->reading_phase) {

            case READING_STATUS_LINE:
                // We might be appending to the status line buffer - rare but possible
                status_line_size = strlen(response->status_line);
                if (status_line_size >= MAX_STATUS_LINE_SIZE) {
                    response->reading_phase = READING_COMPLETE;
                    response->reading_error_code = -3; // Too long status line
                    return false;
                }
                // Read the status line from the socket
                bytes_read = read(sock, response->status_line + status_line_size, MAX_STATUS_LINE_SIZE - status_line_size);
                if (bytes_read <= 0) {
                    response->reading_phase = READING_COMPLETE;
                    response->reading_error_code = -1; // Error reading status line
                    return false;
                }

                // Check if we have a full status line
                response->status_line[status_line_size + bytes_read] = '\0';
                char *crlf = strstr(response->status_line, "\r\n");
                if (crlf) {
                    *crlf = '\0';
                    // Fine the second word in the status line (the status code)
                    char *space = strchr(response->status_line, ' ');
                    if (space) {
                        // Get the status code
                        response->status_code = (int)strtol(space, NULL, 10);
                        // Check if the status code is valid
                        if (response->status_code < 100 || response->status_code > 599) {
                            response->reading_phase = READING_COMPLETE;
                            response->reading_error_code = -4; // Invalid status code
                            return false;
                        }
                        response->reading_phase = READING_HEADERS;
                    }
                    else {
                        response->reading_phase = READING_COMPLETE;
                        response->reading_error_code = -2; // Malformed status line
                        return false;
                    }

                    // Copy the rest of the status line to the header buffer
                    // Is there any data after the status line?
                    size_t size_after_status_line = response->status_line + bytes_read - crlf - 2;
                    if (size_after_status_line > 0) {
                        ensure_buffer_size(&response->header_buffer, size_after_status_line);
                        memccpy(response->header_buffer.buffer, crlf + 2, 0, size_after_status_line);
                        response->header_buffer.size = size_after_status_line;
                        response->split_buffer = true;
                    }

                    // We are now ready to read the headers
                    response->reading_phase = READING_HEADERS;
                }
                else break; // Loop to get the rest of the status line

                // Note no break here - we continue to the next phase

            case READING_HEADERS:
                // Check if we have a split buffer state
                if (response->split_buffer == false || response->header_buffer.size == 0) {
                    // Read the headers from the socket
                    // Ensure we have at least INITIAL_BUFFER_SIZE bytes spare in the buffer for the read
                    if (response->header_buffer.capacity - response->header_buffer.size < INITIAL_BUFFER_SIZE) {
                        ensure_buffer_size(&response->header_buffer, response->header_buffer.capacity + INITIAL_BUFFER_SIZE);
                    }
                    // Read from the socket
                    bytes_read = read(sock, response->header_buffer.buffer + response->header_buffer.size, response->header_buffer.capacity - response->header_buffer.size);
                    if (bytes_read < 0) {
                        response->reading_phase = READING_COMPLETE;
                        response->reading_error_code = -5; // Error reading headers
                        return false;
                    }
                    response->header_buffer.size += bytes_read;
                    if (bytes_read == 0) {
                        response->reading_phase = READING_COMPLETE;
                        // This may not be an error condition - we may have reached the end of the response
                        if (process_headers(response)) {
                            if (response->expected_body_size == 0 && response->chunked == false) {
                                response->reading_phase = READING_COMPLETE;
                                return true;
                            }
                            else {
                                // Error missing body
                                response->reading_error_code = -8; // Error missing body
                                return false;
                            }
                        }
                        else {
                            response->reading_error_code = -5; // Error reading headers
                            return false;
                        }
                    }
                }
                // In any case we won't have a split buffer state now
                response->split_buffer = false;

                // Check if we have a double newline
                if (split_header_buffer(response)) {
                    // We have a double newline - we are ready to read the body
                    response->reading_phase = READING_BODY;
                    response->split_buffer = true; // We have split the buffer, so we need to process the data we put in the body buffer
                    // Process the headers to find the expected body size
                    if (!process_headers(response)) {
                        response->reading_phase = READING_COMPLETE;
                        response->reading_error_code = -6; // Error parsing headers
                        return false;
                    }

                }
                // If we don't have a double newline we need to read more headers
                else break;

                // Note no break here - we continue to the next phase

            case READING_BODY:
                // Check if we have a split buffer state
                if (response->split_buffer == false || response->body_buffer.size == 0) {
                    // Read the body from the socket
                    // Ensure we have at least INITIAL_BUFFER_SIZE bytes spare in the buffer for the read
                    if (response->body_buffer.capacity - response->body_buffer.size < INITIAL_BUFFER_SIZE) {
                        ensure_buffer_size(&response->body_buffer, response->body_buffer.capacity + INITIAL_BUFFER_SIZE);
                    }
                    // Read from the socket
                    bytes_read = read(sock, response->body_buffer.buffer + response->body_buffer.size, response->body_buffer.capacity - response->body_buffer.size);
                    if (bytes_read < 0) {
                        response->reading_phase = READING_COMPLETE;
                        response->reading_error_code = -7; // Error reading body
                        return false;
                    }
                    response->body_buffer.size += bytes_read;
                    if (bytes_read == 0) {
                        response->reading_phase = READING_COMPLETE;
                        return clean_body_buffer(response);
                    }
                }
                // In any case we won't have a split buffer state now
                response->split_buffer = false;

                // Check if we have read the expected body size
                if (response->chunked) {
                    if (is_chunking_complete(response)) {
                        response->reading_phase = READING_TRAILERS;
                    }
                    else {
                        // We need to read more body
                        break;
                    }
                }
                else {
                    // If we are expecting more body continue to read
                    if (response->expected_body_size > response->body_buffer.size)
                        break;
                    else {
                        // We are not chunking, so we may have all the body (or it may be an error)
                        // So let the clean_body_buffer function decide
                        response->reading_phase = READING_COMPLETE;
                        return clean_body_buffer(response);
                    }
                }

                // Note no break here - we continue to the next phase
                // We were processing chunks, and we have read the last chunk snd may have split the buffer to the trailer buffer

            case READING_TRAILERS:
                // Check if we have a split buffer state
                if (response->split_buffer == false || response->trailer_buffer.size == 0) {
                    // Read the trailers from the socket
                    // Ensure we have at least INITIAL_BUFFER_SIZE bytes spare in the buffer for the read
                    if (response->trailer_buffer.capacity - response->trailer_buffer.size < INITIAL_BUFFER_SIZE) {
                        ensure_buffer_size(&response->trailer_buffer, response->trailer_buffer.capacity + INITIAL_BUFFER_SIZE);
                    }
                    // Read from the socket
                    bytes_read = read(sock, response->trailer_buffer.buffer + response->trailer_buffer.size, response->trailer_buffer.capacity - response->trailer_buffer.size);
                    if (bytes_read < 0) {
                        response->reading_phase = READING_COMPLETE;
                        response->reading_error_code = -9; // Error reading trailers
                        return false;
                    }
                    response->trailer_buffer.size += bytes_read;
                    if (bytes_read == 0) {
                        response->reading_phase = READING_COMPLETE;
                        return true;
                    }
                }
                // In any case we won't have a split buffer state now
                response->split_buffer = false;

                // Determine if we are at the end of the trailers - is there a double newline?
                // Search the buffer
                crlf = strstr(response->trailer_buffer.buffer, "\r\n\r\n");
                if (crlf) {
                    // We have read the trailers
                    // Check that the last newline sequence is the end of the buffer
                    if (crlf + 4 < response->trailer_buffer.buffer + response->trailer_buffer.size) {
                        response->reading_phase = READING_COMPLETE;
                        response->reading_error_code = -10; // Error parsing trailers
                        return false;
                    }

                    // Null terminate the trailers
                    *(crlf + 2) = '\0'; // Keep the first newline as part of the trailers
                    response->trailer_buffer.size = crlf - response->trailer_buffer.buffer + 2;
                    response->reading_phase = READING_COMPLETE;
                    return true;
                }
                else break; // We need to read more trailers

            case READING_COMPLETE:
                // We should never get here
                return false;
        }
    }
}

// Makes sure the buffer is at least of  size (doubling the buffer size if need be)
void ensure_buffer_size(DynamicBuffer *buffer, size_t size){
    char *new_buffer;
    if (size < INITIAL_BUFFER_SIZE) size = INITIAL_BUFFER_SIZE;
    if (buffer->capacity < size) {
        size_t new_capacity = buffer->capacity * 2;
        if (new_capacity < size) new_capacity = size;

        // Use realloc to resize the buffer
        if (buffer->buffer == NULL) {
            new_buffer = malloc(new_capacity);
        }
        else {
            new_buffer = realloc(buffer->buffer, new_capacity);
        }
        if (new_buffer) {
            buffer->buffer = new_buffer;
            buffer->capacity = new_capacity;
        }
        else {
            // realloc failed - we just panic and exit on memory allocation failures
            // Print an error message to stderr
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
}

// Checks for \r\n\r\n in the header buffer and if so returns true and splits the buffer to body buffer
bool split_header_buffer(HTTPResponse *response) {
    char *crlf = strstr(response->header_buffer.buffer, "\r\n\r\n");
    if (crlf) {
        // We have a double newline
        // Calculate the string length to be copied to the body buffer
        size_t body_size = response->header_buffer.size - (crlf - response->header_buffer.buffer) - 4;

        // Copy the header buffer to the body buffer
        ensure_buffer_size(&response->body_buffer, body_size);
        memccpy(response->body_buffer.buffer, crlf + 4, 0, body_size);
        // Set the body buffer size
        response->body_buffer.size = body_size;

        // Null terminate the header buffer
        *(crlf + 2) = '\0'; // Keep the first newline as part of the headers
        response->header_buffer.size = crlf - response->header_buffer.buffer + 2;

        return true;
    }
    return false;
}

// Frees the memory used by the buffer
void free_buffer(DynamicBuffer *buffer) {
    if (buffer->buffer) {
        free(buffer->buffer);
        buffer->buffer = NULL;
        buffer->size = 0;
        buffer->capacity = 0;
    }
}

// Frees the memory used by the response
void free_response(HTTPResponse *response) {
    free_buffer(&response->header_buffer);
    free_buffer(&response->body_buffer);
    free_buffer(&response->trailer_buffer);
    response->status_code = 0;
    response->status_line[0] = '\0';
    response->chunked = false;
    response->last_chunk_start = response->body_buffer.buffer;
    response->expected_body_size = 0;
    response->reading_phase = READING_STATUS_LINE;
    response->split_buffer = false;
    response->reading_error_code = 0;
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

// Parses headers to find Content-Length or Transfer-Encoding: chunked
// This updates the response struct with the expected body size and if the body is chunked
// Returns true if the headers were parsed successfully
// Note that the header buffer is not modified by this function
bool process_headers(HTTPResponse *response) {
    size_t i;
    char *line = response->header_buffer.buffer;
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
            }
        } else
            return false;

        line = next_line;
    }

    return true;
}

// Function to parse and clean the body buffer (if not chunked remove the end newline, if chunked remove the chunked encoding)
// returns true if the body was cleaned successfully
bool clean_body_buffer(HTTPResponse *response) {
    size_t i;
    if (response->chunked) {
        // The characters before the response->last_chunk_start have been processed already
        // We loop round processing chunks until we have a completed de-chunked buffer
        char *next_chunk_start = response->last_chunk_start;
        char *next_chunk_size = response->last_chunk_start;
        while (next_chunk_start < response->body_buffer.buffer + response->body_buffer.size) {
            // Find the next chunk size
            char *crlf = strstr(next_chunk_size, "\r\n");
            if (crlf) {
                // Get the chunk size
                // Check we have a number
                for (i = 0; i < crlf - next_chunk_size; i++) {
                    if (    (next_chunk_size[i] < '0' || next_chunk_size[i] > '9')
                         && (next_chunk_size[i] < 'a' || next_chunk_size[i] > 'f')
                         && (next_chunk_size[i] < 'A' || next_chunk_size[i] > 'F') ) {
                        response->reading_error_code = -11; // Error parsing chunk size
                        return false;
                    }
                }
                // Get the chunk size from the hex string
                size_t chunk_size = strtol(next_chunk_size, NULL, 16);
                if (chunk_size == 0) {
                    // We have reached the end of the chunks
                    // Null terminate the buffer
                    *next_chunk_start = '\0';
                    response->body_buffer.size = next_chunk_start - response->body_buffer.buffer;
                    return true;
                }

                // Check we have the full chunk size
                if (crlf + 2 + chunk_size > response->body_buffer.buffer + response->body_buffer.size) {
                    // Error the chunk is incomplete
                    response->reading_error_code = -12; // Error incomplete chunk
                    return false;
                }

                // Move the chunk and everything in the buffer after it back overwriting the chunk size field using the chunk_size_field_size
                memmove(next_chunk_start, crlf + 2, response->body_buffer.size - (crlf + 2 - response->body_buffer.buffer));
                response->body_buffer.size -= crlf - next_chunk_start + 2;

                // Set the next chunk start
                next_chunk_start += chunk_size;
                next_chunk_size = next_chunk_start + 2; // Skip the crlf at the end of the chunk
                response->last_chunk_start = next_chunk_start;
            }
            else {
                // Error missing chunk
                response->reading_error_code = -13; // Error missing chunk
                return false;
            }
        }
        // We have processed all the chunks and have not found the end of the chunks
        response->reading_error_code = -14; // Error missing end of chunks
        return false;
    }
    else {
        // Non-Chunked logic */
        if (response->expected_body_size > 0) {
            // We have an expected body size - so we check if there is an end newline
            /* Remove the end newline - which is after the last body character bases on the expected body size */
            if (response->body_buffer.size > 1 &&
                response->body_buffer.buffer[response->body_buffer.size - 2] == '\r' &&
                response->body_buffer.buffer[response->body_buffer.size - 1] == '\n') {
                response->body_buffer.size -= 2;
                response->body_buffer.buffer[response->body_buffer.size] = '\0';
            }

            if (response->body_buffer.size > response->expected_body_size) {
                response->reading_error_code = -15; // Error body size mismatch
                return false;
            }
        }

        else {
            // We have no expected body size - so we check if there is an end newline
            // If there is we remove it, if not it is an error
            if (response->body_buffer.size > 1 && response->body_buffer.buffer[response->body_buffer.size - 2] == '\r' && response->body_buffer.buffer[response->body_buffer.size - 1] == '\n') {
                response->body_buffer.size -= 2;
                response->body_buffer.buffer[response->body_buffer.size] = '\0';
            }
            else {
                response->reading_error_code = -16; // Error missing end newline
                return false;
            }
        }
        return true;
    }
}

// Function to read a chunked response body buffer and determine if the response is complete
// If it is complete it also splits the buffer into the trailer buffer
// Returns true if the response is complete
bool is_chunking_complete(HTTPResponse *response) {
    // We have processed all the chunks and have not found the end of the chunks
    // We need to find the end of the chunks
    size_t i;
    char *next_chunk_start = response->last_chunk_start;
    char *next_chunk_size = response->last_chunk_start;
    while (next_chunk_start < response->body_buffer.buffer + response->body_buffer.size) {
        // Find the next chunk size
        char *crlf = strstr(next_chunk_size, "\r\n");
        if (crlf) {
            // Get the chunk size
            // Check we have a number
            for (i = 0; i < crlf - next_chunk_size; i++) {
                if ((next_chunk_size[i] < '0' || next_chunk_size[i] > '9')
                    && (next_chunk_size[i] < 'a' || next_chunk_size[i] > 'f')
                    && (next_chunk_size[i] < 'A' || next_chunk_size[i] > 'F')) {
                    response->reading_error_code = -17; // Error parsing chunk size
                    return false;
                }
            }
            // Get the chunk size from the hex string
            size_t chunk_size = strtol(next_chunk_size, NULL, 16);
            if (chunk_size == 0) {
                // We have reached the end of the chunks
                // Null terminate the buffer
                *next_chunk_start = '\0';
                // Calculate the remaining data size to go into the trailer buffer
                size_t remaining_data_size = response->body_buffer.size - (next_chunk_start - response->body_buffer.buffer);

                response->body_buffer.size = next_chunk_start - response->body_buffer.buffer;
                // Split the buffer to the trailer buffer
                ensure_buffer_size(&response->trailer_buffer, response->body_buffer.capacity); // TO DO Capacity
                strcpy(response->trailer_buffer.buffer, crlf + 4);
                response->trailer_buffer.size = remaining_data_size;
                response->split_buffer = true;
                return true;
            }

            // Check we have the full chunk size
            if (crlf + 2 + chunk_size > response->body_buffer.buffer + response->body_buffer.size) {
                // No so we just return false - the caller will read more data
                return false;
            }

            // Move the chunk and everything in the buffer after it back overwriting the chunk size field using the chunk_size_field_size
            memmove(next_chunk_start, crlf + 2, response->body_buffer.size - (crlf + 2 - response->body_buffer.buffer));
            response->body_buffer.size -= crlf - next_chunk_start + 2; // 2 for the crlf and 2 for the chunk size field

            // Set the next chunk start
            next_chunk_start += chunk_size;
            next_chunk_size = next_chunk_start + 2; // Skip the crlf at the end of the chunk
            response->last_chunk_start = next_chunk_start;
        } else {
            // No crlf so we just return false - the caller will read more data
            return false;
        }
    }
    // We have processed all the chunks and have not found the end of the chunks - so we return false
    return false;
}
