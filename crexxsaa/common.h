//
// Created by Adrian Sutherland on 01/03/2024.
//

#ifndef REXXSAAPOC_COMMON_H
#define REXXSAAPOC_COMMON_H

#define MAX_MESSAGE_SIZE 1024
#define PORT 9000
#define IP "127.0.0.1"

// Receive a line from the socket
// Returns the length of the line (excluding the newline)
// Returns -1 on socket error
// Returns -2 on premature closure
// Returns -3 if the line exceeded max_size
long recv_line(int client_fd, char *buffer, int max_size);

#endif //REXXSAAPOC_COMMON_H
