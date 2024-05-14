//
// Test httpcommon.c
//
// Created by Adrian Sutherland on 11/05/2024.
//
#include "httpcommon.h"
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

char mock_buffer[1024];
int mock_index = 0;

ssize_t read(__attribute__((unused)) int fd, void *buf, size_t count) {
    int i = 0;
    while (i < count && mock_buffer[mock_index] != '\0') {
        ((char*)buf)[i] = mock_buffer[mock_index];
        i++;
        mock_index++;
    }
    return i;
}

// Test ensure_buffer_size
void test_ensure_buffer_size() {
    DynamicBuffer buffer;
    buffer.buffer = NULL;
    buffer.size = 0;
    buffer.capacity = 0;
    ensure_buffer_size(&buffer, 10);
    assert(buffer.capacity >= 10);
    assert(buffer.size == 0);
    ensure_buffer_size(&buffer, 20);
    assert(buffer.capacity >= 20);
    assert(buffer.size == 0);
    ensure_buffer_size(&buffer, 5);
    assert(buffer.capacity >= 20);
    assert(buffer.size == 0);
    free(buffer.buffer);
}

// Test split_header_buffer
void test_split_header_buffer() {
    HTTPResponse response;
    char *header_buffer = "Content-Length: 13\r\n\r\nHello, World!\r\n";
    bool result;

    // Smoke test
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Set the header buffer input
    ensure_buffer_size(&response.header_buffer, strlen(header_buffer) + 1);
    response.header_buffer.size = strlen(header_buffer);
    strcpy(response.header_buffer.buffer, header_buffer);
    // Split the header buffer
    result = split_header_buffer(&response);
    // Check the result
    assert(result); // Should return true
    // Check the header buffer size and content
    assert(response.header_buffer.size == strlen("Content-Length: 13\r\n"));
    assert(memcmp(response.header_buffer.buffer, "Content-Length: 13\r\n", 19) == 0);
    // Check the body buffer size and content
    assert(response.body_buffer.size == strlen("Hello, World!\r\n"));
    assert(memcmp(response.body_buffer.buffer, "Hello, World!\r\n", 15) == 0);
    // Free response buffers
    free_response(&response);
}

// Test process_headers
void test_process_headers() {
    HTTPResponse response;
    char *header_buffer;
    bool result;

    // Smoke test
    header_buffer = "Content-Length: 13\r\n";
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Set the header buffer input
    ensure_buffer_size(&response.header_buffer, strlen(header_buffer) + 1);
    response.header_buffer.size = strlen(header_buffer);
    strcpy(response.header_buffer.buffer, header_buffer);
    // process the headers
    result = process_headers(&response);
    // Check the result
    assert(result); // Should return true
    // Check the expected body size
    assert(response.expected_body_size == 13);
    // Check if the body is chunked
    assert(!response.chunked);
    // Free response buffers
    free_response(&response);

    // Test with chunked
    header_buffer = "Content-Length: 20\r\nTransfer-Encoding: chunked\r\n";
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Set the header buffer input
    ensure_buffer_size(&response.header_buffer, strlen(header_buffer) + 1);
    response.header_buffer.size = strlen(header_buffer);
    strcpy(response.header_buffer.buffer, header_buffer);
    // process the headers
    result = process_headers(&response);
    // Check the result
    assert(result); // Should return true
    // Check the expected body size
    assert(response.expected_body_size == 20);
    // Check if the body is chunked
    assert(response.chunked);
    // Free response buffers
    free_response(&response);

    // Test with error
    header_buffer = "Content-Length: 20\r\nTransfer-Encoding chunked\r\n"; // No second colon
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Set the header buffer input
    ensure_buffer_size(&response.header_buffer, strlen(header_buffer) + 1);
    response.header_buffer.size = strlen(header_buffer);
    strcpy(response.header_buffer.buffer, header_buffer);
    // process the headers
    result = process_headers(&response);
    // Check the result
    assert(!result); // Should return false
    // Free response buffers
    free_response(&response);
}

// Function to test clean_body_buffer() which parses and cleans the body buffer.
// If not chunked remove the end newline. If chunked remove the chunked encoding.
// It returns true if the body was cleaned successfully
void test_clean_body_buffer() {
    HTTPResponse response;
    char *body_buffer;
    bool result;
    char *expected_result;

    // Smoke test
    body_buffer = "Hello, World!\r\n";
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Set the body buffer input
    ensure_buffer_size(&response.body_buffer, strlen(body_buffer) + 1);
    response.body_buffer.size = strlen(body_buffer);
    strcpy(response.body_buffer.buffer, body_buffer);
    // Clean the body buffer
    result = clean_body_buffer(&response);
    // Check the result
    assert(result); // Should return true
    // Check the body buffer size and content
    expected_result = "Hello, World!";
    assert(response.body_buffer.size == strlen(expected_result));
    assert(memcmp(response.body_buffer.buffer, expected_result, strlen(expected_result)) == 0);
    // Free response buffers
    free_response(&response);

    // Test with chunked
    body_buffer = "7\r\nHello, \r\n6\r\nWorld!\r\n0\r\n";
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Set the body buffer input
    ensure_buffer_size(&response.body_buffer, strlen(body_buffer) + 1);
    response.body_buffer.size = strlen(body_buffer);
    strcpy(response.body_buffer.buffer, body_buffer);
    response.chunked = true;
    response.last_chunk_start = response.body_buffer.buffer;
    // Clean the body buffer
    result = clean_body_buffer(&response);
    // Check the result
    assert(result); // Should return true
    // Check the body buffer size and content
    assert(response.body_buffer.size == strlen("Hello, World!"));
    assert(memcmp(response.body_buffer.buffer, "Hello, World!", 13) == 0);
    // Free response buffers
    free_response(&response);
}

// Function test is_chunking_complete()
// This reads a chunked response body buffer and determine if the response is complete
// If it is complete it also splits the buffer into the trailer buffer
// Returns true if the response is complete
void test_is_chunking_complete() {
    HTTPResponse response;
    char *body_buffer;
    bool result;
    char *expected_result;

    // Smoke test
    body_buffer = "7\r\nHello, \r\n6\r\nWorld!\r\n0\r\n"
                  "\r\n"
                  "Content-MD5: e59ff97941044f85df5297e1c302d260\r\n"
                  "\r\n";

    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Set the body buffer input
    ensure_buffer_size(&response.body_buffer, strlen(body_buffer) + 1);
    response.body_buffer.size = strlen(body_buffer);
    strcpy(response.body_buffer.buffer, body_buffer);
    response.chunked = true;
    response.last_chunk_start = response.body_buffer.buffer;
    // Check if the response is complete
    result = is_chunking_complete(&response);
    // Check the result
    assert(result); // Should return true
    // Check the body buffer size and content
    expected_result = "Hello, World!";
    assert(response.body_buffer.size == strlen(expected_result));
    assert(memcmp(response.body_buffer.buffer, expected_result, strlen(expected_result)) == 0);
    // Check the trailer buffer size and content
    expected_result = "Content-MD5: e59ff97941044f85df5297e1c302d260\r\n";
    assert(response.trailer_buffer.size == strlen(expected_result));
    assert(memcmp(response.trailer_buffer.buffer, expected_result, strlen(expected_result)) == 0);
    // Free response buffers
    free_response(&response);

}


// Test read_response
void test_read_response() {
    HTTPResponse response;
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));

    // Smoke test
    strcpy(mock_buffer, "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!\r\n");
    mock_index = 0;
    bool result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(response.status_line, "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_buffer.size == 13);
    assert(memcmp(response.body_buffer.buffer, "Hello, World!", 13) == 0);
    assert(memcmp(response.header_buffer.buffer, "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);
}

int main() {
    test_ensure_buffer_size();
    test_split_header_buffer();
    test_process_headers();
    test_read_response();
    test_clean_body_buffer();
    test_is_chunking_complete();
    return 0;
}