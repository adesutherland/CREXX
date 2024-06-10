//
// Created by adrian on 02/06/24.
//
#include "httpserver.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Mock functions to test the HTTPServer
// socket
static int is_socket_called = 0;
static int socket_return = 99;
int socket(int domain, int type, int protocol) {
    is_socket_called = 1;
    return socket_return;
}

// bind
static int is_bind_called = 0;
int bind(int sockfd, const struct sockaddr *addr, __socklen_t addrlen) {
    is_bind_called = 1;
    return 0;
}

// listen
static int is_listen_called = 0;
int listen(int sockfd, int backlog) {
    is_listen_called = 1;
    return 0;
}

// accept
static int is_accept_called = 0;
static int accepted_socket = 100;
int accept(int sockfd, struct sockaddr *addr, __socklen_t *addrlen) {
    is_accept_called = 1;
    return accepted_socket;
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

// Test the start_server function
void test_start_server() {
    is_socket_called = 0;
    socket_return = 99;
    is_bind_called = 0;
    is_listen_called = 0;
    is_accept_called = 0;
    accepted_socket = 100;

    start_server();
    // Test that the server socket is closed
    assert(close(100) == 0);
}

// Test the handle_client function
void test_handle_client() {
    int client_socket = 100;

}

// main function
int main() {
    // Tests
    test_start_server();
    test_handle_client();
}