//
// Test file httpcommon.c
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
    HTTPResponse buffer;
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

// Test is_body_complete
void test_is_body_complete() {
    HTTPResponse response;
    char *buffer = "Hello, World!\r\n";
    bool result;

    // Smoke test
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Set the header buffer input
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 0;
 //   response.body_length = strlen(buffer);
    response.chunked = false;
    response.expected_body_size = strlen(buffer) - 2;
    // Test
    result = is_body_complete(&response);
    // Check the result
    assert(result); // Should return true

    // Check the body buffer size and content
    assert(response.body_length == strlen(buffer) - 2);
    assert(memcmp(BODY(&response), buffer, strlen(buffer) - 2) == 0);
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
    ensure_buffer_size(&response, strlen(header_buffer) + 1);
    response.size = strlen(header_buffer);
    strcpy(response.buffer, header_buffer);
    response.header_length = strlen(header_buffer);
    response.header_start = 0;
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
    ensure_buffer_size(&response, strlen(header_buffer) + 1);
    response.size = strlen(header_buffer);
    strcpy(response.buffer, header_buffer);
    response.header_length = strlen(header_buffer);
    response.header_start = 0;
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
    ensure_buffer_size(&response, strlen(header_buffer) + 1);
    response.size = strlen(header_buffer);
    strcpy(response.buffer, header_buffer);
    response.header_length = strlen(header_buffer);
    response.header_start = 0;
    // process the headers
    result = process_headers(&response);
    // Check the result
    assert(!result); // Should return false
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
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    assert(memcmp(HEADERS(&response), "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);
}

int main() {
    test_ensure_buffer_size();
    test_is_body_complete();
    test_process_headers();
    test_read_response();
    return 0;
}