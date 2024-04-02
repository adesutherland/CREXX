#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"

// Connect to the server
int server(char *listen_ip, int port) {
    int server_fd, client_fd;
    struct sockaddr_in address;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Initialize address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(listen_ip);
    address.sin_port = htons(port);

    // Bind socket
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    // Listen for connections (allow a backlog of one connection)
    listen(server_fd, 1);

    // Accept connection
    client_fd = accept(server_fd, NULL, NULL);

    // Command processing loop
    while (1) {
        char buffer[MAX_MESSAGE_SIZE];
        int bytes_received = recv_line(client_fd, buffer, MAX_MESSAGE_SIZE);

        if (bytes_received < 0) {
            // Handle error based on the rc value
            if (bytes_received == -1) {
                printf("Socket error\n");
            } else if (bytes_received == -2) {
                printf("Connection closed\n");
            } else if (bytes_received == -3) {
                printf("Line exceeded max_size\n");
            }
            return -1;
        }

        else if (bytes_received == 0) {
            // Connection closed or error
            break;
        }

        // Simple parsing (very rudimentary)
        char *command, *var_name, *var_value;
        command = strtok(buffer, "|");
        var_name = strtok(NULL, "|");
        var_value = strtok(NULL, "|");

        char response[MAX_MESSAGE_SIZE] = "RC=0"; // Default success

        if (!command || strcmp(command, "SET") != 0 || !var_name || !var_value) {
            strcpy(response, "RC=1|Invalid command format");
        } else {
            // Logic to set variables in REXX environment)
            printf("Setting variable: %s = %s\n", var_name, var_value);

            // If variable setting failed, update response:
            //if (/* your condition for failed setting */) {
            //    strcpy(response, "RC=2|Failed to set variable\n");
            //}
        }

        send(client_fd, response, strlen(response), 0);
        send(client_fd, "\n", 1, 0); // Send the newline
    }

    // Close sockets
    close(client_fd);
    close(server_fd);

    return 0;
}

int main(int argc, char *argv[]) {
    int port = PORT;  // Default port
    char *listen_ip = IP; // Default IP
    int rc;

    // Process command-line arguments
    if (argc > 1) {
        listen_ip = argv[1];
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }

    // Start server
    rc = server(listen_ip, port);
    if (rc != 0) {
        printf("Server error rc=%d\n", rc);
        return 1;
    }

    return 0;
}