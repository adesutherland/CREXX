//
// Created by Adrian Sutherland on 01/03/2024.
//
// Common Functions
#include <string.h>
#include <sys/socket.h>
#include "common.h"

// Receive a line from the socket
// Returns the length of the line (excluding the newline)
// Returns -1 on socket error
// Returns -2 on premature closure
// Returns -3 if the line exceeded max_size
long recv_line(int client_fd, char *buffer, int max_size) {
    static char remaining_buffer[MAX_MESSAGE_SIZE] = {0};
    static long remaining_size = 0;
    long total_bytes = 0;
    long rc;

    // Check if there is remaining data from the last call
    if (remaining_size > 0) {
        char *newline_pos = strchr(remaining_buffer, '\n');
        if (newline_pos != NULL) {
            // There is a newline in the remaining data
            long line_size = newline_pos - remaining_buffer;
            if (line_size < max_size) {
                // Copy the line to the buffer
                memcpy(buffer, remaining_buffer, line_size);
                buffer[line_size] = '\0'; // Null-terminate the line
                total_bytes = line_size;

                // Update the remaining buffer
                remaining_size -= (line_size + 1);
                memmove(remaining_buffer, newline_pos + 1, remaining_size);
                return total_bytes;
            } else {
                // The line is too long
                return -3;
            }
        } else {
            // There is no newline in the remaining data
            if (remaining_size < max_size) {
                // Copy the remaining data to the buffer
                memcpy(buffer, remaining_buffer, remaining_size);
                total_bytes = remaining_size;
                remaining_size = 0;
            } else {
                // The remaining data is too long
                return -3;
            }
        }
    }

    // Read from the socket
    while (1) {
        rc = recv(client_fd, buffer + total_bytes, 1, 0);
        if (rc < 0) {
            return -1; // Socket error
        } else if (rc == 0) {
            if (total_bytes > 0) return -2; // Premature closure
            else return 0; // No data
        }

        total_bytes += rc;
        if (total_bytes >= max_size) {
            return -3; // Line exceeded max_size
        }

        if (buffer[total_bytes - 1] == '\n') {
            buffer[total_bytes - 1] = '\0'; // Replace newline with null terminator
            return total_bytes - 1; // Return length of the line (excluding the newline)
        }
    }
}

