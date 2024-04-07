// This is the CREXX REXXSAA Client Library
//
// It communicates to the CREXX REXXSAA Server Component Via HTTP Transport / Restful API
// picking up the server details from the environment variable _CREXX_SERVER.
//
// The implementation philosophy is to keep the client as simple as possible with no external dependencies (except for SSL), but the client
// is not designed to be a full-featured HTTP client but is only designed to work with the CREXX REXXSAA Server Component.
// Blocking I/O is used for simplicity, single-threaded, except callbacks from the REXXSAA Server Component (e.g. for subcommand handlers)
// which are implemented using long-polling (i.e. the client sends a request and waits for a response) and in separate threads.
//
// Version : PoC -  Initial Proof of Concept - Unix Only
// Adrian Sutherland - 06/03/2024

// Library Header
#include "crexxsaa.h"
#include "httpclient.h"

// Dependencies
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TCP/IP Dependencies
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Include REXXSAA Header
#include "crexxsaa.h"

// Common http headers for all requests
#define HTTP_COMMON "Content-Type: application/json\r\nUser-Agent: CREXX/0.1\r\nKeep-Alive: timeout=5, max=1000\r\nConnection: Keep-Alive\r\nCache-Control: no-cache\r\n"

// Default http header string to be used for normal connections (including keep-alive, etc.)
#define HTTP_HEADER HTTP_COMMON

// Default http header string to be used for long-polling connections (including keep-alive, etc.)
#define HTTP_HEADER_LONG_POLL HTTP_COMMON

// strtok_crlf - Custom strtok for CRLF
char *strtok_crlf(char *str) {
    static char *next_start;

    if (str) next_start = str;
    else str = next_start;

    if (!next_start) return NULL;

    char *crlf = strstr(next_start, "\r\n");
    if (!crlf) {
        char *remaining = next_start;
        next_start = NULL;
        return remaining;
    }

    *crlf = '\0';
    next_start = crlf + 2;

    return str;
}

/* Connect to the server */
int rexx_connect() {

    // TODO Read Environment variable for server address

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        return -1;
    }

    return sock;
}

// REXXSAA -> REST Functions

int rexx_get_request(int sock, char* get_request) {
    char response[4096];
    char request[4096];
    int body_length = -1;
    int l;
    SHVBLOCK* shvblock = 0;

    snprintf(request, 4096, "GET %s HTTP/1.1\r\nHost: localhost\r\n" HTTP_HEADER "\r\n", get_request);
    printf("Request is %s\n", get_request);

    send(sock, request, strlen(request), 0);

    l = read(sock, response, sizeof(response)-1);
    response[l] = 0;

    // Parse HTTP Headers
    printf("Response Headers:\n");
    char *line = strtok_crlf(response);
    while (line && strlen(line)) {
        printf("%s\n", line);
        line = strtok_crlf(NULL);

        // Note as this client is only designed to works with CREXX it can assume the case of headers
        // TODO Confirm Server is CREXX/0.1

        // TODO Get Body Length
        if (strncmp("Content-Length:", line, 15) == 0) {
            body_length = atoi(line + 16);
        }
    }

    if (body_length <= 0) {
        fprintf(stderr, "Invalid response - no body length\n");
        return -1;
    }

    // Skip blank line
    line = strtok_crlf(NULL);

    // Print Body Payload
    line[body_length] = 0;
    printf("Response Body: %s\n", line);

    parseJSON(line, &shvblock);
    return 0;
}

// Function to retrieve the value of a REXX variable
int RexxVariableGet(const char *varName, RXSTRING *result) {
    // TODO Implement
    return 0;

}

int main() {
    printf("Starting\n");
    int sock = rexx_connect();
    if (sock < 0) return 1;

    rexx_get_request(sock, "/var/test1");
    rexx_get_request(sock, "/var/test2");

    printf("Finished\n");
    close(sock);

    return 0;
}
