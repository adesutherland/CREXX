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

// Handle the client's session
void *handle_client_thread(void *arg) {
    HTTPMessage response;
    bool result;
    memset(&response, 0, sizeof(HTTPMessage));
    // Set the error code to OK
    response.reading_error_code = READ_ERROR_NONE;
    while (response.reading_error_code == READ_ERROR_NONE) {
        if (read_response(0, &response)) {
            // Successfully read the response
            // Check the status code
            if (response.status_code == 200) {
                // Check the shared secret
                if (strncmp(response.buffer, "Shared-Secret: ", 15) == 0) {
                    if (strncmp(response.buffer + 15, "secret", 6) == 0) {
                        // Send the response
                        result = write_response(0, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
                        if (!result) {
                            // Print error message to stderr
                            fprintf(stderr, "Error in handle_client_thread() writing response\n");
                            // TODO - Raise a REXX Signal
                        }
                    }
                    else {
                        // Send the response
                        result = write_response(0, "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n");
                        if (!result) {
                            // Print error message to stderr
                            fprintf(stderr, "Error in handle_client_thread() writing response\n");
                            // TODO - Raise a REXX Signal
                        }
                    }
                }
                else {
                    // Send the response
                    result = write_response(0, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
                    if (!result) {
                        // Print error message to stderr
                        fprintf(stderr, "Error in handle_client_thread() writing response\n");
                        // TODO - Raise a REXX Signal
                    }
                }
            }
            else {
                // Send the response
                result = write_response(0, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
                if (!result) {
                    // Print error message to stderr
                    fprintf(stderr, "Error in handle_client_thread() writing response\n");
                    // TODO - Raise a REXX Signal
                }
            }

        }
        else {
            if (response.reading_error_code == READ_ERROR_SOCKET_CLOSED)
                return NULL; // Exit the thread - the client end closed the connection
            else {
                // Print error message / code to stderr
                fprintf(stderr, "Error in handle_client_thread() reading response: %d\n", response.reading_error_code);
                // TODO - Raise a REXX Signal
            }
        }
    }
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
            // Create a new thread to handle the client passing the client socket
            pthread_t client_thread;
            // Note passing the socket (as a size_t to avoid warnings) as the void* argument rather than the address of the socket
            // TODO Pass the fd and shared secret to the client thread in a malloced structure
            pthread_create(&client_thread, NULL, handle_client_thread, (void *)(size_t)client_socket);
            // Detach the client thread
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
