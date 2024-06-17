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

// Dependencies
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TCP/IP Dependencies
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Include REXXSAA Header
// #include "crexxsaa.h"
#include "jsnemit.h"
#include "httpcommon.h"

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

// Function to retrieve the value of a REXX variable
int RexxVariableGet(const char *varName, char **result) {
    SHVBLOCK *request = (SHVBLOCK *) malloc(sizeof(SHVBLOCK));
    SHVBLOCK *return_shvblock = 0;
    unsigned long rc;

    // Create request SHVBLOCK
    memset(request, 0, sizeof(SHVBLOCK)); // Zero the block
    request->shvname = (char*)varName;
    request->shvcode = RXSHV_FETCH;
    request->shvret = RXSHV_OK;
    request->shvnext = NULL;
    request->shvobject = NULL;

    // Execute
    rc = RexxVariablePool(request, &return_shvblock);
    if (rc) {
        fprintf(stderr, "Error in RexxVariablePool\n");
        return -1;
    }

    // Get the result
    if (return_shvblock->shvret == RXSHV_OK) {
        // We only support string values
        if (return_shvblock->shvobject->type != VALUE_STRING) {
            fprintf(stderr, "Error in RexxVariablePool - Unexpected value type\n");
            return -1;
        }
        // Copy the string to a new buffer
        *result = malloc(strlen(return_shvblock->shvobject->value.string) + 1);
        strcpy(*result, return_shvblock->shvobject->value.string);
    } else {
        fprintf(stderr, "Error in RexxVariablePool\n");
        return -1;
    }

    // Cleanup
    free(request);
    FreeRexxVariablePoolResult(return_shvblock);
    return 0;
}

/* RexxVariablePool - Interface to the REXX variable pool */
unsigned long RexxVariablePool(SHVBLOCK *request, SHVBLOCK **result) {
    int fd;
    int rc;

    // Connect to the server
    fd = rexx_connect();
    if (fd < 0) {
        fprintf(stderr, "Error connecting to server\n");
        return -1;
    }

    SocketBuffer *socket_buffer;
    socket_buffer = malloc(sizeof(SocketBuffer));
    socket_buffer->capacity = 0;
    socket_buffer->buffer = NULL;
    socket_buffer->length = 0;
    socket_buffer->socket = fd;
    socket_buffer->secret = "1234567890";
    socket_buffer->method = "GET";
    socket_buffer->uri = "/crexx/api/v1/get";
    socket_buffer->host = "localhost";
    socket_buffer->http_headers = 0;

    // Send the json request over the http socket
    rc = jsonEMIT(request, emit_to_socket, (void*)&socket_buffer);
    if (rc) {
        fprintf(stderr, "Error sending request\n");
        return -1;
    }

    // Read the response
    // Read the http response
    HTTPMessage *response = malloc(sizeof(HTTPMessage));

    // Clear response
    memset(response, 0, sizeof(HTTPMessage));
    if (read_message(fd, response)) {
        fprintf(stderr, "Error reading response\n");
        return -1;
    }

    // Parse the response
    PARSE_ERROR error;
    rc = parseJSON(response, result, &error);
    if (rc) {
        fprintf(stderr, "Error parsing response\n");
        return -1;
    }

    // Cleanup
    emit_to_socket(ACTION_CLOSE, NULL, (void*)&socket_buffer);
    free(socket_buffer);
    return 0;
}
