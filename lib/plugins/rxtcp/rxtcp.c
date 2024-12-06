//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
  #define wait(ms) Sleep(ms)
#else
// #include <arpa/inet.h>    // Linux
   #define wait(ms) usleep(ms*1000)
#endif
// ***************** Windows **********************************
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h> // For inet_pton() and other address conversion functions
#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

PROCEDURE(tcpopen) {
   WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in server_addr;
    int    result;
    char * ipaddr=GETSTRING(ARG0);
    int    port=GETINT(ARG1);

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) RETURNINTX(-8)

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        printf("Socket creation failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        RETURNINTX(-12);
    }
    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ipaddr, &server_addr.sin_addr) <= 0) {
        closesocket(sockfd);
        WSACleanup();
        RETURNINTX(-16);
    }
    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(sockfd);
        WSACleanup();
        RETURNINTX(-20);
    }
    RETURNINTX(sockfd);
    ENDPROC
}
PROCEDURE(tcpsend) {
    int socki= GETINT(ARG0);
    SOCKET sockfd= socki;
    if (socki<0) RETURNINTX(-64)
    char *message = GETSTRING(ARG1);
 // Send a message to the server
    if (send(sockfd, message, strlen(message), 0) == SOCKET_ERROR) RETURNINTX(-WSAGetLastError())
    else RETURNINTX(0)
    ENDPROC
}
PROCEDURE(tcpreceive) {
    int buffer_size=1000;
    int time=GETINT(ARG1);
    char * buffer=malloc(buffer_size);
    char error[80];
    int socki= GETINT(ARG0);
    SOCKET sockfd= socki;
    struct timeval timeout;
    int bytes_received,total_received=0;
    u_long remaining_bytes;

    if (socki<0) RETURNINTX(-64);
 // set timeout
    time=max(time,10);                // timeout minimum = 1/100 of a sec
    timeout.tv_sec = time/1000;       // timeout in 1/1000 secs, determine seconds
    timeout.tv_usec = time%1000*1000; // convert remaining part from milli secs to Micro secs
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout)) < 0)
        goto settimeout;  // SO_RCVTIMEO

    wait(10);           // wait 10 ms to make sure first block is ready to receive
    while (1 == 1) {    // now read all data from socket
        bytes_received = recv(sockfd, buffer + total_received, buffer_size - total_received - 1, 0);
        if (bytes_received == SOCKET_ERROR) goto socketerror;
        total_received += bytes_received;
        if (total_received >= buffer_size - 1) {
            buffer_size *= 2;  // increase buffer
            buffer = realloc(buffer, buffer_size);
            if (buffer == NULL) goto noalloc;    // buffer allocation failed
        }
        if (total_received == 0) break;         // Connection closed by the server
        ioctlsocket(sockfd, FIONREAD, &remaining_bytes);  // how many bytes are waiting
        if (remaining_bytes <= 0) break;        // buffer is empty
    }
     buffer[total_received] = '\0';  // Null-terminate the received data
     RETURNSTRX(buffer);
socketerror:
    sprintf(error, "-%d\n", WSAGetLastError());
    RETURNSTRX(error);
settimeout:
    RETURNSTRX("-4");
noalloc:
    RETURNSTRX("-12");
ENDPROC
}

/*    in case socket attributes are needed
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(sockfd, &read_set);
    select(sockfd + 1, &read_set, NULL, NULL, &timeout);
    ioctlsocket(sockfd, FIONREAD, &count);
*/

PROCEDURE(tcpclose) {
    // Close the socket
    SOCKET sockfd = GETINT(ARG0);
    if (sockfd<0) RETURNINTX(-64);
    closesocket(sockfd);
    WSACleanup();
    RETURNINTX(0);
ENDPROC
}
// ***************** others **********************************
#else
PROCEDURE(tcpopen) {
    int sockfd;
/*
    struct sockaddr_in server_addr;
    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
     // perror("Socket creation failed");
        RETURNINT(-8);
        PROCRETURN
    }
    printf("Socket created successfully.\n");
    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
     // perror("Invalid address/ Address not supported");
        RETURNINT(-12);
        PROCRETURN
    }
    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    // perror("Connection to the server failed");
        close(sockfd);
        RETURNINT(-16);
        PROCRETURN
    }
    printf("Connected to the server.\n");
 */
    ENDPROC
}
PROCEDURE(tcpsend) {
 /*   SOCKET sockfd = GETINT(ARG0);
    // Send a message to the server
    const char *message = GETSTRING(ARG1);
    send(sockfd, message, strlen(message), 0);
    printf("Message sent to server: %s\n", message);
*/
 ENDPROC
}
PROCEDURE(tcpreceive) {
 /*   SOCKET sockfd = GETINT(ARG0);
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        // perror("Failed to receive data");
        RETURNSTR("-8");
        PROCRETURN
    } else {
        buffer[bytes_received] = '\0';  // Null-terminate the received data
        printf("Server response: %s\n", buffer);
        RETURNSTR("-8");
    }
 */
    ENDPROC
}
PROCEDURE(tcpclose) {   // Close the socket
 /*   SOCKET sockfd = GETINT(ARG0);
    close(sockfd);
    printf("Connection closed.\n");
    RETURNINT(0);
    PROCRETURN*/
    ENDPROC
}
#endif
PROCEDURE(waitX) {
    // Close the socket
    wait(GETINT(ARG0));
    RETURNINTX(0);
ENDPROC
}
// RXTCP function definitions
LOADFUNCS
    ADDPROC(tcpopen,   "rxtcp.tcpopen", "b",".int", "ip=.string,port=.int");
    ADDPROC(tcpsend,   "rxtcp.tcpsend", "b",".int", "socket=.int,message=.string");
    ADDPROC(tcpreceive,"rxtcp.tcpreceive", "b",".string","socket=.int,timeout=.int");
    ADDPROC(tcpclose,  "rxtcp.tcpclose", "b",".int", "socket=.int");
    ADDPROC(waitX,      "rxtcp.wait", "b",".int", "timeout=.int");
 //   ADDPROC(tcpserver,  "rxtcp.tcpserver", "b",".int", "port=.int");
ENDLOADFUNCS