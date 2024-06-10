//
// Created by adrian on 29/05/24.
//

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "crexxsaa.h"
#include "httpclient.h"
#include "jsnemit.h"
#include "httpcommon.h"

// Mock functions to test the HTTPClient
// socket
int socket(int domain, int type, int protocol) {
    return 100;
}

// inet_pton
int inet_pton(int af, const char *src, void *dst) {
    return 0;
}

// connect
int connect(int sockfd, const struct sockaddr *addr, __socklen_t addrlen) {
    return 0;
}

// close
int close(int fd) {
    return 0;
}

char out_buffer[5000];
ssize_t out_buffer_size = 0;
char *in_buffer;
ssize_t in_buffer_size = 0;
ssize_t in_buffer_pos = 0;

// write
ssize_t write(int fd, const void *buf, size_t count) {
    if (fd != 100) {
        printf("Invalid file descriptor\n");
        return -1;
    }
    memcpy(out_buffer + out_buffer_size, buf, count);
    out_buffer_size += (ssize_t)count;
    return (ssize_t)count;
}

// read
ssize_t read(int fd, void *buf, size_t count) {
    if (fd != 100) {
        printf("Invalid file descriptor\n");
        return -1;
    }
    if (in_buffer_pos >= in_buffer_size) {
        return 0;
    }
    ssize_t bytes_to_copy = (ssize_t)count;
    if (in_buffer_pos + bytes_to_copy > in_buffer_size) {
        bytes_to_copy = in_buffer_size - in_buffer_pos;
    }
    memcpy(buf, in_buffer + in_buffer_pos, bytes_to_copy);
    in_buffer_pos += bytes_to_copy;
    return (ssize_t)bytes_to_copy;
}

// Test RexxVariableGet
void test_RexxVariableGet() {
    char* result;

    in_buffer = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 83\r\n\r\n"
                "{\"serviceBlocks\":[{\"name\":\"TEST\",\"request\":\"fetch\",\"result\":\"ok\",\"value\":\"Hello\"}]}";
    in_buffer_size = (ssize_t)strlen(in_buffer);
    int rc = RexxVariableGet("TEST", &result);
    assert(rc == 0);
    assert(strcmp(result, "Hello") == 0);
    free(result);
}

// main function
int main() {
    // Test RexxVariableGet
    test_RexxVariableGet();
}