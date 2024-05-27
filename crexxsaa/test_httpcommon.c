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
// #include <printf.h>

char mock_buffer[1024];
int mock_index = 0;

// This is a mock read function that reads from a buffer
// It is used to test the read_into_buffer() function
// It reads from the mock_buffer and returns the data read (upto the count, or null terminator)
// mock_index is used to keep track of the current position in the buffer (for the next read)
// The character ^ is used to simulate an error in the read
// The character @ is used to simulate \0 in the data stream (as a character not a terminator)
// The character ~ is used to simulate a break in the data stream (the read is incomplete, the next read will continue from here)
// An empty remaining string is used to simulate a closed socket
// If read() is called more than once, it will return the next chunk of data, and the caller
// If a read is called after the end of the buffer has been reached, it will return 0
ssize_t read(__attribute__((unused)) int fd, void *buf, size_t count) {
    int i = 0;
    while (i < count && mock_buffer[mock_index] != '\0') {
        if (mock_buffer[mock_index] == '^') {
            mock_index++;
            return -1;
        } else if (mock_buffer[mock_index] == '@') {
            ((char*)buf)[i] = '\0';
        } else if (mock_buffer[mock_index] == '~') {
            mock_index++;
            return i;
        }
        else {
            ((char *) buf)[i] = mock_buffer[mock_index];
        }
        i++;
        mock_index++;
    }
    return i;
}

// This sets the mock buffer and index for the read function
void set_buffer(char* buffer) {
    strcpy(mock_buffer, buffer);
    mock_index = 0;
}

// Tests read_into_buffer() which:
// Read data from the socket into the buffer
// Returns READ_ERROR_NONE on success, <0 (READ_ERROR_...) > on an error or if the socket is closed (setting reading_error_code and reading_phase)
void test_read_into_buffer() {
    HTTPResponse response;
    int result;
    size_t size;

    // Smoke test
    set_buffer("HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!\r\n");
    size = strlen(mock_buffer);
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Test
    result = read_into_buffer(0, &response);
    // Check the result
    assert(result == READ_ERROR_NONE);
    assert(response.reading_error_code == READ_ERROR_NONE);
    assert(response.size == size);
    assert(memcmp(response.buffer, "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!\r\n", size) == 0);
    // Free response buffers
    free_response(&response);

    // Test with two reads (to make sure buffer appending works)
    set_buffer("HTTP/1.1 200 OK\r\nConte~nt-Length: 13\r\n\r\nHello, World!\r\n");
    size = strlen(mock_buffer) - 1; // Remove the ~
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Test
    result = read_into_buffer(0, &response);
    // Check the result
    assert(result == READ_ERROR_NONE);
    // Read the second chunk
    result = read_into_buffer(0, &response);
    // Check the result
    assert(result == READ_ERROR_NONE);
    assert(response.reading_error_code == READ_ERROR_NONE);
    assert(response.size == size);
    assert(memcmp(response.buffer, "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!\r\n", size) == 0);
    // Free response buffers
    free_response(&response);

    // Test with a closed socket
    set_buffer("\0");
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Test
    result = read_into_buffer(0, &response);
    // Check the result
    assert(result == READ_ERROR_SOCKET_CLOSED);
    assert(response.reading_error_code == READ_ERROR_SOCKET_CLOSED);
    assert(response.reading_phase == READING_COMPLETE); // Should be complete (since the socket is closed)
    // Free response buffers
    free_response(&response);

    // Test with an socket error
    set_buffer("^");
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Test
    result = read_into_buffer(0, &response);
    // Check the result
    assert(result == READ_ERROR_SOCKET_ERROR);
    assert(response.reading_error_code == READ_ERROR_SOCKET_ERROR);
    assert(response.reading_phase == READING_COMPLETE);
    // Free response buffers
    free_response(&response);
}

// Tests ensure_buffer_size() which:
// Makes sure the response buffer is at least of  size (doubling the buffer size if need be)
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
    ensure_buffer_size(&buffer, 5000);
    assert(buffer.capacity >= 5000);
    assert(buffer.size == 0);

    free(buffer.buffer);
}

// Tests process_headers() which:
// Parses headers to find Content-Length or Transfer-Encoding: chunked
// This updates the response struct with the expected body size and if the body is chunked
// Returns true if the headers were parsed successfully
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

// Tests is_body_complete() which:
// Checks if the body is complete
// If chunked it reads the chunked response body buffer and determine if the response is complete
// Otherwise it checks if the body buffer is complete
// If it is complete it also splits the buffer into the trailer buffer
// Returns true if the response is complete
void test_is_body_complete() {
    HTTPResponse response;
    char *buffer;
    size_t size;
    char* expected;
    bool result;

    // Smoke test
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "Hello, World!\r\n";
    size = strlen(buffer) - 2;
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 0;
    response.chunked = false;
    response.expected_body_size = size;
    // Test
    result = is_body_complete(&response);
    // Check the result
    assert(result); // Should return true
    assert(response.body_length == size);
    assert(memcmp(BODY(&response), buffer, size) == 0);
    // Check Initial Trailer buffer
    assert(response.trailer_start == strlen(buffer));
    assert(response.trailer_length == 0);
    // Free response buffers
    free_response(&response);

    // Test non-chunked with some characters before the body start
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "XXXXHello, World!\r\n";
    expected = "Hello, World!";
    size = strlen(expected);
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 4;
    response.chunked = false;
    response.expected_body_size = size;
    // Test
    result = is_body_complete(&response);
    // Check the result
    assert(result); // Should return true
    assert(response.body_length == size);
    assert(memcmp(BODY(&response), expected, size) == 0);
    // Check Initial Trailer buffer
    assert(response.trailer_start == strlen(buffer));
    assert(response.trailer_length == 0);
    // Free response buffers
    free_response(&response);

    // Test Incomplete non-chunked header
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "Hello, World!";
    size = strlen(buffer);
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 0;
    response.chunked = false;
    response.expected_body_size = size + 10;
    // Test
    result = is_body_complete(&response);
    // Check the result
    assert(!result); // Should return false
    // Free response buffers
    free_response(&response);

    // Test complete chunked header
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "d\r\nHello, World!\r\n0\r\n\r\n";
    expected = "Hello, World!";
    size = strlen(expected);
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 0;
    response.chunked = true;
    response.expected_body_size = size;
    // Test
    result = is_body_complete(&response);
    // Check the result
    assert(result); // Should return true
    assert(response.body_length == size);
    assert(memcmp(BODY(&response), expected, size) == 0);
    // Check Initial Trailer buffer
    assert(response.trailer_start == 18); // The buffer should be "Hello, World!\0\r\n\r\n"
    assert(response.trailer_length == 0);
    // Free response buffers
    free_response(&response);

    // Test complete chunked header with some characters before the body start
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "XXXXd\r\nHello, World!\r\n0\r\n\r\n";
    expected = "Hello, World!";
    size = strlen(expected);
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 4;
    response.chunked = true;
    response.expected_body_size = size;
    // Test
    result = is_body_complete(&response);
    // Check the result
    assert(result); // Should return true
    assert(response.body_length == size);
    assert(memcmp(BODY(&response), expected, size) == 0);
    // Check Initial Trailer buffer
    assert(response.trailer_start == 22); // The buffer should be "XXXXHello, World!\0\r\n\r\n"
    assert(response.trailer_length == 0);
    // Free response buffers
    free_response(&response);

    // Test complete chunked header with 3 chunks and a trailer field
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n";
    expected = "Wikipedia in\r\nchunks.";
    size = strlen(expected);
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 0;
    response.chunked = true;
    response.expected_body_size = size;
    // Test
    result = is_body_complete(&response);
    // Check the result
    assert(result); // Should return true
    assert(response.body_length == size);
    assert(memcmp(BODY(&response), expected, size) == 0);
    // Check Initial Trailer buffer
    assert(memcmp(TRAILERS(&response), "Expires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n", 42) == 0);
    // Free response buffers
    free_response(&response);

    // Test Incomplete chunked header
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks";
    expected = "WikipediaC\r\n in\r\nchunks";
    size = strlen(expected);
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 0;
    response.chunked = true;
    response.expected_body_size = size;
    // Test
    result = is_body_complete(&response);
    // Check the result
    assert(!result); // Should return false
    assert(memcmp(BODY(&response), expected, size) == 0);
    assert(response.last_chunk_start == 9); // Should point to the C in "WikipediaC"
    // Free response buffers
    free_response(&response);

    // Test with an chunked error
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "3\r\nWiki\r\n5\r\npedia";
    expected = "WikipediaC\r\n in\r\nchunks";
    size = strlen(expected);
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 0;
    response.chunked = true;
    response.expected_body_size = size + 10;
    // Test
    result = is_body_complete(&response);
    // Check the result
    assert(!result); // Should return false
    // Free response buffers
    free_response(&response);

    // Test with a non-sized non-chunked body
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "Hello, World!\r\n";
    expected = "Hello, World!";
    size = strlen(expected);
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 0;
    response.chunked = false;
    response.expected_body_size = size;
    // Test
    result = is_body_complete(&response);
    // Check the result
    assert(result); // Should return true
    assert(response.body_length == size);
    assert(memcmp(BODY(&response), expected, size) == 0);
    // Check Initial Trailer buffer
    assert(response.trailer_start == size + 2); //  Should be the char after "Hello, World!\0\r\n"
    assert(response.trailer_length == 0);
    // Free response buffers
    free_response(&response);

    // Test with a non-sized non-chunked body error
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "Hello, World!\r_"; // Missing \n
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 0;
    response.chunked = false;
    response.expected_body_size = size;
    // Test
    result = is_body_complete(&response);
    // Check the result
    assert(!result); // Should return false
    // Free response buffers
    free_response(&response);

}

// Tests process_body() which:
// Function to parse and clean the body buffer
// returns true if the body was cleaned successfully
// Note this test assumed is_body_complete() works
void test_process_body(){
    HTTPResponse response;
    char *buffer;
    size_t size;
    char* expected;
    bool result;

    // Non-Chunked
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "Hello, World!\r\n";
    size = strlen(buffer) - 2;
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 0;
    response.chunked = false;
    response.expected_body_size = size;
    // Test
    is_body_complete(&response);
    result = process_body(&response);
    // Check the result
    assert(result); // Should return true
    assert(response.chunked == false);
    assert(response.reading_phase == READING_COMPLETE);
    // Free response buffers
    free_response(&response);

    // Not null terminated - internal error
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "Hello, World!\r\n";
    size = strlen(buffer) - 2;
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 0;
    response.chunked = false;
    response.expected_body_size = size;
    // Test
    is_body_complete(&response);
    response.buffer[size] = 'X'; // Corrupt the buffer
    result = process_body(&response);
    // Check the result
    assert(!result); // Should return false
    assert((response.reading_error_code == READ_ERROR_MALFORMED_BODY));
    // Free response buffers
    free_response(&response);

    // Test complete chunked header
    // Setup
    memset(&response, 0, sizeof(HTTPResponse));
    buffer = "d\r\nHello, World!\r\n0\r\n\r\n";
    expected = "Hello, World!";
    size = strlen(expected);
    ensure_buffer_size(&response, strlen(buffer) + 1);
    response.size = strlen(buffer);
    strcpy(response.buffer, buffer);
    response.header_length = 0;
    response.header_start = 0;
    response.body_start = 0;
    response.chunked = true;
    response.expected_body_size = size;
    // Test
    is_body_complete(&response);
    result = process_body(&response);
    // Check the result
    assert(result); // Should return true
    assert(response.chunked == true);
    assert(response.reading_phase == READING_TRAILERS);
    // Free response buffers
    free_response(&response);
}

// Tests process_trailers() which:
// Parses trailers to find specific headers (to be defined)
// Returns true if the trailers were parsed successfully
void test_process_trailers(){
    HTTPResponse response;
    char *trailer_buffer;
    bool result;

    // Smoke test
    trailer_buffer = "Expires: Wed, 21 Oct 2015 07:28:00 GMT\r\n";
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Set the header buffer input
    ensure_buffer_size(&response, strlen(trailer_buffer) + 1);
    response.size = strlen(trailer_buffer);
    strcpy(response.buffer, trailer_buffer);
    response.trailer_length = strlen(trailer_buffer);
    response.header_start = 0;
    // process the headers
    result = process_trailers(&response);
    // Check the result
    assert(result); // Should return true
    // Free response buffers
    free_response(&response);

    // Two Trailers test
    trailer_buffer = "Expires: Wed, 21 Oct 2015 07:28:00 GMT\r\nCache-Control: no-cache\r\n";
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Set the header buffer input
    ensure_buffer_size(&response, strlen(trailer_buffer) + 1);
    response.size = strlen(trailer_buffer);
    strcpy(response.buffer, trailer_buffer);
    response.trailer_length = strlen(trailer_buffer);
    response.header_start = 0;
    // process the headers
    result = process_trailers(&response);
    // Check the result
    assert(result); // Should return true
    // Free response buffers
    free_response(&response);

    // Invalid Trailers test
    trailer_buffer = "Expires: Wed, 21 Oct 2015 07:28:00 GMT\r\nCache-Control no-cache\r\n"; // No colon
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Set the header buffer input
    ensure_buffer_size(&response, strlen(trailer_buffer) + 1);
    response.size = strlen(trailer_buffer);
    strcpy(response.buffer, trailer_buffer);
    response.trailer_length = strlen(trailer_buffer);
    response.header_start = 0;
    // process the headers
    result = process_trailers(&response);
    // Check the result
    assert(!result); // Should return true
    // Free response buffers
    free_response(&response);
}

// Tests free_response() which:
// Frees the memory used by the response
void test_free_response() {
    HTTPResponse response;
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));
    // Setup - Set the buffer
    ensure_buffer_size(&response, 100);
    // Test
    free_response(&response);
    // Check the result
    assert(response.buffer == NULL);
    assert(response.size == 0);
    assert(response.capacity == 0);
    assert(response.status_code == 0);
    assert(response.expected_body_size == 0);
    assert(response.body_length == 0);
    assert(response.header_length == 0);
    assert(response.trailer_length == 0);
    assert(response.status_line_start == 0);
    assert(response.header_start == 0);
    assert(response.body_start == 0);
    assert(response.trailer_start == 0);
    assert(response.chunked == false);
    assert(response.reading_phase == READING_STATUS_LINE);
    assert(response.reading_error_code == READ_ERROR_NONE);
    // Free response buffers
    free_response(&response);
    // Check the result
    assert(response.buffer == NULL);
    assert(response.size == 0);
    assert(response.capacity == 0);
    assert(response.status_code == 0);
    assert(response.expected_body_size == 0);
    assert(response.body_length == 0);
    assert(response.header_length == 0);
    assert(response.trailer_length == 0);
    assert(response.status_line_start == 0);
    assert(response.header_start == 0);
    assert(response.body_start == 0);
    assert(response.trailer_start == 0);
    assert(response.chunked == false);
    assert(response.reading_phase == READING_STATUS_LINE);
    assert(response.reading_error_code == READ_ERROR_NONE);
}

// Tests strip_whitespace() which:
// Strip a string of leading and trailing whitespace characters in place
void test_strip_whitespace(){
    char buffer[100];
    char *result;

    // Test with leading whitespace
    strcpy(buffer, "   Hello, World!   ");
    strip_whitespace(buffer);
    result = "Hello, World!";
    assert(strcmp(buffer, result) == 0);

    // Test with trailing whitespace
    strcpy(buffer, "Hello, World!   ");
    strip_whitespace(buffer);
    result = "Hello, World!";
    assert(strcmp(buffer, result) == 0);

    // Test with leading and trailing whitespace
    strcpy(buffer, "   Hello, World!   ");
    strip_whitespace(buffer);
    result = "Hello, World!";
    assert(strcmp(buffer, result) == 0);

    // Test with no whitespace
    strcpy(buffer, "Hello, World!");
    strip_whitespace(buffer);
    result = "Hello, World!";
    assert(strcmp(buffer, result) == 0);

    // Test with only whitespace
    strcpy(buffer, "   ");
    strip_whitespace(buffer);
    result = "";
    assert(strcmp(buffer, result) == 0);
}

// Tests parse_header_line() which:
// Parse a header line to get the header name and value
// Max length of header name and value is MAX_HEADER_NAME / MAX_HEADER_VALUE characters.
// Characters beyond this length are ignored (but assumed correctly formatted)
// The line is terminated by a newline character
// Returns true if the header was parsed successfully
void test_parse_header_line() {
    char header_name[100];
    char header_value[100];
    bool result;

    // Test with a valid header
    strcpy(mock_buffer, "Content-Length: 13\r\n");
    mock_index = 0;
    result = parse_header_line(mock_buffer, header_name, header_value);
    assert(result);
    assert(strcmp(header_name, "Content-Length") == 0);
    assert(strcmp(header_value, "13") == 0);

    // Test with a valid header with extra spaces
    strcpy(mock_buffer, "   Content-Length: 13   \r\n");
    mock_index = 0;
    result = parse_header_line(mock_buffer, header_name, header_value);
    assert(result);
    assert(strcmp(header_name, "Content-Length") == 0);
    assert(strcmp(header_value, "13") == 0);

    // Test with a valid header with extra spaces and a value with spaces
    strcpy(mock_buffer, "   Content-Length: 13   \r\n");
    mock_index = 0;
    result = parse_header_line(mock_buffer, header_name, header_value);
    assert(result);
    assert(strcmp(header_name, "Content-Length") == 0);
    assert(strcmp(header_value, "13") == 0);

    // Test with a valid header with extra spaces and a value with spaces
    strcpy(mock_buffer, "   Content-Length: 13   \r\n");
    mock_index = 0;
    result = parse_header_line(mock_buffer, header_name, header_value);
    assert(result);
    assert(strcmp(header_name, "Content-Length") == 0);
    assert(strcmp(header_value, "13") == 0);

    // Test with a valid header with extra spaces and a value with spaces
    strcpy(mock_buffer, "   Content-Length: 13   \r\n");
    mock_index = 0;
    result = parse_header_line(mock_buffer, header_name, header_value);
    assert(result);
    assert(strcmp(header_name, "Content-Length") == 0);
    assert(strcmp(header_value, "13") == 0);

    // Test with a valid header with extra spaces and a value with spaces
    strcpy(mock_buffer, "   Content-Length: 13   \r\n");
    mock_index = 0;
    result = parse_header_line(mock_buffer, header_name, header_value);
    assert(result);
    assert(strcmp(header_name, "Content Length") != 0);
    assert(strcmp(header_value, "13") == 0);
}

// Tests read_response() which:
// Reads a response from the socket and parses it
// *** This is the function clients use ***
// Handles chunked and non-chunked encoding but the response is read into an in-memory buffer, so
// it is not suitable for huge / streamed responses
// Returns true if the response was read successfully
void test_read_response() {
    HTTPResponse response;
    bool result;
    char *expected_body;
    char *expected_headers;
    char *expected_trailers;
    char *expected_status_line;
    // Clear response
    memset(&response, 0, sizeof(HTTPResponse));

    // Smoke test - non-chunked
    set_buffer("HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    assert(memcmp(HEADERS(&response), "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);

    // Smoke test - chunked
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    expected_body = "Wikipedia in\r\nchunks.";
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.body_length == strlen(expected_body));
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(HEADERS(&response), "Transfer-Encoding: chunked\r\n", 27) == 0);
    assert(memcmp(TRAILERS(&response), "Expires: Wed, 21 Oct 2015 07:28:00 GMT\r\n", 40) == 0);
    // Free response buffers
    free_response(&response);

    // Test - non-chunked - without a Content-Length header
    set_buffer("HTTP/1.1 200 OK\r\n\r\nHello, World!");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked with no trailers
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\n\r\n");
    expected_body = "Wikipedia in\r\nchunks.";
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.body_length == strlen(expected_body));
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(HEADERS(&response), "Transfer-Encoding: chunked\r\n", 27) == 0);
    assert(memcmp(TRAILERS(&response), "", 0) == 0);
    // Free response buffers
    free_response(&response);

    // Test - no Content
    set_buffer("HTTP/1.1 204 No Content\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 204);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 204 No Content") == 0);
    assert(response.body_length == 0);
    // Free response buffers
    free_response(&response);

    // Test - no headers
    set_buffer("HTTP/1.1 200 OK\r\n\r\nHello, World!");
    result = read_response(0, &response);
    assert(result);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    // Free response buffers
    free_response(&response);

    // Tests for read() breaks at different points in a non-chunked message
    // Test - non-chunked - break in status line
    set_buffer("HTTP/1~.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    assert(memcmp(HEADERS(&response), "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);

    // Test - non-chunked - break after status line
    set_buffer("HTTP/1.1 200 OK\r\n~Content-Length: 13\r\n\r\nHello, World!\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    assert(memcmp(HEADERS(&response), "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);

    // Test - non-chunked - break in headers
    set_buffer("HTTP/1.1 200 OK\r\nContent-~Length: 13\r\n\r\nHello, World!\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    assert(memcmp(HEADERS(&response), "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);

    // Test - non-chunked - break after headers but before header block end
    set_buffer("HTTP/1.1 200 OK\r\nContent-Length: 13\r\n~\r\nHello, World!\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    assert(memcmp(HEADERS(&response), "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);

    // Test - non-chunked - break after headers
    set_buffer("HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\n~Hello, World!\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    assert(memcmp(HEADERS(&response), "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);

    // Test - non-chunked - break in body
    set_buffer("HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHell~o, World!\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    assert(memcmp(HEADERS(&response), "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);

    // Test - non-chunked - break after body
    set_buffer("HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!~\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    assert(memcmp(HEADERS(&response), "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);

    // Test - non-chunked - break after body within newline
    set_buffer("HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!\r~\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    assert(memcmp(HEADERS(&response), "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);

    // Test - non-chunked - break after body and after terminating newline
    set_buffer("HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!\r\n~");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    assert(memcmp(HEADERS(&response), "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);

    // Test - non-chunked - break in body with no length
    set_buffer("HTTP/1.1 200 OK\r\n\r\nHello, ~World!");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    // Free response buffers
    free_response(&response);

    // Tests for read() breaks at different points in a chunked message
    // Test - chunked - break in status line
    expected_body = "Wikipedia in\r\nchunks.";
    expected_status_line = "HTTP/1.1 200 OK";
    expected_headers = "Transfer-Encoding: chunked\r\n";
    expected_trailers = "Expires: Wed, 21 Oct 2015 07:28:00 GMT\r\n";
    // Test - chunked - break in status line
    set_buffer("HTTP/1.1 2~00 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - break after status line
    set_buffer("HTTP/1.1 200 OK\r\n~Transfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - break in headers
    set_buffer("HTTP/1.1 200 OK\r\nTransf~er-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - break after headers but before header block end
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n~\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - break after headers
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n~4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - break in body - within chunk size
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4~\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - break in body - within size/body separator
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r~\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - break in body - within chunk body
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nW~iki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - break in body - between chunks
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n~5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - break in body - between null chunk body
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.~\r~\n~0~\r~\n~\r~\n~Expires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - in trailer header name
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExp~ires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - in trailer header name
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExp~ires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - in trailer header value
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015~ 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - in trailer terminating newline
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r~\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - in message terminating newline
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r~\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - no trailers
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\n\r\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == 0);
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    // Free response buffers
    free_response(&response);

    // Test - non-chunked - everything split!
    set_buffer("H~T~T~P~/~1~.~1~ ~2~0~0~ ~O~K~\r~\n~C~o~n~t~e~n~t~-~L~e~n~g~t~h~:~ ~1~3~\r~\n~\r~\n~H~e~l~l~o~,~ ~W~o~r~l~d~!~\r~\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(strcmp(STATUS_LINE(&response), "HTTP/1.1 200 OK") == 0);
    assert(response.expected_body_size == 13);
    assert(response.body_length == 13);
    assert(memcmp(BODY(&response), "Hello, World!", 13) == 0);
    assert(memcmp(HEADERS(&response), "Content-Length: 13\r\n", 19) == 0);
    // Free response buffers
    free_response(&response);

    // Test - chunked - everything split!
    set_buffer("H~T~T~P~/~1~.~1~ ~2~0~0~ ~O~K~\r~\n~T~r~a~n~s~f~e~r~-~E~n~c~o~d~i~n~g~:~ ~c~h~u~n~k~e~d~\r~\n~\r~\n~4~"
               "\r~\n~W~i~k~i~\r~\n~5~\r~\n~p~e~d~i~a~\r~\n~C~\r~\n~ ~i~n~\r~\n~c~h~u~n~k~s~.~\r~\n~0~\r~\n~\r~\n~E~x~p~i~r~"
               "e~s~:~ ~W~e~d~,~ ~2~1~ ~O~c~t~ ~2~0~1~5~ ~0~7~:~2~8~:~0~0~ ~G~M~T~\r~\n~\r~\n");
    result = read_response(0, &response);
    assert(result);
    assert(response.status_code == 200);
    assert(response.status_line_length == strlen(expected_status_line));
    assert(response.header_length == strlen(expected_headers));
    assert(response.body_length == strlen(expected_body));
    assert(response.trailer_length == strlen(expected_trailers));
    assert(memcmp(STATUS_LINE(&response), expected_status_line, strlen(expected_status_line)) == 0);
    assert(memcmp(HEADERS(&response), expected_headers, strlen(expected_headers)) == 0);
    assert(memcmp(BODY(&response), expected_body, strlen(expected_body)) == 0);
    assert(memcmp(TRAILERS(&response), expected_trailers, strlen(expected_trailers)) == 0);
    // Free response buffers
    free_response(&response);

    // Test Malformed requests
    // Test - no status line
    set_buffer("Content-Length: 13\r\n\r\nHello, World!\r\n");
    result = read_response(0, &response);
    assert(!result);
    assert(response.reading_error_code == READ_ERROR_MALFORMED_STATUS_LINE);
    free_response(&response);

    // Test - malformed status line
    set_buffer("HTTP 200 \r\nContent-Length: 13\r\n\r\nHello, World!\r\n");
    result = read_response(0, &response);
    assert(!result);
    assert(response.reading_error_code == READ_ERROR_MALFORMED_STATUS_LINE);
    free_response(&response);

    // Test - bad response code
    set_buffer("HTTP/1.1 2000 OK\r\nContent-Length: 13\r\n\r\nHello, World!\r\n");
    result = read_response(0, &response);
    assert(!result);
    assert(response.reading_error_code == READ_ERROR_INVALID_STATUS_CODE);
    free_response(&response);

    // Test - malformed headers
    set_buffer("HTTP/1.1 200 OK\r\nContent-Length 13\r\n\r\nHello, World!\r\n");
    result = read_response(0, &response);
    assert(!result);
    assert(response.reading_error_code == READ_ERROR_MALFORMED_HEADER);
    free_response(&response);

    // Test - incorrect body length (too small)
    set_buffer("HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, World!\r\n");
    result = read_response(0, &response);
    assert(!result);
    assert(response.reading_error_code == READ_ERROR_MALFORMED_BODY);
    free_response(&response);

    // Test - incorrect body length (too big)
    set_buffer("HTTP/1.1 200 OK\r\nContent-Length: 100\r\n\r\nHello, World!\r\n");
    result = read_response(0, &response);
    assert(!result);
    assert(response.reading_error_code == READ_ERROR_SOCKET_CLOSED);
    free_response(&response);

    // Test - malformed chunk
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n7\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n\r\n");
    result = read_response(0, &response);
    assert(!result);
    assert(response.reading_error_code == READ_ERROR_MALFORMED_CHUNK);
    free_response(&response);

    // Test - malformed trailer
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires - Wed, 21 Oct 2015\r\n\r\n");
    result = read_response(0, &response);
    assert(!result);
    assert(response.reading_error_code == READ_ERROR_MALFORMED_TRAILER);
    free_response(&response);

    // Test - no message end
    set_buffer("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\nC\r\n in\r\nchunks.\r\n0\r\n\r\nExpires: Wed, 21 Oct 2015 07:28:00 GMT\r\n");
    result = read_response(0, &response);
    assert(!result);
    assert(response.reading_error_code == READ_ERROR_SOCKET_CLOSED);
    free_response(&response);
}

// Mock Write Function output buffer
static char write_output[10 * 1024];
static size_t write_output_length = 0; // Note also that write() outputs at this point
static ssize_t mock_error = 0; // If set write will return this error number
// If set write will output mock_max_write characters, and move the write pointer
// The caller should repeat the call to write as needed - so this simulates this scenario
static size_t mock_max_write = 0;
// Mock Write Function
// Writes to write_output, null terminates for tester convenience
// Note for this test function buffer overflow is not tested for
ssize_t write(int fd, const void *buf, size_t count) {
    // Print the buffer being written
 //   printf("write \"%.*s\" to socket id %d \n", (int)count, (char*)buf, fd);
    if (mock_error) {
        return mock_error;
    }
    if (mock_max_write) {
        if (count > mock_max_write) count = mock_max_write;
    }
    memcpy(write_output + write_output_length, buf, count);
    write_output_length += count;
    write_output[write_output_length] = 0;
    return (ssize_t)count;
}

// Function to test flush_socket_buffer() which:
// flushes a SocketBuffer buffer to the socket
void test_flush_socket_buffer() {
    int rc;
    char *out = "Hello World";
    size_t out_length = strlen(out);
    SocketBuffer *buffer = malloc(sizeof(SocketBuffer *));
    buffer->capacity = 200;
    buffer->buffer = malloc(buffer->capacity);

    // Basic Test
    // Setup Buffer
    strcpy(buffer->buffer, out);
    buffer->length = out_length;
    // Setup Global write target buffer
    write_output_length = 0;
    mock_error = 0;
    mock_max_write = 0;
    // Test
    rc = flush_socket_buffer(buffer);
    assert(rc == 0);
    assert(strcmp(write_output,out)==0);
    assert(buffer->length == 0);

    // Test with write only writing a few chars at a time
    strcpy(buffer->buffer, out);
    buffer->length = out_length;
    // Setup Global write target buffer
    write_output_length = 0;
    mock_error = 0;
    mock_max_write = 3;
    // Test
    rc = flush_socket_buffer(buffer);
    assert(rc == 0);
    assert(strcmp(write_output,out)==0);
    assert(buffer->length == 0);

    // Test with a write error
    strcpy(buffer->buffer, out);
    buffer->length = out_length;
    // Setup Global write target buffer
    write_output_length = 0;
    mock_error = 0;
    mock_error = -1;
    // Test
    rc = flush_socket_buffer(buffer);
    assert(rc == -1);

    // Cleanup
    free(buffer->buffer);
    free(buffer);
}

// Function test write_to_socket_buffer() which writes data to a SocketBuffer buffer. When the buffer is full it is flushed to the socket
// If the length is bigger than the buffer capacity the buffer is flushed and the data is written directly to the socket
void test_write_to_socket_buffer() {
    int rc;
    char *out = "Hello World";
    size_t out_length = strlen(out);
    SocketBuffer *buffer;

    //  Test - Large buffer
    // Setup Buffer
    buffer = malloc(sizeof(SocketBuffer *));
    buffer->capacity = 200;
    buffer->buffer = malloc(buffer->capacity);
    buffer->buffer[0] = 0;
    buffer->length = 0;
    // Setup Global write target buffer
    write_output[0] = 0;
    write_output_length = 0;
    mock_error = 0;
    mock_max_write = 0;
    // Test
    rc = write_to_socket_buffer(buffer, out, out_length);
    // Nothing should be written to the socket (yet)
    assert(rc == 0);
    assert(strcmp(write_output,"")==0);
    assert(write_output_length == 0);
    assert(buffer->length == out_length);
    // Now Flush the buffer
    rc = flush_socket_buffer(buffer);
    assert(rc == 0);
    assert(strcmp(write_output,out)==0);
    assert(buffer->length == 0);
    assert(write_output_length == out_length);
    free(buffer->buffer);

    //  Test - Very small buffer
    // Setup Buffer
    buffer = malloc(sizeof(SocketBuffer *));
    buffer->capacity = 5;
    buffer->buffer = malloc(buffer->capacity);
    buffer->buffer[0] = 0;
    buffer->length = 0;
    // Setup Global write target buffer
    write_output[0] = 0;
    write_output_length = 0;
    mock_error = 0;
    mock_max_write = 0;
    // Test
    rc = write_to_socket_buffer(buffer, out, out_length);
    assert(rc == 0);
    assert(strcmp(write_output,out)==0);
    assert(buffer->length == 0);
    free(buffer->buffer);

    //  Test - Large buffer with small write capacity
    // Setup Buffer
    buffer = malloc(sizeof(SocketBuffer *));
    buffer->capacity = 200;
    buffer->buffer = malloc(buffer->capacity);
    buffer->buffer[0] = 0;
    buffer->length = 0;
    // Setup Global write target buffer
    write_output[0] = 0;
    write_output_length = 0;
    mock_error = 0;
    mock_max_write = 2;
    // Test
    rc = write_to_socket_buffer(buffer, out, out_length);
    // Nothing should be written to the socket (yet)
    assert(rc == 0);
    assert(strcmp(write_output,"")==0);
    assert(write_output_length == 0);
    assert(buffer->length == out_length);
    // Now Flush the buffer
    rc = flush_socket_buffer(buffer);
    assert(rc == 0);
    assert(strcmp(write_output,out)==0);
    assert(buffer->length == 0);
    assert(write_output_length == out_length);
    free(buffer->buffer);

    //  Test - Very small buffer with small write capacity
    // Setup Buffer
    buffer = malloc(sizeof(SocketBuffer *));
    buffer->capacity = 5;
    buffer->buffer = malloc(buffer->capacity);
    buffer->buffer[0] = 0;
    buffer->length = 0;
    // Setup Global write target buffer
    write_output[0] = 0;
    write_output_length = 0;
    mock_error = 0;
    mock_max_write = 3;
    // Test
    rc = write_to_socket_buffer(buffer, out, out_length);
    assert(rc == 0);
    assert(strcmp(write_output,out)==0);
    assert(buffer->length == 0);
    free(buffer->buffer);

    // Test with a write error
    buffer = malloc(sizeof(SocketBuffer *));
    buffer->capacity = 5;
    buffer->buffer = malloc(buffer->capacity);
    buffer->buffer[0] = 0;
    buffer->length = 0;
    // Setup Global write target buffer
    write_output[0] = 0;
    write_output_length = 0;
    mock_error = 0;
    mock_error = -1;
    // Test
    rc = write_to_socket_buffer(buffer, out, out_length);
    assert(rc == -1);
    free(buffer->buffer);

    // Cleanup
    free(buffer);
}

// Function to test flush_chunked_socket_buffer() which flushes a SocketBuffer buffer to the socket as a chunk
void test_flush_chunked_socket_buffer() {
    int rc;
    char *out = "Hello World";
    size_t out_length = strlen(out);
    SocketBuffer *buffer = malloc(sizeof(SocketBuffer *));
    buffer->capacity = 200;
    buffer->buffer = malloc(buffer->capacity);

    // Basic Test
    // Setup Buffer
    strcpy(buffer->buffer, out);
    buffer->length = out_length;
    // Setup Global write target buffer
    write_output_length = 0;
    mock_error = 0;
    mock_max_write = 0;
    // Test
    rc = flush_chunked_socket_buffer(buffer);
    assert(rc == 0);
    assert(strcmp(write_output,"b\r\nHello World\r\n")==0);
    assert(buffer->length == 0);

    // Test with write only writing a few chars at a time
    strcpy(buffer->buffer, out);
    buffer->length = out_length;
    // Setup Global write target buffer
    write_output_length = 0;
    mock_error = 0;
    mock_max_write = 3;
    // Test
    rc = flush_chunked_socket_buffer(buffer);
    assert(rc == 0);
    assert(strcmp(write_output,"b\r\nHello World\r\n")==0);
    assert(buffer->length == 0);

    // Test with a write error
    strcpy(buffer->buffer, out);
    buffer->length = out_length;
    // Setup Global write target buffer
    write_output_length = 0;
    mock_error = 0;
    mock_error = -1;
    // Test
    rc = flush_chunked_socket_buffer(buffer);
    assert(rc == -1);

    // Cleanup
    free(buffer->buffer);
    free(buffer);
}

// Function to test write_to_chunked_socket_buffer() which writes data to a SocketBuffer buffer. When the buffer is full it is flushed to the socket
// This function generates HTTP body chunked encoding
// If the length is bigger than the buffer capacity then it is added to the buffer and flushed in chunks
void test_write_to_chunked_socket_buffer() {
    int rc;
    char *out = "Hello World";
    size_t out_length = strlen(out);
    char *expected;
    size_t expected_length;
    SocketBuffer *buffer;

    buffer = malloc(sizeof(SocketBuffer *));

    //  Test - Large buffer
    // Setup Buffer
    buffer->capacity = 200;
    buffer->buffer = malloc(buffer->capacity);
    (buffer->buffer)[0] = 0;
    buffer->length = 0;
    // Setup Global write target buffer
    write_output[0] = 0;
    write_output_length = 0;
    mock_error = 0;
    mock_max_write = 0;
    // Test
    expected = "b\r\nHello World\r\n";
    expected_length = strlen(expected);
    rc = write_to_chunked_socket_buffer(buffer, out, out_length);
    // Nothing should be written to the socket (yet)
    assert(rc == 0);
    assert(strcmp(write_output,"")==0);
    assert(write_output_length == 0);
    assert(buffer->length == out_length);
    // Now Flush the buffer
    rc = flush_chunked_socket_buffer(buffer);
    assert(rc == 0);
    assert(strcmp(write_output,expected)==0);
    assert(buffer->length == 0);
    assert(write_output_length == expected_length);
    free(buffer->buffer);

    //  Test - Very small buffer
    // Setup Buffer
    buffer->capacity = 5;
    buffer->buffer = malloc(buffer->capacity);
    (buffer->buffer[0]) = 0;
    buffer->length = 0;
    // Setup Global write target buffer
    write_output[0] = 0;
    write_output_length = 0;
    mock_error = 0;
    mock_max_write = 0;
    // Test
    expected = "5\r\nHello\r\n5\r\n Worl\r\n";
    expected_length = strlen(expected);
    rc = write_to_chunked_socket_buffer(buffer, out, out_length);
    assert(rc == 0);
    assert(strcmp(write_output, expected)==0);
    assert(write_output_length == expected_length);
    assert(buffer->length == 1); // for the last d in World
    // Now Flush the buffer
    expected = "5\r\nHello\r\n5\r\n Worl\r\n1\r\nd\r\n";
    expected_length = strlen(expected);
    rc = flush_chunked_socket_buffer(buffer);
    assert(rc == 0);
    assert(strcmp(write_output,expected)==0);
    assert(buffer->length == 0);
    assert(write_output_length == expected_length);
    free(buffer->buffer);



    // Cleanup
    free(buffer);
}

// Function to test emit_to_socket() which is the JSON emitter functions for writing to socket using the HTTP chunked protocol
// This is used for sending JSON data over a socket connection
// The function buffers output into chuck sizes and sends the chunks over the socket
// context structure holding the buffer for the current chunk, size and socket
void test_emit_to_socket() {
}

int main() {
    test_read_into_buffer();
    test_ensure_buffer_size();
    test_process_headers();
    test_is_body_complete();
    test_process_body();
    test_process_trailers();
    test_free_response();
    test_strip_whitespace();
    test_parse_header_line();
    test_read_response();
    test_flush_socket_buffer();
    test_write_to_socket_buffer();
    test_flush_chunked_socket_buffer();
    test_write_to_chunked_socket_buffer();
    test_emit_to_socket();
    return 0;
}
