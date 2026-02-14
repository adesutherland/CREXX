//
// RXTCP plugin for crexx/pa - portable TCP client/server helpers
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

/*
 * C90-compatible max helper (macOS/Linux don't provide a generic max()).
 * Use a distinct name to avoid clashes with platform headers.
 */
#ifndef RX_MAX
#define RX_MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

/* ------------------------------------------------------------------------------------------------
 * TCP flags, can be set by TCPFLAGS rexx function
 * ------------------------------------------------------------------------------------------------
 */
static int tcp_debug = 0;
static int tcp_max_clients = 10;
static uintptr_t *Messages = 0;
/* ------------------------------------------------------------------------------------------------
 * +++ End of TCP flags section
 * ------------------------------------------------------------------------------------------------
 */

#define debug(cmt)            do { if (tcp_debug==1) printf("TCP %s\n",(cmt)); } while(0)
#define debugs(cmt,string)    do { if (tcp_debug==1) printf("TCP %s %s\n",(cmt),(string)); } while(0)
#define debugi(cmt,ivalue)    do { if (tcp_debug==1) printf("TCP %s %d\n",(cmt),(ivalue)); } while(0)

#define DIMI(var,max)    {int dimj; (var) = malloc(((max)+1) * sizeof(uintptr_t)); \
                          (var)[0]=(max); for (dimj = 1; dimj <=(max); dimj++) (var)[dimj] = 0;}
#define REDIMI(var,newmax)  {uintptr_t *newArr=0; DIMI(newArr,(newmax)); \
                              memcpy(newArr, (var), (var)[0]* sizeof(uintptr_t)); \
                              newArr[0]=(newmax);free((var)); (var)=newArr; }

#if defined(_WIN32)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef SOCKET socket_t;
  #define socket_invalid(s)   ((s) == INVALID_SOCKET)
  #define socket_err()        ((int)WSAGetLastError())
  static int socket_init(void) {
      WSADATA wsaData;
      int r = WSAStartup(MAKEWORD(2,2), &wsaData);
      return (r == 0) ? 0 : -1;
  }
  static void socket_cleanup(void) { WSACleanup(); }
  static void socket_close(socket_t s) { closesocket(s); }
  static void sleep_ms(int ms) { Sleep((DWORD)ms); }
  static int socket_bytes_available(socket_t s, unsigned long *n) {
      return ioctlsocket(s, FIONREAD, n);
  }
  static int socket_set_rcv_timeout(socket_t s, int timeout_ms) {
      DWORD t = (DWORD)timeout_ms;
      return setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&t, sizeof(t));
  }
  static int select_nfds(socket_t maxfd) { (void)maxfd; return 0; } // ignored on Windows
#else
  #include <errno.h>
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <sys/time.h>
  #include <sys/ioctl.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  typedef int socket_t;
  #ifndef INVALID_SOCKET
    #define INVALID_SOCKET (-1)
  #endif
  #ifndef SOCKET_ERROR
    #define SOCKET_ERROR  (-1)
  #endif
  #define socket_invalid(s)   ((s) < 0)
  #define socket_err()        (errno)
  static int socket_init(void) { return 0; }
  static void socket_cleanup(void) { }
  static void socket_close(socket_t s) { close(s); }
  static void sleep_ms(int ms) { usleep((useconds_t)ms * 1000u); }
  static int socket_bytes_available(socket_t s, unsigned long *n) {
      int v = 0;
      int r = ioctl(s, FIONREAD, &v);
      if (r == 0) *n = (unsigned long)v;
      return r;
  }
  static int socket_set_rcv_timeout(socket_t s, int timeout_ms) {
      struct timeval tv;
      tv.tv_sec  = timeout_ms / 1000;
      tv.tv_usec = (timeout_ms % 1000) * 1000;
      return setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
  }
  static int select_nfds(socket_t maxfd) { return (int)maxfd + 1; }
#endif

/* ------------------------------------------------------------------------------------------------
 * Local string/word helpers (file-local; keep out of platform ifdefs)
 * ------------------------------------------------------------------------------------------------
 */
static void toUpperCase(char *str) {
    if (!str) return;
    for (; *str; ++str) *str = (char)toupper((unsigned char)*str);
}

static int CountWords(const char *str) {
    int i = 0, wordCount = 0;
    if (!str) return 0;
    while (isspace((unsigned char)str[i])) i++;
    while (str[i] != '\0') {
        if (!isspace((unsigned char)str[i]) && (i == 0 || isspace((unsigned char)str[i-1]))) {
            wordCount++;
        }
        i++;
    }
    return wordCount;
}

static void GetWord(char *result, const char *str, int n) {
    int i = 0, j = 0, wordCount = 0;
    if (!result) return;
    result[0] = '\0';
    if (!str || n <= 0) return;

    while (isspace((unsigned char)str[i])) i++;
    while (str[i] != '\0') {
        if (!isspace((unsigned char)str[i]) && (i == 0 || isspace((unsigned char)str[i-1]))) {
            wordCount++;
        }
        if (wordCount == n) {
            while (str[i] != '\0' && !isspace((unsigned char)str[i])) {
                result[j++] = str[i++];
                if (j >= 31) break; // callers typically use small buffers
            }
            break;
        }
        i++;
    }
    result[j] = '\0';
}

/* ------------------------------------------------------------------------------------------------
 * Open a TCP address to access a TCP server (IPv4 dotted-quad)
 * rxtcp.tcpopen(ip,port) -> socket int (>=0) or negative error code
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpopen) {
    const char *ipaddr = GETSTRING(ARG0);
    int port = GETINT(ARG1);

    if (!ipaddr || port <= 0 || port > 65535) RETURNINTX(-2);

    if (socket_init() != 0) {
        debug("Socket initialization failed");
        RETURNINTX(-8);
    }

    socket_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_invalid(sockfd)) {
        debugi("Socket creation failed", socket_err());
        socket_cleanup();
        RETURNINTX(-socket_err());
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons((uint16_t)port);

    if (inet_pton(AF_INET, ipaddr, &server_addr.sin_addr) != 1) {
        debug("Invalid IPv4 address");
        socket_close(sockfd);
        socket_cleanup();
        RETURNINTX(-3);
    }

    if (connect(sockfd, (struct sockaddr*)&server_addr, (socklen_t)sizeof(server_addr)) == SOCKET_ERROR) {
        int e = socket_err();
        debugi("Connect failed", e);
        socket_close(sockfd);
        socket_cleanup();
        RETURNINTX(-e);
    }

    RETURNINTX((intptr_t)sockfd);
    ENDPROC
}

/* ------------------------------------------------------------------------------------------------
 * Send a message to a TCP server/client
 * rxtcp.tcpsend(socket,message) -> 0 or negative error
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpsend) {
    intptr_t socki = (intptr_t)GETINT(ARG0);
    if (socki < 0) RETURNINTX(-64);
    socket_t sockfd = (socket_t)socki;

    const char *message = GETSTRING(ARG1);
    if (!message) message = "";

    debug("Send message");
    int r = (int)send(sockfd, message, (int)strlen(message), 0);
    if (r == SOCKET_ERROR) RETURNINTX(-socket_err());
    RETURNINTX(0);
    ENDPROC
}

/* ------------------------------------------------------------------------------------------------
 * Receive data from a TCP server
 * rxtcp.tcpreceive(socket,timeout_ms) -> string data or "-4"/"-8"/"-12" legacy codes
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpreceive) {
    int buffer_size = 10000;
    int timeout_ms  = GETINT(ARG1);
    intptr_t socki  = (intptr_t)GETINT(ARG0);

    if (socki < 0) RETURNSTRX("-64");
    socket_t sockfd = (socket_t)socki;

    char *buffer = (char*)malloc((size_t)buffer_size);
    if (!buffer) RETURNSTRX("-12");

    timeout_ms = RX_MAX(timeout_ms, 10);

    if (socket_set_rcv_timeout(sockfd, timeout_ms) < 0) {
        debug("Set timeout failed");
        free(buffer);
        RETURNSTRX("-4");
    }

    sleep_ms(10);

    int total_received = 0;
    while (1) {
        int bytes_received = (int)recv(sockfd,
                                       buffer + total_received,
                                       buffer_size - total_received - 1,
                                       0);
        if (bytes_received == SOCKET_ERROR) {
            debugi("Socket error", socket_err());
            free(buffer);
            RETURNSTRX("-8");
        }
        if (bytes_received == 0) break; // connection closed

        total_received += bytes_received;
        if (total_received >= buffer_size - 1) {
            buffer_size *= 2;
            char *nb = (char*)realloc(buffer, (size_t)buffer_size);
            if (!nb) {
                free(buffer);
                RETURNSTRX("-12");
            }
            buffer = nb;
        }

        unsigned long remaining_bytes = 0;
        if (socket_bytes_available(sockfd, &remaining_bytes) != 0) break;
        if (remaining_bytes == 0) break;
    }

    buffer[total_received] = '\0';
    RETURNSTRX(buffer);
    ENDPROC
}

/* ------------------------------------------------------------------------------------------------
 * Create a TCP server (bind+listen on IPv4 INADDR_ANY)
 * rxtcp.tcpserver(port, expose sockets=.string[]) -> server socket or negative error
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpserver) {
    int port = GETINT(ARG0);
    if (port <= 0 || port > 65535) RETURNINTX(-2);

    if (socket_init() != 0) {
        debugi("Socket initialization failed", socket_err());
        RETURNINTX(-1);
    }

    socket_t server = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_invalid(server)) {
        debugi("Could not create socket", socket_err());
        socket_cleanup();
        RETURNINTX(-2);
    }

    // allow quick restart
    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, (socklen_t)sizeof(opt));

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family      = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port        = htons((uint16_t)port);

    if (bind(server, (struct sockaddr*)&serveraddr, (socklen_t)sizeof(serveraddr)) == SOCKET_ERROR) {
        int e = socket_err();
        debugi("Bind failed", e);
        socket_close(server);
        socket_cleanup();
        RETURNINTX(-3);
    }

    if (listen(server, tcp_max_clients) == SOCKET_ERROR) {
        int e = socket_err();
        debugi("Listen failed", e);
        socket_close(server);
        socket_cleanup();
        RETURNINTX(-8);
    }

    char ipdetails[128];
    const char *ip = inet_ntoa(serveraddr.sin_addr); // "0.0.0.0" for INADDR_ANY
    snprintf(ipdetails, sizeof(ipdetails), "%ld %s %d",
             (long)(intptr_t)server, ip ? ip : "0.0.0.0", port);

    SETARRAYHI(ARG1, 1);
    SETSARRAY(ARG1, 0, ipdetails);

    RETURNINTX((intptr_t)server);
    ENDPROC
}

static socket_t parse_socket_from_entry(const char *entry) {
    if (!entry) return (socket_t)INVALID_SOCKET;
    // entry format: "<sock> <ip> <port>"
    char tmp[64];
    int i = 0;
    while (*entry && !isspace((unsigned char)*entry) && i < (int)sizeof(tmp)-1) tmp[i++] = *entry++;
    tmp[i] = '\0';
    if (i == 0) return (socket_t)INVALID_SOCKET;
    return (socket_t)(intptr_t)atoi(tmp);
}

static socket_t checkSocket(void *connections) {
    int i;
    socket_t maxfd = (socket_t)0;
    fd_set sockCheck;
    struct timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;

    for (i = 1; i < GETARRAYHI(connections); i++) { // start with 1; 0 is server socket
        const char *entry = GETSARRAY(connections, i);
        socket_t s = parse_socket_from_entry(entry);
        if (socket_invalid(s)) continue;

        FD_ZERO(&sockCheck);
        FD_SET(s, &sockCheck);

        int r = select(select_nfds(s), &sockCheck, NULL, NULL, &timeout);
        if (r == 0) {
            if (s > maxfd) maxfd = s;
            continue;
        } else {
            debugi("Socket invalid", (int)(intptr_t)s);
            REMOVEATTR(connections, i);
        }
    }
    return maxfd;
}

/* ------------------------------------------------------------------------------------------------
 * Wait for an incoming connection (server mode)
 * rxtcp.tcpwait(server,timeout_ms,expose sockets=.string[]) -> new socket or -4 timeout
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpwait) {
    socket_t server_socket = (socket_t)(intptr_t)GETINT(ARG0);
    int timeout_ms         = GETINT(ARG1);

    if (socket_invalid(server_socket)) RETURNINTX(-64);

    struct sockaddr_in client;
    socklen_t client_len = (socklen_t)sizeof(client);

    fd_set read_fds;
    struct timeval timeout;
    timeout_ms = RX_MAX(timeout_ms, 10);
    timeout.tv_sec  = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    socket_t maxfd = checkSocket(ARG2);
    if (server_socket > maxfd) maxfd = server_socket;

    FD_ZERO(&read_fds);
    FD_SET(server_socket, &read_fds);

    int activity = select(select_nfds(maxfd), &read_fds, NULL, NULL, &timeout);
    if (activity == 0) {
        debug("Timeout: no incoming connection");
        RETURNINTX(-4);
    }

    socket_t new_socket = accept(server_socket, (struct sockaddr*)&client, &client_len);
    if (socket_invalid(new_socket)) {
        debugi("Accept failed", socket_err());
        RETURNINTX(-socket_err());
    }

    char ipdetails[128];
    const char *cip = inet_ntoa(client.sin_addr);
    int cport = ntohs(client.sin_port);
    snprintf(ipdetails, sizeof(ipdetails), "%ld %s %d",
             (long)(intptr_t)new_socket, cip ? cip : "0.0.0.0", cport);

    int hi = GETARRAYHI(ARG2) + 1;
    SETARRAYHI(ARG2, hi);
    SETSARRAY(ARG2, hi - 1, ipdetails);

    RETURNINTX((intptr_t)new_socket);
    ENDPROC
}

/* ------------------------------------------------------------------------------------------------
 * Close TCP connection
 * rxtcp.tcpclose(socket) -> 0 or negative error
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(tcpclose) {
    socket_t sockfd = (socket_t)(intptr_t)GETINT(ARG0);
    if (socket_invalid(sockfd)) RETURNINTX(-64);

    socket_close(sockfd);
    socket_cleanup();

    debug("Socket closed");
    RETURNINTX(0);
    ENDPROC
}

/* ------------------------------------------------------------------------------------------------
 * Wait pauses execution (milliseconds)
 * rxtcp.wait(timeout_ms) -> 0
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(waitX) {
    int waittime = GETINT(ARG0);
    waittime = RX_MAX(waittime, 0);
    sleep_ms(waittime);
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
    int words = 0, i;

    if (!flags) flags = "";

    toUpperCase(flags);
    words = CountWords(flags);

    if (strstr(flags, "RESET")) {
        tcp_debug = 0;
        tcp_max_clients = 10;
        RETURNINT(0);
    }

    for (i = 1; i <= words; i++) {
        GetWord(word, flags, i);
        if (strcmp(word, "NDEBUG") == 0) tcp_debug = 0;
        else if (strcmp(word, "DEBUG") == 0) tcp_debug = 1;
        else if (strcmp(word, "MAXCLIENTS") == 0) {
            i++;
            GetWord(word, flags, i);
            tcp_max_clients = atoi(word);
            if (tcp_max_clients <= 0) tcp_max_clients = 10;
        }
    }

    RETURNINTX(0);
    ENDPROC
}

// RXTCP function definitions
LOADFUNCS
    ADDPROC(tcpflags,   "rxtcp.tcpflags",    "b", ".int",    "flags=.string");
    ADDPROC(waitX,      "rxtcp.wait",        "b", ".int",    "timeout=.int");

    ADDPROC(tcpopen,    "rxtcp.tcpopen",     "b", ".int",    "ip=.string,port=.int");
    ADDPROC(tcpsend,    "rxtcp.tcpsend",     "b", ".int",    "socket=.int,message=.string");
    ADDPROC(tcpreceive, "rxtcp.tcpreceive",  "b", ".string", "socket=.int,timeout=.int");
    ADDPROC(tcpclose,   "rxtcp.tcpclose",    "b", ".int",    "socket=.int");
    ADDPROC(tcpserver,  "rxtcp.tcpserver",   "b", ".int",    "port=.int,expose sockets=.string[]");
    ADDPROC(tcpwait,    "rxtcp.tcpwait",     "b", ".int",    "server=.int,timeout=.int,expose sockets=.string[]");
}
