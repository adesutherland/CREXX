//
// TCP Socket Client/Server Interface for crexx/pa
// with optional TLS (OpenSSL) client support
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#include <winsock2.h>
#else
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>        // fcntl(), F_GETFL, F_SETFL, O_NONBLOCK
#include <sys/ioctl.h>    // ioctl(), FIONREAD
#include <netinet/tcp.h>  // TCP_NODELAY
#endif

#include "crexxpa.h"  // your framework header

// --------------------------- TLS (OpenSSL) ---------------------------
#include <openssl/ssl.h>
#include <openssl/err.h>

static int ssl_global_init_done = 0;

static void ensure_openssl_init(void) {
    if (!ssl_global_init_done) {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        ssl_global_init_done = 1;
    }
}

static const char *ssl_err_to_str(SSL *ssl, int rc, char *buf, size_t bufsz) {
    int e = SSL_get_error(ssl, rc);
    switch (e) {
        case SSL_ERROR_WANT_READ:  snprintf(buf, bufsz, "SSL WANT_READ"); break;
        case SSL_ERROR_WANT_WRITE: snprintf(buf, bufsz, "SSL WANT_WRITE"); break;
        case SSL_ERROR_ZERO_RETURN:snprintf(buf, bufsz, "SSL closed cleanly"); break;
        case SSL_ERROR_SYSCALL:    snprintf(buf, bufsz, "SSL SYSCALL err=%d", errno); break;
        default: {
            unsigned long le = ERR_get_error();
            if (le) ERR_error_string_n(le, buf, (unsigned long)bufsz);
            else snprintf(buf, bufsz, "SSL error %d", e);
        }
    }
    return buf;
}

typedef struct {
#ifdef _WIN32
    SOCKET sock;
#else
    int sock;
#endif
    int status;
    int is_server;        // 1 = listening socket, 0 = client or accepted socket
    int default_timeout;  // ms, 0 = no timeout
    int last_error;
    char last_error_msg[128];
    char * linebuf;       // Line buffer for partial reads
    int linebuf_used;
    int linebuf_size;     // capacity

    // TLS
    int use_tls;          // 1 = TLS active
    SSL *ssl;             // TLS session
    SSL_CTX *ctx;         // TLS context
} TcpSocket;

static void set_error_with_detail(TcpSocket *s, const char *prefix, const char *detail) {
    size_t prefix_len;
    size_t detail_len;
    size_t detail_cap;

    if (!detail) detail = "";

    prefix_len = strlen(prefix);
    if (prefix_len >= sizeof(s->last_error_msg)) {
        memcpy(s->last_error_msg, prefix, sizeof(s->last_error_msg) - 1);
        s->last_error_msg[sizeof(s->last_error_msg) - 1] = 0;
        return;
    }

    memcpy(s->last_error_msg, prefix, prefix_len);
    detail_cap = sizeof(s->last_error_msg) - prefix_len - 1;
    detail_len = strlen(detail);
    if (detail_len > detail_cap) detail_len = detail_cap;

    memcpy(s->last_error_msg + prefix_len, detail, detail_len);
    s->last_error_msg[prefix_len + detail_len] = 0;
}
// --------------------------------------------------------------------

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
#define LINEBUF_INITIAL (8192)

static int ensure_linebuf(TcpSocket *s, int needfree) {
    int needed = s->linebuf_used + needfree;
    if (needed <= s->linebuf_size) return 0;
    int newsize = s->linebuf_size ? s->linebuf_size * 2 : 8192;
    while (newsize < needed) newsize *= 2;
    if (newsize > LINEBUF_MAX) return -1;
    char *p = (char*)realloc(s->linebuf, (size_t)newsize);
    if (!p) return -2;
    s->linebuf = p;
    s->linebuf_size = newsize;
    return 0;
}

// ----------------------------------------------------------------------------
// Create socket token (does not connect yet)
// ----------------------------------------------------------------------------
PROCEDURE(socketcreate) {
    TcpSocket *s = (TcpSocket*)malloc(sizeof(TcpSocket));
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
    s->linebuf = (char*)malloc(LINEBUF_INITIAL);
    s->linebuf_size = LINEBUF_INITIAL;
    s->linebuf_used = 0;
    s->is_server = 0;

    s->use_tls = 0;
    s->ssl = NULL;
    s->ctx = NULL;

    rxinteger r = (rxinteger)s;
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

    rc = connect(s->sock, res->ai_addr, (socklen_t)res->ai_addrlen);
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
    s->is_server = 0;
    RETURNINTX(0);
    ENDPROC
}

// ----------------------------------------------------------------------------
// Enable TLS (client-side). Arg1: hostname for SNI (optional but recommended)
// ----------------------------------------------------------------------------
PROCEDURE(socketenabletls) {
    rxinteger r = GETINT(ARG0);
    char *servername = GETSTRING(ARG1); // may be ""
    TcpSocket *s = (TcpSocket *)r;

    if (s->use_tls) {
        SET_SOCK_ERR(s, -200, "TLS already enabled");
        RETURNINTX(-200);
    }

    ensure_openssl_init();

    s->ctx = SSL_CTX_new(TLS_client_method());
    if (!s->ctx) {
        SET_SOCK_ERR(s, -201, "Failed to create SSL_CTX");
        RETURNINTX(-201);
    }

    // (Optional) You can set verification here later via SSL_CTX_set_verify(...)
    // For now, no CA bundle is loaded (no verification). Add a separate API if needed.

    s->ssl = SSL_new(s->ctx);
    if (!s->ssl) {
        SSL_CTX_free(s->ctx);
        s->ctx = NULL;
        SET_SOCK_ERR(s, -202, "Failed to create SSL object");
        RETURNINTX(-202);
    }

    if (servername && servername[0]) {
        // SNI + enables hostname-based selection in virtual-hosted servers
        SSL_set_tlsext_host_name(s->ssl, servername);
    }

    SSL_set_fd(s->ssl, s->sock);
   int rc=SSL_connect(s->ssl);
   if (rc <= 0) {
        ERR_print_errors_fp(stderr);
    } else {
        printf("TLS handshake complete with %s\n", SSL_get_cipher(s->ssl));
    }

    if (rc != 1) {
        char ebuf[128];
        ssl_err_to_str(s->ssl, rc, ebuf, sizeof(ebuf));
        set_error_with_detail(s, "TLS handshake failed: ", ebuf);
        s->last_error = -203;
        SSL_free(s->ssl);   s->ssl = NULL;
        SSL_CTX_free(s->ctx); s->ctx = NULL;
        RETURNINTX(-203);
    }

    s->use_tls = 1;
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
    int len = (int)strlen(data);
#ifdef _WIN32
    if (s->sock == INVALID_SOCKET) {
        SET_SOCK_ERR(s, -6, "Socket not connected");
        RETURNINTX(-6);
    }
#else
    if (s->sock < 0) {
        SET_SOCK_ERR(s, -6, "Socket not connected");
        RETURNINTX(-6);
    }
#endif

    int rc;
    if (s->use_tls) {
        rc = SSL_write(s->ssl, data, len);
        if (rc <= 0) {
            char ebuf[128]; ssl_err_to_str(s->ssl, rc, ebuf, sizeof(ebuf));
            set_error_with_detail(s, "SSL_write failed: ", ebuf);
            s->last_error = -3;
            RETURNINTX(-3);
        }
    } else {
#ifdef _WIN32
        rc = send(s->sock, data, len, 0);
        if (rc == SOCKET_ERROR) {
            int err = WSAGetLastError();
            snprintf(s->last_error_msg, sizeof(s->last_error_msg),
                     "Send failed, WSA error: %d", err);
            s->last_error = -3;
            RETURNINTX(-3);
        }
#else
        rc = (int)write(s->sock, data, (size_t)len);
        if (rc < 0) {
            snprintf(s->last_error_msg, sizeof(s->last_error_msg),
                     "Send failed, errno: %d", errno);
            s->last_error = -3;
            RETURNINTX(-3);
        }
#endif
    }
    s->last_error = 0;
    s->last_error_msg[0] = 0;
    RETURNINTX(rc);
    ENDPROC
}

// ----------------------------------------------------------------------------
// Send all data (loop until complete)
// ----------------------------------------------------------------------------
PROCEDURE(socketsendall) {
    rxinteger r = GETINT(ARG0);
    char *data = GETSTRING(ARG1);
    int len = (int)strlen(data);
    TcpSocket *s = (TcpSocket *)r;

#ifdef _WIN32
    if (s->sock == INVALID_SOCKET) {
        s->last_error = -6;
        strcpy(s->last_error_msg, "Socket not connected");
        RETURNINTX(-6);
    }
#else
    if (s->sock < 0) {
        s->last_error = -6;
        strcpy(s->last_error_msg, "Socket not connected");
        RETURNINTX(-6);
    }
#endif

    int total_sent = 0;
    while (total_sent < len) {
        int rc;
        if (s->use_tls) {
            rc = SSL_write(s->ssl, data + total_sent, len - total_sent);
            if (rc <= 0) {
                char ebuf[128]; ssl_err_to_str(s->ssl, rc, ebuf, sizeof(ebuf));
                set_error_with_detail(s, "SSL_write failed: ", ebuf);
                s->last_error = -3;
                RETURNINTX(-3);
            }
        } else {
#ifdef _WIN32
            rc = send(s->sock, data + total_sent, len - total_sent, 0);
            if (rc == SOCKET_ERROR) {
                int err = WSAGetLastError();
                snprintf(s->last_error_msg, sizeof(s->last_error_msg),
                         "Send failed, WSA error: %d", err);
                s->last_error = -3;
                RETURNINTX(-3);
            }
#else
            rc = (int)write(s->sock, data + total_sent, (size_t)(len - total_sent));
            if (rc < 0) {
                snprintf(s->last_error_msg, sizeof(s->last_error_msg),
                         "Send failed, errno: %d", errno);
                s->last_error = -3;
                RETURNINTX(-3);
            }
#endif
        }
        if (rc == 0) break; // shouldn't happen, but avoid loop
        total_sent += rc;
    }
    s->last_error = 0;
    s->last_error_msg[0] = 0;
    RETURNINTX(total_sent);
    ENDPROC
}

// ----------------------------------------------------------------------------
// Receive data (returns string, up to 4096 bytes requested)
// ----------------------------------------------------------------------------
PROCEDURE(socketrecv) {
    rxinteger r = GETINT(ARG0);
    int nbytes = GETINT(ARG1);
    TcpSocket *s = (TcpSocket *)r;

    char buffer[4096 + 1];

#ifdef _WIN32
    if (s->sock == INVALID_SOCKET) {
        SET_SOCK_ERR(s, -6, "Socket not connected");
        RETURNSTRX("");
    }
#else
    if (s->sock < 0) {
        SET_SOCK_ERR(s, -6, "Socket not connected");
        RETURNSTRX("");
    }
#endif

    int want = nbytes < (int)sizeof(buffer) ? nbytes : (int)sizeof(buffer) - 1;
    int rc = 0;

    if (s->use_tls) {
        SSL_set_mode(s->ssl, SSL_MODE_AUTO_RETRY);
        rc = SSL_read(s->ssl, buffer, want);
        fprintf(stderr, "SSL_read rc=%d\n", rc);

        if (rc <= 0) {
            int err = SSL_get_error(s->ssl, rc);

            if (err == SSL_ERROR_ZERO_RETURN) {
                // ✅ Clean TLS shutdown (EOF)
                SET_SOCK_ERR(s, 0, "TLS connection closed cleanly by remote");
                RETURNSTRX("");
            }
            else if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
                // 🔄 Non-blocking or timeout condition
                SET_SOCK_ERR(s, -7, "recv() timeout (TLS)");
                RETURNSTRX("");
            }
            else {
                // ❌ Real TLS or I/O error
                unsigned long ecode = ERR_get_error();
                const char *msg = ERR_reason_error_string(ecode);
                if (!msg) msg = "Unknown SSL error";

                char ebuf[256];
                snprintf(ebuf, sizeof(ebuf),
                         "SSL_read failed (code=%d, reason=%s)", err, msg);
                fprintf(stderr, "%s\n", ebuf);
                SET_SOCK_ERR(s, -5, ebuf);
                RETURNSTRX("");
            }
        }
    } else {
#ifdef _WIN32
        rc = recv(s->sock, buffer, want, 0);
        if (rc == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAETIMEDOUT) SET_SOCK_ERR(s, -7, "recv() timeout");
            else SET_SOCK_ERR(s, -5, "recv() failed");
            RETURNSTRX("");
        }
#else
        rc = (int)read(s->sock, (void *)buffer, (size_t)want);
        if (rc < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                SET_SOCK_ERR(s, -7, "recv() timeout");
            else
                SET_SOCK_ERR(s, -5, "read() failed");
            RETURNSTRX("");
        }
#endif
    }

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
// Receive single line (handles CRLF/LF, grows buffer as needed)
// ----------------------------------------------------------------------------
PROCEDURE(socketrecvline) {
    rxinteger r = GETINT(ARG0);
    TcpSocket *s = (TcpSocket *)r;
    char *newline;

    while (1) {
        // 1. Check for newline in the buffer first
        newline = (char*)memchr(s->linebuf, '\n', (size_t)s->linebuf_used);
        if (newline) {
            int linelen = (int)(newline - s->linebuf) + 1;
            char *result = (char*)malloc((size_t)linelen + 1);
            memcpy(result, s->linebuf, (size_t)linelen);
            result[linelen] = 0;
            // Move leftover to start of buffer
            int remaining = s->linebuf_used - linelen;
            if (remaining > 0) memmove(s->linebuf, s->linebuf + linelen, (size_t)remaining);
            s->linebuf_used = remaining;
            // Trim trailing \r\n
            int trimlen = linelen;
            while (trimlen > 0 && (result[trimlen - 1] == '\r' || result[trimlen - 1] == '\n'))
                result[--trimlen] = 0;
            s->last_error = 0;
            s->last_error_msg[0] = 0;
            RETURNSTRX(result);
            // (REXX runtime copies the string; we can free after RETURNSTRX if required,
            // but to be safe, we won't free result here.)
        }

        // 2. Buffer near full? Grow buffer
        if (s->linebuf_used >= s->linebuf_size - 4096) {
            if (ensure_linebuf(s, 4096) < 0) {
                SET_SOCK_ERR(s, -10, "Line too long for buffer");
                RETURNSTRX("");
            }
        }

        // 3. Need to read more data
#ifdef _WIN32
        int rc;
        if (s->use_tls) {
            rc = SSL_read(s->ssl, s->linebuf + s->linebuf_used,
                          s->linebuf_size - s->linebuf_used - 1);
            if (rc <= 0) {
                int e = SSL_get_error(s->ssl, rc);
                if (e == SSL_ERROR_WANT_READ || e == SSL_ERROR_WANT_WRITE) {
                    // Check if line already complete
                    newline = (char*)memchr(s->linebuf, '\n', (size_t)s->linebuf_used);
                    if (newline) continue;
                    SET_SOCK_ERR(s, -7, "recv() timeout (TLS)");
                    rc = 0;
                } else {
                    char ebuf[128]; ssl_err_to_str(s->ssl, rc, ebuf, sizeof(ebuf));
                    SET_SOCK_ERR(s, -5, ebuf);
                    RETURNSTRX("");
                }
            }
        } else {
            rc = recv(s->sock, s->linebuf + s->linebuf_used,
                      s->linebuf_size - s->linebuf_used - 1, 0);
            if (rc == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err == WSAETIMEDOUT) {
                    newline = (char*)memchr(s->linebuf, '\n', (size_t)s->linebuf_used);
                    if (newline) continue;
                    SET_SOCK_ERR(s, -7, "recv() timeout");
                    rc = 0;
                } else {
                    SET_SOCK_ERR(s, -5, "recv() failed or closed");
                    RETURNSTRX("");
                }
            }
        }
#else
        int rc;
        if (s->use_tls) {
            rc = SSL_read(s->ssl, s->linebuf + s->linebuf_used,
                          s->linebuf_size - s->linebuf_used - 1);
            if (rc <= 0) {
                int e = SSL_get_error(s->ssl, rc);
                if (e == SSL_ERROR_WANT_READ || e == SSL_ERROR_WANT_WRITE) {
                    newline = (char*)memchr(s->linebuf, '\n', (size_t)s->linebuf_used);
                    if (newline) continue;
                    SET_SOCK_ERR(s, -7, "recv() timeout (TLS)");
                    rc = 0;
                } else {
                    char ebuf[128]; ssl_err_to_str(s->ssl, rc, ebuf, sizeof(ebuf));
                    SET_SOCK_ERR(s, -5, ebuf);
                    RETURNSTRX("");
                }
            }
        } else {
            rc = (int)read(s->sock, s->linebuf + s->linebuf_used,
                           (size_t)(s->linebuf_size - s->linebuf_used - 1));
            if (rc < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    newline = (char*)memchr(s->linebuf, '\n', (size_t)s->linebuf_used);
                    if (newline) continue;
                    SET_SOCK_ERR(s, -7, "recv() timeout");
                    rc = 0;
                } else {
                    SET_SOCK_ERR(s, -5, "read() failed or closed");
                    RETURNSTRX("");
                }
            }
        }
#endif
        if (rc == 0) {
            // Connection closed OR timeout. If buffer has any data, return it as a "final line"
            if (s->linebuf_used > 0) {
                char *result = (char*)malloc((size_t)s->linebuf_used + 1);
                memcpy(result, s->linebuf, (size_t)s->linebuf_used);
                result[s->linebuf_used] = 0;
                s->linebuf_used = 0;
                int trimlen = (int)strlen(result);
                while (trimlen > 0 && (result[trimlen - 1] == '\r'))
                    result[--trimlen] = 0;
                s->last_error = 0;
                s->last_error_msg[0] = 0;
                RETURNSTRX(result);
            }
            SET_SOCK_ERR(s, 0, "Connection closed by remote or timeout");
            RETURNSTRX("");
        }
        s->linebuf_used += rc;
        // loop to check for newline again
    }
    ENDPROC
}

// ----------------------------------------------------------------------------
PROCEDURE(socketclose) {
    rxinteger r = GETINT(ARG0);
    TcpSocket *s = (TcpSocket *)r;
#ifdef _WIN32
    if (s->sock != INVALID_SOCKET) {
        // TLS cleanup first
        if (s->use_tls && s->ssl) {
            SSL_shutdown(s->ssl);
            SSL_free(s->ssl);
            s->ssl = NULL;
        }
        if (s->ctx) { SSL_CTX_free(s->ctx); s->ctx = NULL; }
        closesocket(s->sock);
    }
#else
    if (s->sock >= 0) {
        if (s->use_tls && s->ssl) {
            SSL_shutdown(s->ssl);
            SSL_free(s->ssl);
            s->ssl = NULL;
        }
        if (s->ctx) { SSL_CTX_free(s->ctx); s->ctx = NULL; }
        close(s->sock);
    }
#endif
    if (s->linebuf) free(s->linebuf);
    free(s);
    RETURNINTX(0);
    ENDPROC
}

// ----------------------------------------------------------------------------
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
    rc = (int)recv(s->sock, &tmp, 1, MSG_PEEK | MSG_DONTWAIT);
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

PROCEDURE(socketsettimeout) {
    rxinteger r = GETINT(ARG0);
    int ms = GETINT(ARG1);
    TcpSocket *s = (TcpSocket *)r;
    if (!s) RETURNINTX(-8);
    if (ms < 0) ms = 0;
    s->default_timeout = ms;
#ifdef _WIN32
    setsockopt(s->sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&ms, sizeof(ms));
    setsockopt(s->sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&ms, sizeof(ms));
#else
    struct timeval tv = { ms/1000, (ms%1000)*1000 };
    setsockopt(s->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(s->sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
    RETURNINTX(0);
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

PROCEDURE(socketsetblocking) {
    rxinteger r = GETINT(ARG0);
    int blocking = GETINT(ARG1);
    TcpSocket *s = (TcpSocket *)r;
    int rc = 0;
#ifdef _WIN32
    u_long mode = blocking ? 0 : 1;
    rc = ioctlsocket(s->sock, FIONBIO, &mode);
#else
    int flags = fcntl(s->sock, F_GETFL, 0);
    if (flags < 0) RETURNINTX(-1);
    if (blocking)  flags &= ~O_NONBLOCK;
    else           flags |= O_NONBLOCK;
    rc = fcntl(s->sock, F_SETFL, flags);
#endif
    RETURNINTX(rc == 0 ? 0 : -1);
    ENDPROC
}

PROCEDURE(socketpendingbytes) {
    rxinteger r = GETINT(ARG0);
    TcpSocket *s = (TcpSocket *)r;
    int count = 0;
#ifdef _WIN32
    ioctlsocket(s->sock, FIONREAD, (u_long *)&count);
#else
    ioctl(s->sock, FIONREAD, &count);
#endif
    RETURNINTX(count);
    ENDPROC
}

PROCEDURE(socketbind) {
    rxinteger r = GETINT(ARG0);
    char *ip = GETSTRING(ARG1);   // "" or "0.0.0.0" for any address
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

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = (ip && ip[0]) ? inet_addr(ip) : INADDR_ANY;

    int rc = bind(s->sock, (struct sockaddr *)&addr, sizeof(addr));
    if (rc != 0) {
        SET_SOCK_ERR(s, -30, "Bind failed");
#ifdef _WIN32
        closesocket(s->sock); s->sock = INVALID_SOCKET;
#else
        close(s->sock); s->sock = -1;
#endif
        RETURNINTX(-30);
    }
    s->is_server = 1;
    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(socketlisten) {
    rxinteger r = GETINT(ARG0);
    int backlog = GETINT(ARG1);
    TcpSocket *s = (TcpSocket *)r;
    int rc = listen(s->sock, backlog > 0 ? backlog : 5);
    if (rc != 0) {
        SET_SOCK_ERR(s, -31, "Listen failed");
        RETURNINTX(-31);
    }
    s->is_server = 1;
    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(socketaccept) {
    rxinteger r = GETINT(ARG0);
    TcpSocket *s = (TcpSocket *)r;

    struct sockaddr_in addr;
#ifdef _WIN32
    int addrlen = sizeof(addr);
    SOCKET cs = accept(s->sock, (struct sockaddr*)&addr, &addrlen);
    if (cs == INVALID_SOCKET) {
        SET_SOCK_ERR(s, -32, "Accept failed");
        RETURNINTX(-32);
    }
#else
    socklen_t addrlen = sizeof(addr);
    int cs = (int)accept(s->sock, (struct sockaddr*)&addr, &addrlen);
    if (cs < 0) {
        SET_SOCK_ERR(s, -32, "Accept failed");
        RETURNINTX(-32);
    }
#endif
    // Create new TcpSocket token for the client; copy settings
    TcpSocket *client = (TcpSocket*)malloc(sizeof(TcpSocket));
    if (!client) RETURNINTX(-8);
    memcpy(client, s, sizeof(TcpSocket)); // copies default_timeout, etc.
    client->sock = cs;
    client->is_server = 0;
    client->linebuf = (char*)malloc(LINEBUF_INITIAL);
    client->linebuf_size = LINEBUF_INITIAL;
    client->linebuf_used = 0;

    // TLS is not active on accepted sockets by default (server TLS not implemented)
    client->use_tls = 0;
    client->ssl = NULL;
    client->ctx = NULL;

    rxinteger cr = (rxinteger)client;
    RETURNINTX(cr);
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
    ADDPROC(socketcreate,          "socket.socketcreate",          "b", ".int", "");
    ADDPROC(socketconnect,         "socket.socketconnect",         "b", ".int", "sock=.int,host=.string,port=.int");
    ADDPROC(socketsend,            "socket.socketsend",            "b", ".int", "sock=.int,data=.string");
    ADDPROC(socketsendall,         "socket.socketsendall",         "b", ".int", "sock=.int,data=.string");
    ADDPROC(socketrecv,            "socket.socketrecv",            "b", ".string", "sock=.int,size=.int");
    ADDPROC(socketrecvline,        "socket.socketrecvline",        "b", ".string", "sock=.int");
    ADDPROC(socketclose,           "socket.socketclose",           "b", ".int", "sock=.int");
    ADDPROC(socketsetblocking,     "socket.socketsetblocking",     "b", ".int", "sock=.int,blocking=.int");
    ADDPROC(socketpendingbytes,    "socket.socketpendingbytes",    "b", ".int", "sock=.int");
    ADDPROC(socketisconnected,     "socket.socketisconnected",     "b", ".int", "sock=.int");
    ADDPROC(socketpeerinfo,        "socket.socketpeerinfo",        "b", ".string", "sock=.int");
    ADDPROC(socketlocalinfo,       "socket.socketlocalinfo",       "b", ".string", "sock=.int");
    ADDPROC(socketsettimeout,      "socket.socketsettimeout",      "b", ".int", "sock=.int,timeout=.int");
    ADDPROC(socketshutdown,        "socket.socketshutdown",        "b", ".int", "sock=.int,how=.int");
    ADDPROC(socketlasterror,       "socket.socketlasterror",       "b", ".string", "sock=.int");
    ADDPROC(socketnodelay,         "socket.socketnodelay",         "b", ".int", "sock=.int,enable=.int");
    ADDPROC(socketkeepalive,       "socket.socketkeepalive",       "b", ".int", "sock=.int,enable=.int");
    ADDPROC(socketbind,            "socket.socketbind",            "b", ".int", "sock=.int,ip=.string,port=.int");
    ADDPROC(socketlisten,          "socket.socketlisten",          "b", ".int", "sock=.int,backlog=.int");
    ADDPROC(socketaccept,          "socket.socketaccept",          "b", ".int", "sock=.int");

// New TLS procedure (client-side)
    ADDPROC(socketenabletls,       "socket.socketenabletls",       "b", ".int", "sock=.int,hostname=.string");
ENDLOADFUNCS
