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
#include <pthread.h>

#include "httpserver.h"
#include "httpcommon.h"

// Structure to hold the server parameters to pass to the server thread
struct http_server_data {
    int listen_port;
    char shared_secret[SECRET_SIZE + 1];
    int exit_fd;
    int *status_code; // Address of the status code
};

// Structure to hold the client parameters to pass to the client thread
struct http_client_data {
    int client_socket;
    char shared_secret[SECRET_SIZE + 1];
};

// Handle the client's session
void *handle_client_thread(void *arg) {
    struct http_client_data *data = (struct http_client_data *)arg;
    HTTPMessage request;
    bool result;
    memset(&request, 0, sizeof(HTTPMessage));
    // Set the error code to OK
    request.reading_error_code = READ_ERROR_NONE;

    while (request.reading_error_code == READ_ERROR_NONE) {
        if (read_message(data->client_socket, &request)) {

            // Check the shared secret
            if (strcmp(request.secret, data->shared_secret) != 0) {
                // Shared secret is incorrect
                // Send the error request - don't worry about error handling
                char* error_result = "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n";
                write_all(data->client_socket, error_result, strlen(error_result));
                break;
            }

            // Check the request method
            if (strcmp(request.method, "POST") != 0) {
                // Invalid method
                // Send the error request - don't worry about error handling
                char* error_result = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
                write_all(data->client_socket, error_result, strlen(error_result));
                break;
            }

            // Check the body type
            if (strcmp(request.content_type, "application/json") == 0) {
                // Invalid body type
                // Send the error request - don't worry about error handling
                char* error_result = "HTTP/1.1 415 Unsupported Media Type\r\nContent-Length: 0\r\n\r\n";
                write_all(data->client_socket, error_result, strlen(error_result));
                break;
            }

            // Check the URI
            if (strcmp(request.uri, "/crexx/api/v1/shvblock") == 0) {
                // Handle the shvblock request
                // Parse the JSON
                SHVBLOCK *shvblock_handle = 0;
                PARSE_ERROR error;
                int rc = parseJSON(&request, &shvblock_handle, &error);
                if (rc != 0) {
                    // Error parsing JSON
                    // Send the error request - don't worry about error handling
                    char* error_result = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
                    write_all(data->client_socket, error_result, strlen(error_result));
                    // Free the shvblock handle
                    FreeRexxVariablePoolResult(shvblock_handle);
                    break;
                }

                // Process the shvblock request

                // Emit the JSON

                // Free the shvblock handle
                FreeRexxVariablePoolResult(shvblock_handle); // todo check if this is right
            }
            // Other request types will be handled here
            else {
                // Invalid URI
                // Send the error request - don't worry about error handling
                char* error_result = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
                write_all(data->client_socket, error_result, strlen(error_result));
                break;
            }

        }
        else {
            if (request.reading_error_code != READ_ERROR_SOCKET_CLOSED) {
                // Note: If it was READ_ERROR_SOCKET_CLOSED then the client closed the connection
-
                // Some Error - so print error message / code to stderr
                fprintf(stderr, "Error in handle_client_thread() reading request: %d\n", request.reading_error_code);
                // TODO - Raise a REXX Signal
            }
            break;
        }
    }
    free(arg);
    free_message(&request);
    return NULL;
}

// server_thread is the function that runs in the server thread
// It listens for connections and calls handle_client() for each client
// that connects.
// We always return NULL from this function and result information is passed
// back via the data structure.
void *server_thread(void *arg) {
    struct http_server_data *data = (struct http_server_data *)arg;
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_len = sizeof(client_address);

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        *(data->status_code) = SERVER_THREAD_ERROR_CREATING_SOCKET;
        free(data);
        return NULL;
    }

    // Initialize server_address to zero
    memset(&server_address, 0, sizeof(server_address));

    // Set the values in server_address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(data->listen_port);

    // Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        *(data->status_code) = SERVER_THREAD_ERROR_BINDING_SOCKET;
        free(data);
        return NULL;
    }

    // Listen for connections
    listen(server_socket, 5);

    fd_set readfds;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(data->exit_fd, &readfds); // Add the exit fd to the set
        FD_SET(server_socket, &readfds); // Add the server socket fd to the set
        int nfds = data->exit_fd > server_socket ? data->exit_fd + 1 : server_socket + 1; // Calculate the nfds

        if (select(nfds , &readfds, NULL, NULL, NULL) == -1) {
            *(data->status_code) = SERVER_THREAD_ERROR_SELECT;
            free(data);
            return NULL;
        }

         // Check if the exit fd is set
        if (FD_ISSET(data->exit_fd, &readfds)) {
            // Close the server socket
            close(server_socket);
            // Set the status code to indicate the server thread exited cleanly
            *(data->status_code) = SERVER_THREAD_EXITED_OK;
            // Free the data structure
            free(data);
            return NULL;
        }

        // Otherwise, handle the HTTP request
        else if (FD_ISSET(server_socket, &readfds)) {
            // Accept a connection
            client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_len);
            if (client_socket < 0) {
                *(data->status_code) = SERVER_THREAD_ERROR_ACCEPT;
                free(data);
                return NULL;
            }

            // Handle the client's request
            // Create the data structure to pass to the client thread
            struct http_client_data *client_data = malloc(sizeof(struct http_client_data));
            client_data->client_socket = client_socket;
            strncpy(client_data->shared_secret, data->shared_secret, SECRET_SIZE);
            client_data->shared_secret[SECRET_SIZE] = '\0';
            // Note the client thread will free this data structure

            // Create a new thread to handle the client passing the client socket
            pthread_t client_thread;
            pthread_create(&client_thread, NULL, handle_client_thread, (void *)client_data);
            // Detach the client thread - its on its own!
            pthread_detach(client_thread);
            // Continue to listen for connections
        }
        else {
            // This should never happen
            *(data->status_code) = SERVER_THREAD_ERROR_UNKNOWN_FD;
            free(data);
            return NULL;
        }
    }
}

// This function starts a thread listening on a server socket and
// calls handle_client() for each client that connects - it is a simple
// single-threaded server.
// The argument are
// - The port number to listen on.
// - The shared secret is used to authenticate the client.
// - Returns the thread id of the server thread (the caller should detach or join as needed).
// - Returns the fd that is used by the caller to signal the thread should exit.
//   This is used to signal the thread to exit (the fd is closed by the main thread
//   to signal the server thread to exit).
// - Returns the status code of the server thread (the main thread can check this to see if the
//   server thread exited cleanly). The memory for this return code needs to exist until the
//   server thread exits.
// This function returns immediately and the server runs in a separate thread.
void crexxsaa_server(int port, const char *shared_secret, pthread_t *thread, int *exit_fd, int *status_code) {
    struct http_server_data *data = malloc(sizeof(struct http_server_data));
    int rc;

    // Create a pair of file descriptors for signaling the server thread to exit
    int pipe_pair[2];
    if (pipe(pipe_pair) == -1) {
        // Treat like a malloc failure - just panic!
        perror("PANIC - Error creating pipe");
        exit(1);
    }
    *exit_fd = pipe_pair[1]; // Write to (or close) this fd to signal the server thread to exit

    // Set data server structure
    data->status_code = status_code;
    *(data->status_code) = 0;
    data->exit_fd = pipe_pair[0]; // Read from this fd to check if the server thread has exited
    data->listen_port = port;
    strncpy(data->shared_secret, shared_secret, SECRET_SIZE);
    data->shared_secret[SECRET_SIZE] = '\0';

    // Spawn a thread to listen for connections - this thread will exit when the exit_fd is closed
    rc = pthread_create(thread, NULL, server_thread, &data);
    // Check for errors
    if (rc) {
        // Treat this as a fatal error (like a malloc failure) - just panic!
        perror("PANIC - Error creating thread");
        exit(1);
    }

    // Note that the thread will free the data structure when it exits
}
