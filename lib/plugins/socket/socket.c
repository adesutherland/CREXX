//
// TCP Socket Client Interface for crexx/pa
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif


#include "crexxpa.h"  // your framework header

typedef struct {
#ifdef _WIN32
    SOCKET sock;
#else
    int sock;
#endif
    int status;
    int default_timeout;   // ms, 0 = no timeout
    int last_error;
    char last_error_msg[128];
    char * linebuf;  // Line buffer for partial reads
    int linebuf_used;
    int linebuf_size; // capacity
} TcpSocket;


#define SET_SOCK_ERR(s, code, msg) do { \
    (s)->last_error = (code); \
    strncpy((s)->last_error_msg, (msg), sizeof((s)->last_error_msg)-1); \
    (s)->last_error_msg[sizeof((s)->last_error_msg)-1] = 0; \
} while(0)

#ifdef _WIN32
typedef DWORD socktimeout_t;
#else
typedef struct timeval socktimeout_t;
#endif

#define LINEBUF_MAX (4*1024*1024) // 4 MB safety

int ensure_linebuf(TcpSocket *s, int needfree) {
    int needed = s->linebuf_used + needfree;
    if (needed <= s->linebuf_size) return 0;
    int newsize = s->linebuf_size ? s->linebuf_size * 2 : 8192;
    while (newsize < needed) newsize *= 2;
    if (newsize > LINEBUF_MAX) return -1;
    char *p = realloc(s->linebuf, newsize);
    if (!p) return -2;
    s->linebuf = p;
    s->linebuf_size = newsize;
    return 0;
}

// ----------------------------------------------------------------------------
// Create socket token (does not connect yet)
// ----------------------------------------------------------------------------
#define LINEBUF_INITIAL (8192)
PROCEDURE(socketcreate) {
        TcpSocket *s = malloc(sizeof(TcpSocket));
        if (!s) RETURNINTX(-8);
#ifdef _WIN32
        static int wsa_init = 0;
    if (!wsa_init) { WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa); wsa_init = 1; }
    s->sock = INVALID_SOCKET;
#else
        s->sock = -1;
#endif
    s->status = 0;
    s->default_timeout = 0;
    s->last_error = 0;
    s->last_error_msg[0] = 0;
    rxinteger r = (rxinteger)s;
    s->linebuf = malloc(LINEBUF_INITIAL );
    s->linebuf_size = LINEBUF_INITIAL ;
    s->linebuf_used = 0;

    RETURNINTX(r);
    ENDPROC
}

// ----------------------------------------------------------------------------
// Connect to TCP server (host, port)
// ----------------------------------------------------------------------------
PROCEDURE(socketconnect) {
    rxinteger r = GETINT(ARG0);
    char *host = GETSTRING(ARG1);
    int port = GETINT(ARG2);
    TcpSocket *s = (TcpSocket *)r;

#ifdef _WIN32
    s->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (s->sock == INVALID_SOCKET) {
        SET_SOCK_ERR(s, -1, "Cannot create socket");
        RETURNINTX(-1);
    }
#else
    s->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (s->sock < 0) {
        SET_SOCK_ERR(s, -1, "Cannot create socket");
        RETURNINTX(-1);
    }
#endif

    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);

    int rc = getaddrinfo(host, portstr, &hints, &res);
    if (rc != 0 || !res) {
#ifdef _WIN32
        closesocket(s->sock); s->sock = INVALID_SOCKET;
#else
        close(s->sock); s->sock = -1;
#endif
        SET_SOCK_ERR(s, -4, "Hostname resolution failed");
        RETURNINTX(-4);
    }

    rc = connect(s->sock, res->ai_addr, res->ai_addrlen);
#ifdef _WIN32
    if (rc == SOCKET_ERROR) {
        int err = WSAGetLastError();
        snprintf(s->last_error_msg, sizeof(s->last_error_msg),
                 "Connect failed, WSA error: %d", err);
        s->last_error = -2;
        closesocket(s->sock); s->sock = INVALID_SOCKET;
        freeaddrinfo(res);
        RETURNINTX(-2);
    }
#else
    if (rc != 0) {
        snprintf(s->last_error_msg, sizeof(s->last_error_msg),
                 "Connect failed, errno: %d", errno);
        s->last_error = -2;
        close(s->sock); s->sock = -1;
        freeaddrinfo(res);
        RETURNINTX(-2);
    }
#endif
    freeaddrinfo(res);
    s->status = 1;
    s->last_error = 0;
    s->last_error_msg[0] = 0;
    RETURNINTX(0);
    ENDPROC
}

// ----------------------------------------------------------------------------
// Send data (string) to socket
// ----------------------------------------------------------------------------
PROCEDURE(socketsend) {
    rxinteger r = GETINT(ARG0);
    TcpSocket *s = (TcpSocket *)r;
    char *data = GETSTRING(ARG1);
    int len = strlen(data);
    if (s->sock < 0) {
        SET_SOCK_ERR(s, -6, "Socket not connected");
        RETURNINTX(-6);
    }
#ifdef _WIN32
    int rc = send(s->sock, data, len, 0);
    if (rc == SOCKET_ERROR) {
        int err = WSAGetLastError();
        snprintf(s->last_error_msg, sizeof(s->last_error_msg),
                 "Send failed, WSA error: %d", err);
        s->last_error = -3;
        RETURNINTX(-3);
    }
#else
    int rc = write(s->sock, data, len);
    if (rc < 0) {
        snprintf(s->last_error_msg, sizeof(s->last_error_msg),
                 "Send failed, errno: %d", errno);
        s->last_error = -3;
        RETURNINTX(-3);
    }
#endif
    s->last_error = 0;
    s->last_error_msg[0] = 0;
    RETURNINTX(rc);
    ENDPROC
}

// ----------------------------------------------------------------------------
// Receive data (returns string, up to 4096 bytes)
// ----------------------------------------------------------------------------
PROCEDURE(socketrecv) {
    rxinteger r = GETINT(ARG0);
    int nbytes = GETINT(ARG1);

    TcpSocket *s = (TcpSocket *)r;

    char buffer[4096];
    if (s->sock < 0) {
        SET_SOCK_ERR(s, -6, "Socket not connected");
        RETURNSTRX("");
    }
#ifdef _WIN32
    int rc = recv(s->sock, buffer, nbytes < sizeof(buffer) ? nbytes : sizeof(buffer), 0);

    int err = WSAGetLastError();
    if (rc == SOCKET_ERROR) {
        if (err == WSAETIMEDOUT) SET_SOCK_ERR(s, -7, "recv() timeout");
        else SET_SOCK_ERR(s, -5, "recv() failed");
        RETURNSTRX("");
    }
    snprintf(s->last_error_msg, sizeof(s->last_error_msg),
                 "Recv failed, WSA error: %d", err);
    s->last_error = -5;
    RETURNSTRX("");
#else
    int rc = read(s->sock, buf, nbytes < sizeof(buf) ? nbytes : sizeof(buf));
    if (rc < 0) {
       if (errno == EWOULDBLOCK || errno == EAGAIN) SET_SOCK_ERR(s, -7, "recv() timeout");
       else SET_SOCK_ERR(s, -5, "read() failed");
       RETURNSTRX("")
    }
#endif
    if (rc == 0) {
        SET_SOCK_ERR(s, 0, "Connection closed by remote");
        RETURNSTRX("");
    }
    buffer[rc] = 0;
    s->last_error = 0;
    s->last_error_msg[0] = 0;
    RETURNSTRX(buffer);
    ENDPROC
}
// ----------------------------------------------------------------------------
// Receive single line
// ----------------------------------------------------------------------------

PROCEDURE(socketrecvline) {
    rxinteger r = GETINT(ARG0);
    TcpSocket *s = (TcpSocket *)r;
    char *newline;

    while (1) {
        // 1. Always check for newline in the buffer first!
        newline = memchr(s->linebuf, '\n', s->linebuf_used);
        if (newline) {
            int linelen = (int)(newline - s->linebuf) + 1;
            char *result = malloc(linelen + 1);
            memcpy(result, s->linebuf, linelen);
            result[linelen] = 0;
            // Move leftover to start of buffer
            int remaining = s->linebuf_used - linelen;
            if (remaining > 0) memmove(s->linebuf, s->linebuf + linelen, remaining);
            s->linebuf_used = remaining;
            // Trim trailing \r\n
            int trimlen = linelen;
            while (trimlen > 0 && (result[trimlen - 1] == '\r' || result[trimlen - 1] == '\n'))
                result[--trimlen] = 0;
            s->last_error = 0;
            s->last_error_msg[0] = 0;
            RETURNSTRX(result);
            free(result);
        }

        // 2. Buffer full, but still no newline? Grow buffer!
        if (s->linebuf_used >= s->linebuf_size - 4096) {
            if (ensure_linebuf(s, 4096) < 0) {
                SET_SOCK_ERR(s, -10, "Line too long for buffer");
                RETURNSTRX("");
            }
        }

        // 3. Need to read more data
#ifdef _WIN32
        int rc = recv(s->sock, s->linebuf + s->linebuf_used,
                      s->linebuf_size - s->linebuf_used - 1, 0);
        int err = WSAGetLastError();
        if (rc == SOCKET_ERROR) {
            if (err == WSAETIMEDOUT) {
                newline = memchr(s->linebuf, '\n', s->linebuf_used);
                if (newline) continue;
                SET_SOCK_ERR(s, -7, "recv() timeout");
                rc = 0;
            } else {
                SET_SOCK_ERR(s, -5, "recv() failed or closed");
                RETURNSTRX("");
            }
        }
#else
        int rc = read(s->sock, s->linebuf + s->linebuf_used,
                      s->linebuf_size - s->linebuf_used - 1);
        if (rc < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                newline = memchr(s->linebuf, '\n', s->linebuf_used);
                if (newline) continue;
                SET_SOCK_ERR(s, -7, "recv() timeout");
                rc = 0;
            } else {
                SET_SOCK_ERR(s, -5, "read() failed or closed");
                RETURNSTRX("");
            }
        }
#endif
        if (rc == 0) {
            // Connection closed. If buffer has any data, return it as a "final line"
            if (s->linebuf_used > 0) {
                char *result = malloc(s->linebuf_used + 1);
                memcpy(result, s->linebuf, s->linebuf_used);
                result[s->linebuf_used] = 0;
                s->linebuf_used = 0;
                int trimlen = (int)strlen(result);
                while (trimlen > 0 && (result[trimlen - 1] == '\r'))
                    result[--trimlen] = 0;
                s->last_error = 0;
                s->last_error_msg[0] = 0;
                free(result);
                RETURNSTRX(result);
            }
            SET_SOCK_ERR(s, 0, "Connection closed by remote");
            RETURNSTRX("");
        }
        s->linebuf_used += rc;
        // Go back to top of loop to check for newlines!
    }
    ENDPROC
}

// ----------------------------------------------------------------------------
// Close socket and free token
// ----------------------------------------------------------------------------
PROCEDURE(socketclose) {
    rxinteger r = GETINT(ARG0);
    TcpSocket *s = (TcpSocket *)r;
#ifdef _WIN32
    if (s->sock != INVALID_SOCKET) closesocket(s->sock);
#else
    if (s->sock >= 0) close(s->sock);
#endif
    if (s->linebuf) free(s->linebuf);
    free(s);
    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(socketisconnected) {
    rxinteger r = GETINT(ARG0);
    TcpSocket *s = (TcpSocket *)r;
    if (!s) RETURNINTX(0);

#ifdef _WIN32
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(s->sock, &readfds);
    struct timeval tv = {0, 0}; // Zero timeout
    int rc = select(0, &readfds, NULL, NULL, &tv);
    if (rc < 0) RETURNINTX(0); // Error
    if (rc == 0) RETURNINTX(1); // No data, but socket not closed (still connected)
    // If there is data, we can try recv to see if socket closed
    char tmp;
    rc = recv(s->sock, &tmp, 1, MSG_PEEK);
    if (rc == 0) RETURNINTX(0); // Disconnected
    RETURNINTX(1);
#else
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(s->sock, &readfds);
    struct timeval tv = {0, 0};
    int rc = select(s->sock+1, &readfds, NULL, NULL, &tv);
    if (rc < 0) RETURNINTX(0);
    if (rc == 0) RETURNINTX(1); // No data, but still connected
    char tmp;
    rc = recv(s->sock, &tmp, 1, MSG_PEEK | MSG_DONTWAIT);
    if (rc == 0) RETURNINTX(0); // Disconnected
    RETURNINTX(1);
#endif
    ENDPROC
}

PROCEDURE(socketpeerinfo) {
    rxinteger r = GETINT(ARG0);
    TcpSocket *s = (TcpSocket *)r;
    char buf[128] = {0};
#ifdef _WIN32
    struct sockaddr_in addr;
    int len = sizeof(addr);
    if (getpeername(s->sock, (struct sockaddr *)&addr, &len) == 0) {
        snprintf(buf, sizeof(buf), "%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        RETURNSTRX(buf);
    }
#else
        struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    if (getpeername(s->sock, (struct sockaddr *)&addr, &len) == 0) {
        snprintf(buf, sizeof(buf), "%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        RETURNSTRX(buf);
    }
#endif
    RETURNSTRX("");
    ENDPROC
}

PROCEDURE(socketsettimeout) {
    rxinteger r = GETINT(ARG0);
    int ms = GETINT(ARG1);
    TcpSocket *s = (TcpSocket *)r;
    if (!s) RETURNINTX(-8);
    if (ms < 0) ms = 0;
    s->default_timeout = ms;
#ifdef _WIN32
    setsockopt(s->sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&ms, sizeof(ms));
#else
    struct timeval tv = { ms/1000, (ms%1000)*1000 };
    setsockopt(s->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(socketlocalinfo) {
    rxinteger r = GETINT(ARG0);
    TcpSocket *s = (TcpSocket *)r;
    char buf[128] = {0};
#ifdef _WIN32
    struct sockaddr_in addr;
    int len = sizeof(addr);
    if (getsockname(s->sock, (struct sockaddr *)&addr, &len) == 0) {
        snprintf(buf, sizeof(buf), "%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        RETURNSTRX(buf);
    }
#else
        struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    if (getsockname(s->sock, (struct sockaddr *)&addr, &len) == 0) {
        snprintf(buf, sizeof(buf), "%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        RETURNSTRX(buf);
    }
#endif
    RETURNSTRX("");
    ENDPROC
}

PROCEDURE(socketshutdown) {
    rxinteger r = GETINT(ARG0);
    int how = GETINT(ARG1);  // 0=receive, 1=send, 2=both (standard shutdown values)
    TcpSocket *s = (TcpSocket *)r;

    int rc = -1;
#ifdef _WIN32
    rc = shutdown(s->sock, how);
#else
    rc = shutdown(s->sock, how);
#endif
    if (rc == 0) {
        s->last_error = 0;
        s->last_error_msg[0] = 0;
        RETURNINTX(0);
    } else {
        s->last_error = -20;
        snprintf(s->last_error_msg, sizeof(s->last_error_msg), "shutdown() failed");
        RETURNINTX(-20);
    }
    ENDPROC
}
PROCEDURE(socketnodelay) {
    rxinteger r = GETINT(ARG0);
    int enable = GETINT(ARG1);  // 1 = ON, 0 = OFF
    TcpSocket *s = (TcpSocket *)r;
    int flag = (enable ? 1 : 0);
    int rc =
#ifdef _WIN32
            setsockopt(s->sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag));
#else
    setsockopt(s->sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
#endif
    if (rc == 0) {
        s->last_error = 0;
        s->last_error_msg[0] = 0;
        RETURNINTX(0);
    } else {
        s->last_error = -21;
        snprintf(s->last_error_msg, sizeof(s->last_error_msg), "setsockopt TCP_NODELAY failed");
        RETURNINTX(-21);
    }
    ENDPROC
}
PROCEDURE(socketkeepalive) {
    rxinteger r = GETINT(ARG0);
    int enable = GETINT(ARG1);  // 1 = ON, 0 = OFF
    TcpSocket *s = (TcpSocket *)r;
    int flag = (enable ? 1 : 0);
    int rc =
#ifdef _WIN32
            setsockopt(s->sock, SOL_SOCKET, SO_KEEPALIVE, (const char*)&flag, sizeof(flag));
#else
    setsockopt(s->sock, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag));
#endif
    if (rc == 0) {
        s->last_error = 0;
        s->last_error_msg[0] = 0;
        RETURNINTX(0);
    } else {
        s->last_error = -22;
        snprintf(s->last_error_msg, sizeof(s->last_error_msg), "setsockopt KEEPALIVE failed");
        RETURNINTX(-22);
    }
    ENDPROC
}



// ----------------------------------------------------------------------------
// Report last error
// ----------------------------------------------------------------------------

PROCEDURE(socketlasterror) {
    rxinteger r = GETINT(ARG0);
    TcpSocket *s = (TcpSocket *)r;
    char buf[160];
    snprintf(buf, sizeof(buf), "%d %s", s->last_error, s->last_error_msg);
    RETURNSTRX(buf);
    ENDPROC
}


// ----------------------------------------------------------------------------
// Registration Table (for ADDPROC system)
// ----------------------------------------------------------------------------
LOADFUNCS
ADDPROC(socketcreate,  "socket.socketcreate",  "b",    ".int" ,"");
ADDPROC(socketconnect, "socket.socketconnect", "b",    ".int" ,"sock=.int,host=.string,port=.int");
ADDPROC(socketsend,    "socket.socketsend",    "b",    ".int" ,"sock=.int,data=.string");
ADDPROC(socketrecv,    "socket.socketrecv",    "b",    ".string","sock=.int,size=.int");
ADDPROC(socketrecvline,"socket.socketrecvline", "b",   ".string", "sock=.int");
ADDPROC(socketclose,   "socket.socketclose",    "b",   ".int" ,"sock=.int");
ADDPROC(socketisconnected, "socket.socketisconnected", "b", ".int", "sock=.int");
ADDPROC(socketpeerinfo,    "socket.socketpeerinfo",    "b", ".string", "sock=.int");
ADDPROC(socketpeerinfo,    "socket.socketlocalinfo",   "b", ".string", "sock=.int");
ADDPROC(socketsettimeout, "socket.socketsettimeout",    "b", ".int", "sock=.int,timeout=.int");
ADDPROC(socketshutdown, "socket.socketshutdown", "b", ".int", "sock=.int,how=.int");
ADDPROC(socketlasterror,"socket.socketlasterror","b",  ".string","sock=.int");
ADDPROC(socketnodelay,  "socket.socketnodelay",   "b", ".int", "sock=.int,enable=.int");
ADDPROC(socketkeepalive,"socket.socketkeepalive", "b", ".int", "sock=.int,enable=.int");

ENDLOADFUNCS