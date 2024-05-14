//
// Created by Adrian Sutherland on 06/03/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Common http headers for all requests
#define HTTP_COMMON "Content-Type: application/json\r\nServer: CREXX/0.1\r\nKeep-Alive: timeout=5, max=1000\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\n"

// Default http header string to be used for normal connections
#define HTTP_HEADER HTTP_COMMON

// Default http header string to be used for long-polling connections
#define HTTP_HEADER_LONG_POLL HTTP_COMMON
/*
void request_handler(struct evhttp_request *req, void *arg) {
    struct evbuffer *evb = evbuffer_new();

    printf("Request received %s\n", req->uri);

    // Print all headers to the console
    struct evkeyvalq *headers = req->input_headers;
    struct evkeyval *header;
    TAILQ_FOREACH(header, headers, next) {
        printf("Header: %s: %s\n", header->key, header->value);
    }


    // Add headers to the response
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Server", "CREXX/0.1");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Cache-Control", "no-cache");

    // TODO Keep-Alive: timeout=5, max=1000
    // TODO Connection: Keep-Alive

    // Add response to the body of the response
    // {
    //    "variable_name": "<variable_value>", ...
    // }
    evbuffer_add_printf(evb, "{\"%s\":\"<variable_value>\"}", req->uri);

    evhttp_send_reply(req, HTTP_OK, "OK", evb);
    evbuffer_free(evb);
}
*/






#define MAX_BUFFER_SIZE 1024
#define PORT 8080

void handle_client(int client_socket) {
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, MAX_BUFFER_SIZE);

    // Read the client's request url
    read(client_socket, buffer, MAX_BUFFER_SIZE - 1);


    // Read the client's request
    read(client_socket, buffer, MAX_BUFFER_SIZE - 1);

    // Prepare the response
    char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello, World!\n";

    // Send the response to the client
    write(client_socket, response, sizeof(response));

    // Close the connection
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_len = sizeof(client_address);

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // Initialize server_address to zero
    memset(&server_address, 0, sizeof(server_address));

    // Set the values in server_address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    // Listen for connections
    listen(server_socket, 5);

    while (1) {
        // Accept a connection
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_len);
        if (client_socket < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        // Handle the client's request
        handle_client(client_socket);
    }

    return 0;
}

