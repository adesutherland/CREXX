//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <stdint.h>
#include <string.h>
#include <ctype.h>

/* ------------------------------------------------------------------------------------------------
 * TCP flags, can be set by TCPFLAGS rexx function
 * ------------------------------------------------------------------------------------------------
*/
//
int tcp_debug=0;
tcp_max_clients=10;
uintptr_t *Messages=0;
/* ------------------------------------------------------------------------------------------------
 * +++ End of TCP flags section
 * ------------------------------------------------------------------------------------------------
*/
#define debug(cmt) if(tcp_debug==1) printf("TCP %s\n",cmt)
#define debugs(cmt,string) if(tcp_debug==1) printf("TCP %s %s\n",cmt,string)
#define debugi(cmt,ivalue) if(tcp_debug==1) printf("TCP %s %d\n",cmt,ivalue)
#define DIMI(var,max)    {int dimj; var = malloc((max+1) * sizeof(uintptr_t)); \
                          var[0]=max; for (dimj = 1; dimj <=max; dimj++) var[dimj] = 0;}
#define REDIMI(var,newmax)  {uintptr_t *newArr=0; DIMI(newArr,newmax); \
                              memcpy(newArr, var, var[0]* sizeof(uintptr_t)); \
                              newArr[0]=newmax;free(var); var=newArr; }  \

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

/* ------------------------------------------------------------------------------------------------
 * this and that
 * ------------------------------------------------------------------------------------------------
*/
void toUpperCase(char *str) {
    if (str == NULL) return; // Handle null pointer
    while (*str) {
        *str = toupper((unsigned char)*str); // Convert current character to uppercase
        str++;
    }
}
void GetWord(char *result, const char *str, int n) {
    int i = 0, j = 0, wordCount = 0;

    while (isspace((unsigned char)str[i])) i++;   // Skip leading spaces

    while (str[i] != '\0') {    // Traverse the string
        if (!isspace((unsigned char)str[i]) &&
            (i == 0 || isspace((unsigned char)str[i - 1]))) {   // Check if the current character starts a new word
            wordCount++; // Found a new word
        }
        if (wordCount == n) {   // If this is the N-th word, copy it to the buffer
            while (str[i] != '\0' && !isspace((unsigned char)str[i])) {
                result[j++] = str[i++];
            }
            break;
        }
        i++;
    }
    result[j] = '\0'; // Null-terminate the result
    if (wordCount < n) {      // If the N-th word wasn't found, set result to an empty string
        strcpy(result, "");
    }
}
int CountWords(const char *str) {
    int i = 0,wordCount = 0;

    while (isspace((unsigned char)str[i]))  i++;  // Skip leading spaces
    while (str[i] != '\0') {    // Traverse the string
        if (!isspace((unsigned char)str[i]) &&
            (i == 0 || isspace((unsigned char)str[i - 1]))) {    // Check if the current character starts a new word
            wordCount++; // Found a new word
        }
        i++;
    }
    return wordCount;
}
/* ------------------------------------------------------------------------------------------------
 * Open a TCP address to access a TCP server
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpopen) {
   WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in server_addr;
    int    result;
    char * ipaddr=GETSTRING(ARG0);
    int    port=GETINT(ARG1);

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        debug("Socket initialisation faild");
        RETURNINTX(-8)
    }

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        debugi("Socket creation failed with error:", WSAGetLastError());
        WSACleanup();
        RETURNINTX(-12);
    }
    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ipaddr, &server_addr.sin_addr) <= 0) {
        closesocket(sockfd);
        WSACleanup();
        debug("Set up IP:Port failed");
        RETURNINTX(-16);
    }
    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(sockfd);
        debug("Connect to server failed");
        WSACleanup();
        RETURNINTX(-20);
    }
    RETURNINTX(sockfd);
    ENDPROC
}
/* ------------------------------------------------------------------------------------------------
 * Sends a message to a TCP server
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpsend) {
    int socki= GETINT(ARG0);
    SOCKET sockfd= socki;
    if (socki<0) RETURNINTX(-64)
    char *message = GETSTRING(ARG1);
    debug("Send Message to server/client");
    if (send(sockfd, message, strlen(message), 0) == SOCKET_ERROR) RETURNINTX(-WSAGetLastError())
    else RETURNINTX(0)
    ENDPROC
}
/* ------------------------------------------------------------------------------------------------
 * Recive a message from a TCP server
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpreceive) {
    int buffer_size=10000;
    int time=GETINT(ARG1);
    char * buffer=malloc(buffer_size);
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
    debugi("Socket error",WSAGetLastError());
    RETURNSTRX("-8");
settimeout:
    debug("Time out occurred");
    RETURNSTRX("-4");
noalloc:
    debug("Buffer re-allocation failed");
    RETURNSTRX("-12");
ENDPROC
}
/* ------------------------------------------------------------------------------------------------
 * Create a TCP server
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpserver) {
    WSADATA wsa;
    SOCKET server;
    struct sockaddr_in serveraddr;
    int i, port = GETINT(ARG0);
    char ipdetails[128];
    // Winsock initialisieren
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        debugi("Winsock initialization failed. Error Code:", WSAGetLastError());
        RETURNINTX(-1);
    }
    debug("Winsock initialized");

     // Server-Socket erstellen
        if ((server = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            debugi("Could not create socket. Error Code:", WSAGetLastError());
            RETURNINTX(-2);
        }
        debug("Socket created");

        // Server-Adresse konfigurieren
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = INADDR_ANY;
        serveraddr.sin_port = htons(port);

        // Socket binden
        if (bind(server, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR) {
            debugi("Bind failed. Error Code:", WSAGetLastError());
            RETURNINTX(-3);
        }
        debug("Bind successful");

        // Auf Verbindungen lauschen
        if (listen(server, tcp_max_clients) == SOCKET_ERROR) {
            debugi("Listen failed. Error Code:", WSAGetLastError());
            RETURNINTX(-8);
        }
        debugi("Listening for incoming connections on port:",port);
        sprintf(ipdetails, "%d %s %d",server,inet_ntoa(serveraddr.sin_addr), ntohs(serveraddr.sin_port));
        SETARRAYHI(ARG1,1);    // reset ip address array
        SETSARRAY(ARG1,0,ipdetails);
        RETURNINTX(server);
 ENDPROC
}
int checkSocket(void * connections) {
    int i, num, result, hi=0;
    fd_set sockCheck;
    struct timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;

    for (i = 1; i < GETARRAYHI(connections); i++) {     // start with 1. item, 0. item is server socket
        char *firstWord = strtok(GETSARRAY(connections, i), " "); // delimeter is blank
        if (firstWord == NULL) continue;
        num = atoi(firstWord);
        FD_ZERO(&sockCheck);
        FD_SET(num, &sockCheck);
        result = select(0, &sockCheck, NULL, NULL, &timeout);
        if (result == 0) {
            hi=max(hi,num);
            continue;
        } else {
            debugi("Socket invalid:", num);
            REMOVEATTR(connections, i);
        }
    }
    return hi;
}
/* ------------------------------------------------------------------------------------------------
 * Waits to receive a message from a server (server mode)
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpwait){
        SOCKET server_socket=GETINT(ARG0),new_socket;
        struct sockaddr_in client;
        int client_len, addrhi;

        fd_set read_fds;
        int time=GETINT(ARG1);
        char ipdetails[128];

        struct timeval timeout;
        time=max(time,10);                // timeout minimum = 1/100 of a sec
        timeout.tv_sec = time/1000;       // timeout in 1/1000 secs, determine seconds
        timeout.tv_usec = time%1000*1000; // convert remaining part from milli secs to Micro secs

        addrhi=checkSocket(ARG2);

        while (1) {
            client_len = sizeof(client);
            FD_ZERO(&read_fds);
            FD_SET(server_socket, &read_fds);

            int activity = select(addrhi + 1, &read_fds, NULL, NULL, &timeout);
            if (activity == 0) {
               debug("Timeout: No incoming connection within 500 ms");
               RETURNINTX(-4);
            }
            new_socket = accept(server_socket, (struct sockaddr *) &client, &client_len);
            if (new_socket == INVALID_SOCKET) {
                debugi("Accept failed. Error Code:", WSAGetLastError());
                continue;
            }
            sprintf(ipdetails, "%d %s %d",new_socket,inet_ntoa(client.sin_addr), ntohs(client.sin_port));
            addrhi= GETARRAYHI(ARG2) + 1;
            SETARRAYHI(ARG2, addrhi);    // add new client entry to ip array
            SETSARRAY(ARG2, addrhi - 1, ipdetails);
            RETURNINTX(new_socket)
        }
ENDPROC
}
/* ------------------------------------------------------------------------------------------------
 * Close TCP connection
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpclose) {
    // Close the socket
    SOCKET sockfd = GETINT(ARG0);
    if (sockfd<0) RETURNINTX(-64);
    closesocket(sockfd);
    WSACleanup();
    debugi("Port %d closed",sockfd);
    RETURNINTX(0);
ENDPROC
}
// ***************** others **********************************
#else
// Linux/MAC Version
#endif
/* ------------------------------------------------------------------------------------------------
 * Wait pauses the execution (in milli seconds)
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(waitX) {
    wait(GETINT(ARG0));
    RETURNINTX(0);
ENDPROC
}
/* ------------------------------------------------------------------------------------------------
 * TCP Flags
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpflags) {
    char *flags = GETSTRING(ARG0);
    char word[32];
    int words = 0,i;
    toUpperCase(flags);
    words = CountWords(flags);
    printf("Flags before %s %d\n", flags, tcp_debug);
    if (strstr(flags, "RESET")) {
        tcp_debug = 0;
        tcp_max_clients = 10;
        RETURNINT(0);
    }
    for (i = 1; i <= words; i++) {
        GetWord(word,flags,i);
        if (strcmp(word, "NDEBUG")==0)     tcp_debug = 0;
        else if (strcmp(word, "DEBUG")==0) tcp_debug = 1;
        else if (strcmp(word, "MAXCLIENTS")==0) {
            i++;
            GetWord(word,flags,i);
            tcp_max_clients = atoi(word);
            if (tcp_max_clients==0) tcp_max_clients=10;
        }
    }
    printf("Flags after   %d\n",tcp_debug);
    RETURNINTX(0);
    ENDPROC
}
// RXTCP function definitions
LOADFUNCS
    ADDPROC(tcpflags,  "rxtcp.tcpflags", "b",".int", "ip=.string");
    ADDPROC(tcpopen,   "rxtcp.tcpopen", "b",".int", "ip=.string,port=.int");
    ADDPROC(tcpsend,   "rxtcp.tcpsend", "b",".int", "socket=.int,message=.string");
    ADDPROC(tcpreceive,"rxtcp.tcpreceive", "b",".string","socket=.int,timeout=.int");
    ADDPROC(tcpclose,  "rxtcp.tcpclose", "b",".int", "socket=.int");
    ADDPROC(waitX,     "rxtcp.wait", "b",".int", "timeout=.int");
    ADDPROC(tcpserver, "rxtcp.tcpserver", "b",".int", "port=.int,expose sockets=.string[]");
    ADDPROC(tcpwait,   "rxtcp.tcpwait", "b",".int", "server=.int,timeout=.int,expose sockets=.string[]");
ENDLOADFUNCS