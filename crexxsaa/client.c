#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"

#define SERVER_IP_ENV "RX_ENV_SERVER"
#define SERVER_PORT_ENV "RX_ENV_PORT"
#define DEFAULT_PORT 9000
#define DEFAULT_IP "127.0.0.1"
#define MAX_MESSAGE_SIZE 1024

// Connect to the server
int connect_to_server(char *server_ip, int port) {
    int client_fd;
    struct sockaddr_in address;

    // Check if server_ip is provided
    if (server_ip == NULL) {
        // Retrieve from environment variables
        server_ip = getenv(SERVER_IP_ENV);
        char *env_port = getenv(SERVER_PORT_ENV);

        if (server_ip == NULL) {
            server_ip = DEFAULT_IP;
        }

        if (env_port != NULL) {
            port = atoi(env_port);
        } else {
            port = DEFAULT_PORT;
        }
    }

    // Create socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Initialize address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(server_ip);
    address.sin_port = htons(port);

    // Connect to server
    if (connect(client_fd, (struct sockaddr *) &address, sizeof(address)) != 0) {
        return -1; // Indicate connection error
    }
    return client_fd;
}

// Close the socket
void close_server_connection(int client_fd) {
    close(client_fd);
}

// Send the message
int send_message(int client_fd, char *message) {
    int rc = send(client_fd, message, strlen(message), 0);
    send(client_fd, "\n", 1, 0); // Send the newline

    char response[MAX_MESSAGE_SIZE];
    recv_line(client_fd, response, sizeof(response));

    printf("Server response: %s\n", response);
    return rc;
}

// Main function
int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    char *server_ip = DEFAULT_IP;
    int rc;

    // Process command-line arguments
    if (argc > 1) {
        server_ip = argv[1];
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }

    // Connect to the server
    int client_fd = connect_to_server(server_ip, port);

    // Check if connection was successful
    if (client_fd < 0) {
        printf("Error connecting to server\n");
        return 1;
    }

    // Send message 1
    rc = send_message(client_fd, "SET|var1|value1");
    if (rc < 0) {
        printf("Error sending message 1\n");
    }

    // Send message 2
    rc = send_message(client_fd, "SET|var2|value2");
    if (rc < 0) {
        printf("Error sending message 2\n");
    }

    // Close the socket
    close_server_connection(client_fd);

    return 0;
}