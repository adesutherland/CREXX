//
// Created by adrian on 02/06/24.
//

#ifndef CREXX_HTTPSERVER_H
#define CREXX_HTTPSERVER_H

#define MAX_BUFFER_SIZE 1024
#define PORT 8080
#define SECRET_SIZE 16

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
void crexxsaa_server(int port, const char *shared_secret, pthread_t *thread, int *exit_fd, int *status_code);

// Status code for the server thread
#define SERVER_THREAD_EXITED_OK 0
#define SERVER_THREAD_STARTING_OK 1
#define SERVER_THREAD_LISTENING_OK 2
#define SERVER_THREAD_ERROR_CREATING_SOCKET (-1)
#define SERVER_THREAD_ERROR_BINDING_SOCKET (-2)
#define SERVER_THREAD_ERROR_SELECT (-3)
#define SERVER_THREAD_ERROR_UNKNOWN_FD (-4)
#define SERVER_THREAD_ERROR_ACCEPT (-5)

void handle_client(int client_socket);


#endif //CREXX_HTTPSERVER_H
