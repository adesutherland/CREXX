/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
typedef SOCKET rxvm_socket_fd;
#define RXVM_SOCKET_INVALID INVALID_SOCKET
#else
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
typedef int rxvm_socket_fd;
#define RXVM_SOCKET_INVALID (-1)
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#endif

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif
#ifndef NI_MAXSERV
#define NI_MAXSERV 32
#endif

#include "rxvmintp.h"
#include "rxvmvars.h"
#include "rxvmsock.h"

#ifdef CREXX_TLS_OPENSSL
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509_vfy.h>
#endif

#ifdef CREXX_TLS_NETWORK
#include <dispatch/dispatch.h>
#include <Network/Network.h>
#include <Security/SecBase.h>
#include <Security/SecProtocolOptions.h>
#endif

#ifdef CREXX_TLS_SCHANNEL
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif
#include <security.h>
#include <schannel.h>
#include <wincrypt.h>

#ifndef UNISP_NAME_W
#define UNISP_NAME_W L"Microsoft Unified Security Protocol Provider"
#endif
#ifndef SCH_CRED_MANUAL_CRED_VALIDATION
#define SCH_CRED_MANUAL_CRED_VALIDATION 0x00000008
#endif
#ifndef SCH_CRED_NO_DEFAULT_CREDS
#define SCH_CRED_NO_DEFAULT_CREDS 0x00000010
#endif
#ifndef SCH_USE_STRONG_CRYPTO
#define SCH_USE_STRONG_CRYPTO 0x00400000
#endif
#endif

#define RXSOCK_OK 0
#define RXSOCK_EOF 1
#define RXSOCK_TIMEOUT 2
#define RXSOCK_WOULDBLOCK 3
#define RXSOCK_ERR_INVALID_HANDLE (-1)
#define RXSOCK_ERR_NO_MEMORY (-2)
#define RXSOCK_ERR_OS (-3)
#define RXSOCK_ERR_DNS (-4)
#define RXSOCK_ERR_ARGUMENT (-5)
#define RXSOCK_ERR_NOT_OPEN (-6)
#define RXSOCK_ERR_TLS_UNAVAILABLE (-7)
#define RXSOCK_ERR_TLS_STATE (-8)
#define RXSOCK_ERR_TLS_HANDSHAKE (-9)
#define RXSOCK_ERR_TLS_VERIFY (-10)

typedef struct rxvm_socket_entry {
    rxinteger handle;
    rxvm_socket_fd fd;
    int in_use;
    int is_server;
    int blocking;
    int timeout_ms;
    rxinteger last_status;
    int last_errno;
    char last_error[192];
    char *connect_host;
    rxinteger connect_port;
#ifdef CREXX_TLS_OPENSSL
    SSL_CTX *tls_ctx;
    SSL *tls_ssl;
#endif
#ifdef CREXX_TLS_NETWORK
    nw_connection_t nw_conn;
    dispatch_queue_t nw_queue;
#endif
#ifdef CREXX_TLS_SCHANNEL
    CredHandle schannel_cred;
    CtxtHandle schannel_ctx;
    SecPkgContext_StreamSizes schannel_sizes;
    int schannel_have_cred;
    int schannel_have_ctx;
    char *schannel_encrypted;
    size_t schannel_encrypted_len;
    char *schannel_decrypted;
    size_t schannel_decrypted_len;
    size_t schannel_decrypted_pos;
#endif
    int tls_active;
} rxvm_socket_entry;

typedef struct rxvm_socket_registry {
    rxvm_socket_entry *entries;
    size_t count;
    size_t capacity;
    rxinteger next_handle;
#ifdef _WIN32
    int wsa_started;
#endif
} rxvm_socket_registry;

static int rxvm_socket_is_invalid(rxvm_socket_fd fd) {
#ifdef _WIN32
    return fd == INVALID_SOCKET;
#else
    return fd < 0;
#endif
}

static int rxvm_socket_last_errno(void) {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

static void rxvm_socket_close_fd(rxvm_socket_fd fd) {
    if (rxvm_socket_is_invalid(fd)) return;
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}

static void rxvm_socket_entry_ok(rxvm_socket_entry *entry) {
    if (!entry) return;
    entry->last_status = RXSOCK_OK;
    entry->last_errno = 0;
    strcpy(entry->last_error, "0 OK");
}

static void rxvm_socket_entry_status(rxvm_socket_entry *entry,
                                     rxinteger status,
                                     int os_error,
                                     const char *message) {
    if (!entry) return;
    entry->last_status = status;
    entry->last_errno = os_error;
    if (!message) message = "";
    snprintf(entry->last_error, sizeof(entry->last_error), "%ld %s",
             (long)status, message);
}

static void rxvm_socket_entry_os_error(rxvm_socket_entry *entry,
                                       rxinteger status,
                                       const char *prefix) {
    int err = rxvm_socket_last_errno();
    char buffer[160];

    if (!prefix) prefix = "socket error";
#ifdef _WIN32
    snprintf(buffer, sizeof(buffer), "%s (WSA %d)", prefix, err);
#else
    snprintf(buffer, sizeof(buffer), "%s (%s)", prefix, strerror(err));
#endif
    rxvm_socket_entry_status(entry, status, err, buffer);
}

#ifdef CREXX_TLS_OPENSSL
static void rxvm_socket_entry_tls_error(rxvm_socket_entry *entry,
                                        rxinteger status,
                                        SSL *ssl,
                                        int rc,
                                        const char *prefix) {
    char detail[128];
    char buffer[176];
    unsigned long openssl_error;
    int ssl_error = ssl ? SSL_get_error(ssl, rc) : SSL_ERROR_SSL;

    openssl_error = ERR_get_error();
    if (openssl_error) {
        ERR_error_string_n(openssl_error, detail, sizeof(detail));
    } else if (ssl_error == SSL_ERROR_WANT_READ) {
        snprintf(detail, sizeof(detail), "operation would block waiting for TLS read");
    } else if (ssl_error == SSL_ERROR_WANT_WRITE) {
        snprintf(detail, sizeof(detail), "operation would block waiting for TLS write");
    } else if (ssl_error == SSL_ERROR_ZERO_RETURN) {
        snprintf(detail, sizeof(detail), "TLS connection closed");
    } else if (ssl_error == SSL_ERROR_SYSCALL) {
#ifdef _WIN32
        snprintf(detail, sizeof(detail), "TLS syscall error (WSA %d)", rxvm_socket_last_errno());
#else
        snprintf(detail, sizeof(detail), "TLS syscall error (%s)", strerror(rxvm_socket_last_errno()));
#endif
    } else {
        snprintf(detail, sizeof(detail), "TLS error %d", ssl_error);
    }

    snprintf(buffer, sizeof(buffer), "%s: %s", prefix ? prefix : "TLS error", detail);
    rxvm_socket_entry_status(entry, status, 0, buffer);
}

static void rxvm_socket_entry_tls_verify_error(rxvm_socket_entry *entry,
                                               long verify_result) {
    char buffer[176];

    snprintf(buffer, sizeof(buffer), "TLS certificate verification failed: %s",
             X509_verify_cert_error_string(verify_result));
    rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_VERIFY, 0, buffer);
}

static int rxvm_socket_tls_hostname_is_ip(const char *host) {
    struct addrinfo hints;
    struct addrinfo *res = 0;
    int rc;

    if (!host || !host[0]) return 0;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST;
    rc = getaddrinfo(host, 0, &hints, &res);
    if (res) freeaddrinfo(res);
    return rc == 0;
}

static int rxvm_socket_tls_set_peer_name(SSL *ssl, const char *host) {
    if (!ssl || !host || !host[0]) return 0;
    if (SSL_set_tlsext_host_name(ssl, host) != 1) return 0;
    if (rxvm_socket_tls_hostname_is_ip(host)) {
        return X509_VERIFY_PARAM_set1_ip_asc(SSL_get0_param(ssl), host) == 1;
    }
    return SSL_set1_host(ssl, host) == 1;
}
#endif

#ifdef CREXX_TLS_NETWORK
typedef struct rxvm_socket_network_wait {
    dispatch_semaphore_t done;
    nw_error_t error;
    int complete;
    int ready;
} rxvm_socket_network_wait;

typedef struct rxvm_socket_network_recv_state {
    dispatch_semaphore_t done;
    nw_error_t error;
    size_t copied;
    int complete;
    int stream_complete;
} rxvm_socket_network_recv_state;

static dispatch_time_t rxvm_socket_network_deadline(rxvm_socket_entry *entry) {
    if (entry && entry->timeout_ms > 0) {
        return dispatch_time(DISPATCH_TIME_NOW, (int64_t)entry->timeout_ms * NSEC_PER_MSEC);
    }
    return DISPATCH_TIME_FOREVER;
}

static int rxvm_socket_network_wait_for(rxvm_socket_entry *entry,
                                        dispatch_semaphore_t done) {
    if (dispatch_semaphore_wait(done, rxvm_socket_network_deadline(entry)) == 0) return 0;
    return -1;
}

static int rxvm_socket_network_tls_code_is_verify(int code) {
    switch (code) {
        case errSSLXCertChainInvalid:
        case errSSLBadCert:
        case errSSLUnknownRootCert:
        case errSSLNoRootCert:
        case errSSLCertExpired:
        case errSSLCertNotYetValid:
        case errSSLPeerBadCert:
        case errSSLPeerUnsupportedCert:
        case errSSLPeerCertRevoked:
        case errSSLPeerCertExpired:
        case errSSLPeerCertUnknown:
        case errSSLPeerUnknownCA:
        case errSSLHostNameMismatch:
        case errSSLATSCertificateTrustViolation:
            return 1;
        default:
            return 0;
    }
}

static rxinteger rxvm_socket_network_error_status(nw_error_t error,
                                                  rxinteger fallback) {
    nw_error_domain_t domain;
    int code;

    if (!error) return fallback;
    domain = nw_error_get_error_domain(error);
    code = nw_error_get_error_code(error);
    if (domain == nw_error_domain_posix) {
        if (code == ETIMEDOUT) return RXSOCK_TIMEOUT;
        return RXSOCK_ERR_OS;
    }
    if (domain == nw_error_domain_dns) return RXSOCK_ERR_DNS;
    if (domain == nw_error_domain_tls && rxvm_socket_network_tls_code_is_verify(code)) {
        return RXSOCK_ERR_TLS_VERIFY;
    }
    return fallback;
}

static void rxvm_socket_entry_network_error(rxvm_socket_entry *entry,
                                           rxinteger fallback,
                                           nw_error_t error,
                                           const char *prefix) {
    char buffer[176];
    rxinteger status = rxvm_socket_network_error_status(error, fallback);
    int domain = error ? (int)nw_error_get_error_domain(error) : 0;
    int code = error ? nw_error_get_error_code(error) : 0;

    if (status == RXSOCK_ERR_TLS_VERIFY) {
        prefix = "TLS certificate verification failed";
    }
    snprintf(buffer, sizeof(buffer), "%s: Network.framework error domain=%d code=%d",
             prefix ? prefix : "TLS error", domain, code);
    rxvm_socket_entry_status(entry, status, code, buffer);
}

static void rxvm_socket_network_clear(rxvm_socket_entry *entry) {
    if (!entry) return;
    if (entry->nw_conn) {
        nw_connection_cancel(entry->nw_conn);
        nw_release(entry->nw_conn);
        entry->nw_conn = 0;
    }
    if (entry->nw_queue) {
        dispatch_release(entry->nw_queue);
        entry->nw_queue = 0;
    }
}
#endif

#ifdef CREXX_TLS_SCHANNEL
static void rxvm_socket_entry_schannel_error(rxvm_socket_entry *entry,
                                             rxinteger status,
                                             SECURITY_STATUS schannel_status,
                                             const char *prefix) {
    char buffer[176];

    snprintf(buffer, sizeof(buffer), "%s: SChannel error 0x%08lx",
             prefix ? prefix : "TLS error", (unsigned long)schannel_status);
    rxvm_socket_entry_status(entry, status, (int)schannel_status, buffer);
}

static void rxvm_socket_entry_schannel_verify_error(rxvm_socket_entry *entry,
                                                    DWORD error_code,
                                                    const char *prefix) {
    char buffer[176];

    snprintf(buffer, sizeof(buffer), "%s: Windows trust error 0x%08lx",
             prefix ? prefix : "TLS certificate verification failed",
             (unsigned long)error_code);
    rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_VERIFY, (int)error_code, buffer);
}

static wchar_t *rxvm_socket_wide_from_utf8(const char *text) {
    int length;
    wchar_t *wide;

    if (!text) return 0;
    length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, 0, 0);
    if (length <= 0) {
        length = MultiByteToWideChar(CP_ACP, 0, text, -1, 0, 0);
    }
    if (length <= 0) return 0;

    wide = malloc((size_t)length * sizeof(wchar_t));
    if (!wide) return 0;
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, wide, length) <= 0 &&
        MultiByteToWideChar(CP_ACP, 0, text, -1, wide, length) <= 0) {
        free(wide);
        return 0;
    }
    return wide;
}

static int rxvm_socket_schannel_append_buffer(char **buffer,
                                              size_t *buffer_len,
                                              const char *data,
                                              size_t data_len) {
    char *new_buffer;

    if (!data_len) return 0;
    if (!buffer || !buffer_len || !data) return -1;
    if (data_len > ((size_t)-1) - *buffer_len) return -1;
    new_buffer = realloc(*buffer, *buffer_len + data_len);
    if (!new_buffer) return -1;
    memcpy(new_buffer + *buffer_len, data, data_len);
    *buffer = new_buffer;
    *buffer_len += data_len;
    return 0;
}

static void rxvm_socket_schannel_clear(rxvm_socket_entry *entry) {
    if (!entry) return;
    if (entry->schannel_have_ctx) {
        DeleteSecurityContext(&entry->schannel_ctx);
        memset(&entry->schannel_ctx, 0, sizeof(entry->schannel_ctx));
        entry->schannel_have_ctx = 0;
    }
    if (entry->schannel_have_cred) {
        FreeCredentialsHandle(&entry->schannel_cred);
        memset(&entry->schannel_cred, 0, sizeof(entry->schannel_cred));
        entry->schannel_have_cred = 0;
    }
    memset(&entry->schannel_sizes, 0, sizeof(entry->schannel_sizes));
    free(entry->schannel_encrypted);
    entry->schannel_encrypted = 0;
    entry->schannel_encrypted_len = 0;
    free(entry->schannel_decrypted);
    entry->schannel_decrypted = 0;
    entry->schannel_decrypted_len = 0;
    entry->schannel_decrypted_pos = 0;
}
#endif

static void rxvm_socket_tls_clear(rxvm_socket_entry *entry) {
    if (!entry) return;
#ifdef CREXX_TLS_OPENSSL
    if (entry->tls_ssl) {
        SSL_shutdown(entry->tls_ssl);
        SSL_free(entry->tls_ssl);
        entry->tls_ssl = 0;
    }
    if (entry->tls_ctx) {
        SSL_CTX_free(entry->tls_ctx);
        entry->tls_ctx = 0;
    }
#endif
#ifdef CREXX_TLS_NETWORK
    rxvm_socket_network_clear(entry);
#endif
#ifdef CREXX_TLS_SCHANNEL
    rxvm_socket_schannel_clear(entry);
#endif
    entry->tls_active = 0;
}

static int rxvm_socket_error_is_timeout_or_wouldblock(rxvm_socket_entry *entry,
                                                      int err,
                                                      rxinteger *status,
                                                      const char **message) {
#ifdef _WIN32
    if (err == WSAETIMEDOUT) {
        *status = RXSOCK_TIMEOUT;
        *message = "timeout";
        return 1;
    }
    if (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS || err == WSAEALREADY) {
        *status = RXSOCK_WOULDBLOCK;
        *message = "operation would block";
        return 1;
    }
#else
    if (err == EINPROGRESS || err == EALREADY) {
        *status = RXSOCK_WOULDBLOCK;
        *message = "operation would block";
        return 1;
    }
    if (err == EAGAIN || err == EWOULDBLOCK) {
        if (entry && entry->blocking && entry->timeout_ms > 0) {
            *status = RXSOCK_TIMEOUT;
            *message = "timeout";
        } else {
            *status = RXSOCK_WOULDBLOCK;
            *message = "operation would block";
        }
        return 1;
    }
#endif
    (void)entry;
    return 0;
}

#ifdef CREXX_TLS_SCHANNEL
static int rxvm_socket_schannel_send_all_fd(rxvm_socket_entry *entry,
                                            const char *data,
                                            size_t length,
                                            const char *prefix) {
    size_t total = 0;

    if (!entry || rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return -1;
    }
    while (total < length) {
        size_t chunk = length - total;
        int sent;

        if (chunk > INT_MAX) chunk = INT_MAX;
        sent = send(entry->fd, data + total, (int)chunk, 0);
        if (sent == SOCKET_ERROR) {
            rxinteger status;
            const char *message;
            int err = rxvm_socket_last_errno();

            if (rxvm_socket_error_is_timeout_or_wouldblock(entry, err, &status, &message)) {
                rxvm_socket_entry_status(entry, status, err, message);
                return -1;
            }
            rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, prefix ? prefix : "send failed");
            return -1;
        }
        if (sent == 0) {
            rxvm_socket_entry_status(entry, RXSOCK_EOF, 0,
                                     prefix ? prefix : "connection closed");
            return -1;
        }
        total += (size_t)sent;
    }
    return 0;
}

static int rxvm_socket_schannel_recv_fd(rxvm_socket_entry *entry,
                                        char *buffer,
                                        size_t max_bytes,
                                        const char *prefix) {
    int received;

    if (!entry || rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return -1;
    }
    if (max_bytes > INT_MAX) max_bytes = INT_MAX;
    received = recv(entry->fd, buffer, (int)max_bytes, 0);
    if (received == SOCKET_ERROR) {
        rxinteger status;
        const char *message;
        int err = rxvm_socket_last_errno();

        if (rxvm_socket_error_is_timeout_or_wouldblock(entry, err, &status, &message)) {
            rxvm_socket_entry_status(entry, status, err, message);
            return -1;
        }
        rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, prefix ? prefix : "recv failed");
        return -1;
    }
    if (received == 0) {
        rxvm_socket_entry_status(entry, RXSOCK_EOF, 0,
                                 prefix ? prefix : "connection closed");
        return 0;
    }
    return received;
}

static int rxvm_socket_schannel_read_encrypted(rxvm_socket_entry *entry,
                                               char **buffer,
                                               size_t *buffer_len,
                                               const char *prefix) {
    char temp[8192];
    int received;

    received = rxvm_socket_schannel_recv_fd(entry, temp, sizeof(temp), prefix);
    if (received <= 0) return received;
    if (rxvm_socket_schannel_append_buffer(buffer, buffer_len, temp, (size_t)received) != 0) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0, "out of memory");
        return -1;
    }
    return received;
}
#endif

static int rxvm_socket_platform_init(rxvm_socket_registry *registry) {
#ifdef _WIN32
    WSADATA wsa;
    int rc;

    if (registry->wsa_started) return 0;
    rc = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (rc != 0) return -1;
    registry->wsa_started = 1;
#else
    (void)registry;
#endif
    return 0;
}

static rxvm_socket_registry *rxvm_socket_registry_for(rxvm_context *context) {
    rxvm_socket_registry *registry;

    if (!context) return 0;
    if (context->socket_registry) return context->socket_registry;

    registry = calloc(1, sizeof(rxvm_socket_registry));
    if (!registry) return 0;
    registry->capacity = 16;
    registry->entries = calloc(registry->capacity, sizeof(rxvm_socket_entry));
    if (!registry->entries) {
        free(registry);
        return 0;
    }
    registry->next_handle = 1;
    if (rxvm_socket_platform_init(registry) != 0) {
        free(registry->entries);
        free(registry);
        return 0;
    }
    context->socket_registry = registry;
    return registry;
}

static rxvm_socket_entry *rxvm_socket_lookup(rxvm_context *context, rxinteger handle) {
    rxvm_socket_registry *registry;
    size_t i;

    if (!context || handle <= 0) return 0;
    registry = context->socket_registry;
    if (!registry) return 0;

    for (i = 0; i < registry->count; i++) {
        if (registry->entries[i].in_use && registry->entries[i].handle == handle) {
            return &registry->entries[i];
        }
    }
    return 0;
}

static rxvm_socket_entry *rxvm_socket_alloc_entry(rxvm_context *context) {
    rxvm_socket_registry *registry = rxvm_socket_registry_for(context);
    size_t i;

    if (!registry) return 0;

    for (i = 0; i < registry->count; i++) {
        if (!registry->entries[i].in_use) return &registry->entries[i];
    }

    if (registry->count == registry->capacity) {
        size_t new_capacity = registry->capacity * 2;
        rxvm_socket_entry *new_entries = realloc(registry->entries, new_capacity * sizeof(rxvm_socket_entry));
        if (!new_entries) return 0;
        memset(new_entries + registry->capacity, 0,
               (new_capacity - registry->capacity) * sizeof(rxvm_socket_entry));
        registry->entries = new_entries;
        registry->capacity = new_capacity;
    }

    return &registry->entries[registry->count++];
}

static void rxvm_socket_prepare_fd(rxvm_socket_fd fd) {
#ifdef SO_NOSIGPIPE
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (const char *)&one, sizeof(one));
#else
    (void)fd;
#endif
}

static int rxvm_socket_apply_timeout(rxvm_socket_entry *entry) {
    int timeout_ms;

    if (!entry || rxvm_socket_is_invalid(entry->fd)) return 0;
    timeout_ms = entry->timeout_ms < 0 ? 0 : entry->timeout_ms;

#ifdef _WIN32
    {
        DWORD timeout = (DWORD)timeout_ms;
        if (setsockopt(entry->fd, SOL_SOCKET, SO_RCVTIMEO,
                       (const char *)&timeout, sizeof(timeout)) != 0) return -1;
        if (setsockopt(entry->fd, SOL_SOCKET, SO_SNDTIMEO,
                       (const char *)&timeout, sizeof(timeout)) != 0) return -1;
    }
#else
    {
        struct timeval timeout;
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;
        if (setsockopt(entry->fd, SOL_SOCKET, SO_RCVTIMEO,
                       (const char *)&timeout, sizeof(timeout)) != 0) return -1;
        if (setsockopt(entry->fd, SOL_SOCKET, SO_SNDTIMEO,
                       (const char *)&timeout, sizeof(timeout)) != 0) return -1;
    }
#endif
    return 0;
}

static int rxvm_socket_apply_blocking(rxvm_socket_entry *entry) {
    if (!entry || rxvm_socket_is_invalid(entry->fd)) return 0;
#ifdef _WIN32
    {
        u_long mode = entry->blocking ? 0 : 1;
        return ioctlsocket(entry->fd, FIONBIO, &mode);
    }
#else
    {
        int flags = fcntl(entry->fd, F_GETFL, 0);
        if (flags < 0) return -1;
        if (entry->blocking) flags &= ~O_NONBLOCK;
        else flags |= O_NONBLOCK;
        return fcntl(entry->fd, F_SETFL, flags);
    }
#endif
}

static char *rxvm_socket_copy_cstring(const char *text) {
    size_t length;
    char *copy;

    if (!text) return 0;
    length = strlen(text);
    copy = malloc(length + 1);
    if (!copy) return 0;
    memcpy(copy, text, length + 1);
    return copy;
}

static void rxvm_socket_clear_connect_target(rxvm_socket_entry *entry) {
    if (!entry) return;
    free(entry->connect_host);
    entry->connect_host = 0;
    entry->connect_port = 0;
}

static int rxvm_socket_set_connect_target(rxvm_socket_entry *entry,
                                          const char *host,
                                          rxinteger port) {
    char *host_copy;

    if (!entry || !host || !host[0]) return -1;
    host_copy = rxvm_socket_copy_cstring(host);
    if (!host_copy) return -1;
    free(entry->connect_host);
    entry->connect_host = host_copy;
    entry->connect_port = port;
    return 0;
}

static void rxvm_socket_close_entry_fd(rxvm_socket_entry *entry) {
    if (!entry) return;
    rxvm_socket_tls_clear(entry);
    if (!rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_close_fd(entry->fd);
        entry->fd = RXVM_SOCKET_INVALID;
    }
    entry->is_server = 0;
}

static rxvm_socket_fd rxvm_socket_open_addrinfo(rxvm_socket_entry *entry, const struct addrinfo *ai) {
    rxvm_socket_fd fd;
    int one = 1;

    fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (rxvm_socket_is_invalid(fd)) return RXVM_SOCKET_INVALID;
    rxvm_socket_prepare_fd(fd);
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));

    entry->fd = fd;
    if (rxvm_socket_apply_timeout(entry) != 0 || rxvm_socket_apply_blocking(entry) != 0) {
        rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "setsockopt failed");
        entry->fd = RXVM_SOCKET_INVALID;
        rxvm_socket_close_fd(fd);
        return RXVM_SOCKET_INVALID;
    }

    return fd;
}

static char *rxvm_socket_value_to_cstring(value *v) {
    if (!v) return 0;
    return reg2nullstring(v);
}

void rxvm_socket_free_registry(struct rxvm_context *context) {
    rxvm_socket_registry *registry;
    size_t i;

    if (!context || !context->socket_registry) return;
    registry = context->socket_registry;

    for (i = 0; i < registry->count; i++) {
        if (registry->entries[i].in_use) {
            rxvm_socket_close_entry_fd(&registry->entries[i]);
            rxvm_socket_clear_connect_target(&registry->entries[i]);
            registry->entries[i].in_use = 0;
        }
    }

#ifdef _WIN32
    if (registry->wsa_started) WSACleanup();
#endif
    free(registry->entries);
    free(registry);
    context->socket_registry = 0;
}

rxinteger rxvm_socket_new(struct rxvm_context *context) {
    rxvm_socket_entry *entry = rxvm_socket_alloc_entry(context);

    if (!entry) return RXSOCK_ERR_NO_MEMORY;

    memset(entry, 0, sizeof(*entry));
    entry->handle = context->socket_registry->next_handle++;
    if (context->socket_registry->next_handle <= 0) context->socket_registry->next_handle = 1;
    entry->fd = RXVM_SOCKET_INVALID;
    entry->in_use = 1;
    entry->blocking = 1;
    rxvm_socket_entry_ok(entry);
    return entry->handle;
}

rxinteger rxvm_socket_close(struct rxvm_context *context, rxinteger handle) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);

    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    rxvm_socket_close_entry_fd(entry);
    rxvm_socket_clear_connect_target(entry);
    entry->in_use = 0;
    return RXSOCK_OK;
}

rxinteger rxvm_socket_connect(struct rxvm_context *context, rxinteger handle, value *host_value, rxinteger port) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    struct addrinfo hints;
    struct addrinfo *res = 0;
    struct addrinfo *ai;
    char *host;
    char port_text[16];
    int rc;

    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (port <= 0 || port > 65535) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0, "invalid port");
        return RXSOCK_ERR_ARGUMENT;
    }

    host = rxvm_socket_value_to_cstring(host_value);
    if (!host || !host[0]) {
        if (host) free(host);
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0, "invalid host");
        return RXSOCK_ERR_ARGUMENT;
    }

    rxvm_socket_close_entry_fd(entry);
    rxvm_socket_clear_connect_target(entry);
    snprintf(port_text, sizeof(port_text), "%ld", (long)port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    rc = getaddrinfo(host, port_text, &hints, &res);
    if (rc != 0 || !res) {
        free(host);
        rxvm_socket_entry_status(entry, RXSOCK_ERR_DNS, rc, "name resolution failed");
        return RXSOCK_ERR_DNS;
    }

    for (ai = res; ai; ai = ai->ai_next) {
        rxvm_socket_fd fd = rxvm_socket_open_addrinfo(entry, ai);
        if (rxvm_socket_is_invalid(fd)) continue;
        if (connect(fd, ai->ai_addr, (socklen_t)ai->ai_addrlen) == 0) {
            if (rxvm_socket_set_connect_target(entry, host, port) != 0) {
                free(host);
                freeaddrinfo(res);
                rxvm_socket_close_entry_fd(entry);
                rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0, "out of memory");
                return RXSOCK_ERR_NO_MEMORY;
            }
            entry->is_server = 0;
            rxvm_socket_entry_ok(entry);
            free(host);
            freeaddrinfo(res);
            return RXSOCK_OK;
        }
        {
            rxinteger status;
            const char *message;
            int err = rxvm_socket_last_errno();
            if (rxvm_socket_error_is_timeout_or_wouldblock(entry, err, &status, &message)) {
                rxvm_socket_entry_status(entry, status, err, message);
                free(host);
                freeaddrinfo(res);
                return status;
            }
            rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "connect failed");
            rxvm_socket_close_entry_fd(entry);
        }
    }

    free(host);
    freeaddrinfo(res);
    return entry->last_status ? entry->last_status : RXSOCK_ERR_OS;
}

rxinteger rxvm_socket_bind(struct rxvm_context *context, rxinteger handle, value *host_value, rxinteger port) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    struct addrinfo hints;
    struct addrinfo *res = 0;
    struct addrinfo *ai;
    char *host = 0;
    char port_text[16];
    int rc;

    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (port < 0 || port > 65535) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0, "invalid port");
        return RXSOCK_ERR_ARGUMENT;
    }

    if (host_value && host_value->string_length) host = rxvm_socket_value_to_cstring(host_value);
    rxvm_socket_close_entry_fd(entry);
    rxvm_socket_clear_connect_target(entry);
    snprintf(port_text, sizeof(port_text), "%ld", (long)port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    rc = getaddrinfo((host && host[0]) ? host : 0, port_text, &hints, &res);
    if (host) free(host);
    if (rc != 0 || !res) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_DNS, rc, "bind name resolution failed");
        return RXSOCK_ERR_DNS;
    }

    for (ai = res; ai; ai = ai->ai_next) {
        rxvm_socket_fd fd = rxvm_socket_open_addrinfo(entry, ai);
        if (rxvm_socket_is_invalid(fd)) continue;
        if (bind(fd, ai->ai_addr, (socklen_t)ai->ai_addrlen) == 0) {
            entry->is_server = 1;
            rxvm_socket_entry_ok(entry);
            freeaddrinfo(res);
            return RXSOCK_OK;
        }
        rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "bind failed");
        rxvm_socket_close_entry_fd(entry);
    }

    freeaddrinfo(res);
    return entry->last_status ? entry->last_status : RXSOCK_ERR_OS;
}

rxinteger rxvm_socket_listen(struct rxvm_context *context, rxinteger handle, rxinteger backlog) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    int effective_backlog;

    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
    }

    if (backlog <= 0) effective_backlog = 5;
    else effective_backlog = backlog > INT_MAX ? INT_MAX : (int)backlog;
    if (listen(entry->fd, effective_backlog) != 0) {
        rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "listen failed");
        return RXSOCK_ERR_OS;
    }
    entry->is_server = 1;
    rxvm_socket_entry_ok(entry);
    return RXSOCK_OK;
}

rxinteger rxvm_socket_accept(struct rxvm_context *context, rxinteger server_handle) {
    rxvm_socket_entry *server = rxvm_socket_lookup(context, server_handle);
    rxvm_socket_entry *client;
    rxinteger client_handle;
    rxvm_socket_fd fd;
    struct sockaddr_storage addr;
#ifdef _WIN32
    int addr_len = sizeof(addr);
#else
    socklen_t addr_len = sizeof(addr);
#endif

    if (!server) return RXSOCK_ERR_INVALID_HANDLE;
    if (rxvm_socket_is_invalid(server->fd)) {
        rxvm_socket_entry_status(server, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
    }

    fd = accept(server->fd, (struct sockaddr *)&addr, &addr_len);
    if (rxvm_socket_is_invalid(fd)) {
        rxinteger status;
        const char *message;
        int err = rxvm_socket_last_errno();
        if (rxvm_socket_error_is_timeout_or_wouldblock(server, err, &status, &message)) {
            rxvm_socket_entry_status(server, status, err, message);
            return status;
        }
        rxvm_socket_entry_os_error(server, RXSOCK_ERR_OS, "accept failed");
        return RXSOCK_ERR_OS;
    }

    rxvm_socket_prepare_fd(fd);
    client_handle = rxvm_socket_new(context);
    if (client_handle < 0) {
        rxvm_socket_close_fd(fd);
        rxvm_socket_entry_status(server, RXSOCK_ERR_NO_MEMORY, 0, "out of memory");
        return RXSOCK_ERR_NO_MEMORY;
    }

    client = rxvm_socket_lookup(context, client_handle);
    client->fd = fd;
    client->is_server = 0;
    client->timeout_ms = server->timeout_ms;
    client->blocking = server->blocking;
    if (rxvm_socket_apply_timeout(client) != 0 || rxvm_socket_apply_blocking(client) != 0) {
        rxvm_socket_entry_os_error(client, RXSOCK_ERR_OS, "accepted socket setup failed");
    } else {
        rxvm_socket_entry_ok(client);
    }
    rxvm_socket_entry_ok(server);
    return client_handle;
}

rxinteger rxvm_socket_shutdown(struct rxvm_context *context, rxinteger handle, rxinteger how) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    int native_how;

    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
#ifdef CREXX_TLS_NETWORK
    if (entry->tls_active && entry->nw_conn) {
        rxvm_socket_tls_clear(entry);
        rxvm_socket_entry_ok(entry);
        return RXSOCK_OK;
    }
#endif
    if (!entry->tls_active && rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
    }
    if (how < 0 || how > 2) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0, "invalid shutdown mode");
        return RXSOCK_ERR_ARGUMENT;
    }

#ifdef _WIN32
    native_how = how == 0 ? SD_RECEIVE : (how == 1 ? SD_SEND : SD_BOTH);
#else
    native_how = how == 0 ? SHUT_RD : (how == 1 ? SHUT_WR : SHUT_RDWR);
#endif
    if (shutdown(entry->fd, native_how) != 0) {
        rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "shutdown failed");
        return RXSOCK_ERR_OS;
    }
    rxvm_socket_entry_ok(entry);
    return RXSOCK_OK;
}

#ifdef CREXX_TLS_NETWORK
static rxinteger rxvm_socket_connect_tls_network(rxvm_socket_entry *entry,
                                                 const char *connect_host,
                                                 rxinteger connect_port,
                                                 const char *tls_host) {
    nw_endpoint_t endpoint = 0;
    nw_parameters_t parameters = 0;
    nw_connection_t connection = 0;
    dispatch_queue_t queue = 0;
    __block rxvm_socket_network_wait wait;
    char port_text[16];
    rxinteger result = RXSOCK_OK;
    int timed_out = 0;

    if (!connect_host || !connect_host[0] || !tls_host || !tls_host[0]) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0,
                                 "invalid TLS hostname");
        return RXSOCK_ERR_ARGUMENT;
    }
    if (connect_port <= 0 || connect_port > 65535) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0, "invalid port");
        return RXSOCK_ERR_ARGUMENT;
    }
    if (entry->is_server) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_STATE, 0,
                                 "client TLS is not available on listening sockets");
        return RXSOCK_ERR_TLS_STATE;
    }
    if (!entry->blocking) {
        rxvm_socket_entry_status(entry, RXSOCK_WOULDBLOCK, 0,
                                 "Network.framework TLS requires blocking socket mode");
        return RXSOCK_WOULDBLOCK;
    }

    rxvm_socket_close_entry_fd(entry);
    rxvm_socket_clear_connect_target(entry);

    snprintf(port_text, sizeof(port_text), "%ld", (long)connect_port);
    endpoint = nw_endpoint_create_host(connect_host, port_text);
    if (!endpoint) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0,
                                 "Network.framework endpoint creation failed");
        return RXSOCK_ERR_NO_MEMORY;
    }

    parameters = nw_parameters_create_secure_tcp(^(nw_protocol_options_t options) {
        sec_protocol_options_t security_options = nw_tls_copy_sec_protocol_options(options);
        if (security_options) {
            sec_protocol_options_set_tls_server_name(security_options, tls_host);
            sec_protocol_options_set_peer_authentication_required(security_options, true);
            sec_protocol_options_set_verify_block(security_options,
                                                  ^(sec_protocol_metadata_t metadata,
                                                    sec_trust_t trust_ref,
                                                    sec_protocol_verify_complete_t complete) {
                SecTrustRef trust;
                CFErrorRef trust_error = 0;
                bool trusted = false;

                (void)metadata;
                trust = sec_trust_copy_ref(trust_ref);
                if (trust) {
                    trusted = SecTrustEvaluateWithError(trust, &trust_error);
                    if (trust_error) CFRelease(trust_error);
                    CFRelease(trust);
                }
                complete(trusted);
            }, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
            sec_release(security_options);
        }
    }, NW_PARAMETERS_DEFAULT_CONFIGURATION);
    if (!parameters) {
        nw_release(endpoint);
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_STATE, 0,
                                 "Network.framework TLS parameter creation failed");
        return RXSOCK_ERR_TLS_STATE;
    }

    queue = dispatch_queue_create("org.crexx.rxvm.socket.tls", DISPATCH_QUEUE_SERIAL);
    if (!queue) {
        nw_release(parameters);
        nw_release(endpoint);
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0,
                                 "Network.framework queue creation failed");
        return RXSOCK_ERR_NO_MEMORY;
    }

    connection = nw_connection_create(endpoint, parameters);
    nw_release(parameters);
    nw_release(endpoint);
    if (!connection) {
        dispatch_release(queue);
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_STATE, 0,
                                 "Network.framework connection creation failed");
        return RXSOCK_ERR_TLS_STATE;
    }

    memset(&wait, 0, sizeof(wait));
    wait.done = dispatch_semaphore_create(0);
    if (!wait.done) {
        nw_release(connection);
        dispatch_release(queue);
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0,
                                 "Network.framework wait setup failed");
        return RXSOCK_ERR_NO_MEMORY;
    }

    nw_connection_set_queue(connection, queue);
    nw_connection_set_state_changed_handler(connection, ^(nw_connection_state_t state,
                                                         nw_error_t error) {
        if (wait.complete) return;
        if ((state == nw_connection_state_waiting && error) ||
            state == nw_connection_state_ready ||
            state == nw_connection_state_failed ||
            state == nw_connection_state_cancelled) {
            wait.ready = state == nw_connection_state_ready;
            if (error) wait.error = nw_retain(error);
            wait.complete = 1;
            dispatch_semaphore_signal(wait.done);
        }
    });
    nw_connection_start(connection);

    if (rxvm_socket_network_wait_for(entry, wait.done) != 0) {
        timed_out = 1;
        nw_connection_cancel(connection);
        dispatch_semaphore_wait(wait.done, DISPATCH_TIME_FOREVER);
    }
    nw_connection_set_state_changed_handler(connection, 0);

    if (timed_out) {
        rxvm_socket_entry_status(entry, RXSOCK_TIMEOUT, ETIMEDOUT,
                                 "TLS handshake timeout");
        result = RXSOCK_TIMEOUT;
    } else if (!wait.ready) {
        rxvm_socket_entry_network_error(entry, RXSOCK_ERR_TLS_HANDSHAKE,
                                        wait.error, "TLS handshake failed");
        result = entry->last_status ? entry->last_status : RXSOCK_ERR_TLS_HANDSHAKE;
    }

    if (wait.error) nw_release(wait.error);
    dispatch_release(wait.done);

    if (result != RXSOCK_OK) {
        nw_connection_cancel(connection);
        nw_release(connection);
        dispatch_release(queue);
        return result;
    }

    if (rxvm_socket_set_connect_target(entry, connect_host, connect_port) != 0) {
        nw_connection_cancel(connection);
        nw_release(connection);
        dispatch_release(queue);
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0, "out of memory");
        return RXSOCK_ERR_NO_MEMORY;
    }

    entry->is_server = 0;
    entry->nw_conn = connection;
    entry->nw_queue = queue;
    entry->tls_active = 1;
    rxvm_socket_entry_ok(entry);
    return RXSOCK_OK;
}
#endif

#ifdef CREXX_TLS_SCHANNEL
static SecBuffer *rxvm_socket_schannel_find_buffer(SecBuffer *buffers,
                                                   unsigned long count,
                                                   unsigned long buffer_type) {
    unsigned long i;

    for (i = 0; i < count; i++) {
        if (buffers[i].BufferType == buffer_type && buffers[i].cbBuffer > 0 &&
            buffers[i].pvBuffer) {
            return &buffers[i];
        }
    }
    return 0;
}

static rxinteger rxvm_socket_schannel_handshake_read(rxvm_socket_entry *entry,
                                                     char **input,
                                                     size_t *input_len) {
    int rc = rxvm_socket_schannel_read_encrypted(entry, input, input_len,
                                                 "TLS handshake receive failed");

    if (rc > 0) return RXSOCK_OK;
    if (entry->last_status == RXSOCK_EOF) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_HANDSHAKE, 0,
                                 "TLS handshake closed by peer");
        return RXSOCK_ERR_TLS_HANDSHAKE;
    }
    return entry->last_status ? entry->last_status : RXSOCK_ERR_TLS_HANDSHAKE;
}

static rxinteger rxvm_socket_schannel_verify_server(rxvm_socket_entry *entry,
                                                    wchar_t *wide_host) {
    SECURITY_STATUS status;
    PCCERT_CONTEXT cert = 0;
    PCCERT_CHAIN_CONTEXT chain = 0;
    CERT_CHAIN_PARA chain_para;
    SSL_EXTRA_CERT_CHAIN_POLICY_PARA https_policy;
    CERT_CHAIN_POLICY_PARA policy_para;
    CERT_CHAIN_POLICY_STATUS policy_status;
    rxinteger result = RXSOCK_OK;

    status = QueryContextAttributes(&entry->schannel_ctx,
                                    SECPKG_ATTR_REMOTE_CERT_CONTEXT,
                                    (void *)&cert);
    if (status != SEC_E_OK || !cert) {
        rxvm_socket_entry_schannel_error(entry, RXSOCK_ERR_TLS_VERIFY, status,
                                         "TLS peer certificate lookup failed");
        return RXSOCK_ERR_TLS_VERIFY;
    }

    memset(&chain_para, 0, sizeof(chain_para));
    chain_para.cbSize = sizeof(chain_para);
    if (!CertGetCertificateChain(0, cert, 0, cert->hCertStore, &chain_para, 0, 0, &chain)) {
        rxvm_socket_entry_schannel_verify_error(entry, GetLastError(),
                                                "TLS certificate chain validation failed");
        CertFreeCertificateContext(cert);
        return RXSOCK_ERR_TLS_VERIFY;
    }

    memset(&https_policy, 0, sizeof(https_policy));
    https_policy.cbSize = sizeof(https_policy);
    https_policy.dwAuthType = AUTHTYPE_SERVER;
    https_policy.fdwChecks = 0;
    https_policy.pwszServerName = wide_host;

    memset(&policy_para, 0, sizeof(policy_para));
    policy_para.cbSize = sizeof(policy_para);
    policy_para.pvExtraPolicyPara = &https_policy;

    memset(&policy_status, 0, sizeof(policy_status));
    policy_status.cbSize = sizeof(policy_status);
    if (!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL, chain,
                                          &policy_para, &policy_status)) {
        rxvm_socket_entry_schannel_verify_error(entry, GetLastError(),
                                                "TLS certificate policy validation failed");
        result = RXSOCK_ERR_TLS_VERIFY;
    } else if (policy_status.dwError != 0) {
        rxvm_socket_entry_schannel_verify_error(entry, policy_status.dwError,
                                                "TLS certificate verification failed");
        result = RXSOCK_ERR_TLS_VERIFY;
    }

    CertFreeCertificateChain(chain);
    CertFreeCertificateContext(cert);
    return result;
}

static rxinteger rxvm_socket_starttls_schannel(rxvm_socket_entry *entry,
                                               const char *host) {
    SCHANNEL_CRED cred;
    TimeStamp expiry;
    wchar_t *wide_host = 0;
    char *input = 0;
    size_t input_len = 0;
    SECURITY_STATUS status;
    DWORD attrs = 0;
    rxinteger result = RXSOCK_OK;
    const DWORD request_flags = ISC_REQ_SEQUENCE_DETECT |
                                ISC_REQ_REPLAY_DETECT |
                                ISC_REQ_CONFIDENTIALITY |
                                ISC_REQ_EXTENDED_ERROR |
                                ISC_REQ_ALLOCATE_MEMORY |
                                ISC_REQ_STREAM;

    if (!host || !host[0]) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0,
                                 "invalid TLS hostname");
        return RXSOCK_ERR_ARGUMENT;
    }
    if (entry->is_server) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_STATE, 0,
                                 "client TLS is not available on listening sockets");
        return RXSOCK_ERR_TLS_STATE;
    }
    if (!entry->blocking) {
        rxvm_socket_entry_status(entry, RXSOCK_WOULDBLOCK, 0,
                                 "SChannel TLS requires blocking socket mode");
        return RXSOCK_WOULDBLOCK;
    }

    wide_host = rxvm_socket_wide_from_utf8(host);
    if (!wide_host) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0,
                                 "TLS hostname conversion failed");
        return RXSOCK_ERR_NO_MEMORY;
    }

    memset(&cred, 0, sizeof(cred));
    cred.dwVersion = SCHANNEL_CRED_VERSION;
    cred.dwFlags = SCH_CRED_MANUAL_CRED_VALIDATION |
                   SCH_CRED_NO_DEFAULT_CREDS |
                   SCH_USE_STRONG_CRYPTO;

    status = AcquireCredentialsHandleW(0, UNISP_NAME_W, SECPKG_CRED_OUTBOUND,
                                       0, &cred, 0, 0,
                                       &entry->schannel_cred, &expiry);
    if (status != SEC_E_OK) {
        rxvm_socket_entry_schannel_error(entry, RXSOCK_ERR_TLS_STATE, status,
                                         "TLS credential setup failed");
        free(wide_host);
        return RXSOCK_ERR_TLS_STATE;
    }
    entry->schannel_have_cred = 1;

    for (;;) {
        SecBuffer out_buffers[1];
        SecBufferDesc out_desc;
        SecBuffer in_buffers[2];
        SecBufferDesc in_desc;
        SecBufferDesc *input_desc = 0;
        int had_input = input_len > 0;

        memset(out_buffers, 0, sizeof(out_buffers));
        out_buffers[0].BufferType = SECBUFFER_TOKEN;
        out_desc.ulVersion = SECBUFFER_VERSION;
        out_desc.cBuffers = 1;
        out_desc.pBuffers = out_buffers;

        if (had_input) {
            memset(in_buffers, 0, sizeof(in_buffers));
            in_buffers[0].BufferType = SECBUFFER_TOKEN;
            in_buffers[0].pvBuffer = input;
            in_buffers[0].cbBuffer = (unsigned long)input_len;
            in_buffers[1].BufferType = SECBUFFER_EMPTY;
            in_desc.ulVersion = SECBUFFER_VERSION;
            in_desc.cBuffers = 2;
            in_desc.pBuffers = in_buffers;
            input_desc = &in_desc;
        }

        status = InitializeSecurityContextW(&entry->schannel_cred,
                                            entry->schannel_have_ctx ? &entry->schannel_ctx : 0,
                                            wide_host,
                                            request_flags,
                                            0,
                                            SECURITY_NATIVE_DREP,
                                            input_desc,
                                            0,
                                            &entry->schannel_ctx,
                                            &out_desc,
                                            &attrs,
                                            &expiry);

        if (out_buffers[0].pvBuffer && out_buffers[0].cbBuffer > 0) {
            if (rxvm_socket_schannel_send_all_fd(entry,
                                                 (const char *)out_buffers[0].pvBuffer,
                                                 out_buffers[0].cbBuffer,
                                                 "TLS handshake send failed") != 0) {
                result = entry->last_status ? entry->last_status : RXSOCK_ERR_TLS_HANDSHAKE;
                FreeContextBuffer(out_buffers[0].pvBuffer);
                break;
            }
        }
        if (out_buffers[0].pvBuffer) FreeContextBuffer(out_buffers[0].pvBuffer);

        if (status == SEC_E_INCOMPLETE_MESSAGE) {
            result = rxvm_socket_schannel_handshake_read(entry, &input, &input_len);
            if (result != RXSOCK_OK) break;
            continue;
        }
        if (status != SEC_E_OK && status != SEC_I_CONTINUE_NEEDED) {
            rxvm_socket_entry_schannel_error(entry, RXSOCK_ERR_TLS_HANDSHAKE, status,
                                             "TLS handshake failed");
            result = RXSOCK_ERR_TLS_HANDSHAKE;
            break;
        }

        entry->schannel_have_ctx = 1;
        if (status == SEC_E_OK) {
            if (had_input) {
                SecBuffer *extra = rxvm_socket_schannel_find_buffer(in_buffers, 2,
                                                                    SECBUFFER_EXTRA);
                if (extra && rxvm_socket_schannel_append_buffer(&entry->schannel_encrypted,
                                                                &entry->schannel_encrypted_len,
                                                                (const char *)extra->pvBuffer,
                                                                extra->cbBuffer) != 0) {
                    rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0,
                                             "out of memory");
                    result = RXSOCK_ERR_NO_MEMORY;
                    break;
                }
            }
            result = RXSOCK_OK;
            break;
        }

        if (had_input) {
            SecBuffer *extra = rxvm_socket_schannel_find_buffer(in_buffers, 2,
                                                                SECBUFFER_EXTRA);
            if (extra) {
                memmove(input, extra->pvBuffer, extra->cbBuffer);
                input_len = extra->cbBuffer;
            } else {
                input_len = 0;
            }
        }

        if (input_len == 0) {
            result = rxvm_socket_schannel_handshake_read(entry, &input, &input_len);
            if (result != RXSOCK_OK) break;
        }
    }

    free(input);
    if (result == RXSOCK_OK) {
        result = rxvm_socket_schannel_verify_server(entry, wide_host);
    }
    if (result == RXSOCK_OK) {
        status = QueryContextAttributes(&entry->schannel_ctx,
                                        SECPKG_ATTR_STREAM_SIZES,
                                        &entry->schannel_sizes);
        if (status != SEC_E_OK) {
            rxvm_socket_entry_schannel_error(entry, RXSOCK_ERR_TLS_STATE, status,
                                             "TLS stream setup failed");
            result = RXSOCK_ERR_TLS_STATE;
        }
    }
    free(wide_host);

    if (result != RXSOCK_OK) {
        rxvm_socket_schannel_clear(entry);
        return result;
    }

    entry->tls_active = 1;
    rxvm_socket_entry_ok(entry);
    return RXSOCK_OK;
}
#endif

rxinteger rxvm_socket_connect_tls(struct rxvm_context *context,
                                  rxinteger handle,
                                  value *host_value,
                                  rxinteger port) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    char *host;
    rxinteger rc;

    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (port <= 0 || port > 65535) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0, "invalid port");
        return RXSOCK_ERR_ARGUMENT;
    }

    host = rxvm_socket_value_to_cstring(host_value);
    if (!host || !host[0]) {
        if (host) free(host);
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0, "invalid host");
        return RXSOCK_ERR_ARGUMENT;
    }

#ifdef CREXX_TLS_NETWORK
    rc = rxvm_socket_connect_tls_network(entry, host, port, host);
    free(host);
    return rc;
#elif defined(CREXX_TLS_SCHANNEL)
    rc = rxvm_socket_connect(context, handle, host_value, port);
    if (rc != RXSOCK_OK) {
        free(host);
        return rc;
    }
    rc = rxvm_socket_starttls_schannel(entry, host);
    free(host);
    if (rc != RXSOCK_OK) {
        rxvm_socket_close_entry_fd(entry);
        rxvm_socket_clear_connect_target(entry);
    }
    return rc;
#elif defined(CREXX_TLS_OPENSSL)
    free(host);
    rc = rxvm_socket_connect(context, handle, host_value, port);
    if (rc != RXSOCK_OK) return rc;
    rc = rxvm_socket_starttls(context, handle, host_value);
    if (rc != RXSOCK_OK) {
        rxvm_socket_close_entry_fd(entry);
        rxvm_socket_clear_connect_target(entry);
    }
    return rc;
#else
    rxvm_socket_close_entry_fd(entry);
    rxvm_socket_clear_connect_target(entry);
    free(host);
    rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_UNAVAILABLE, 0,
                             "TLS support is not enabled in this cREXX build");
    return RXSOCK_ERR_TLS_UNAVAILABLE;
#endif
}

rxinteger rxvm_socket_starttls(struct rxvm_context *context, rxinteger handle, value *host_value) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    char *host;

    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (entry->tls_active) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_STATE, 0, "TLS is already active on socket");
        return RXSOCK_ERR_TLS_STATE;
    }
    if (!entry->tls_active && rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
    }
    host = rxvm_socket_value_to_cstring(host_value);
    if (!host || !host[0]) {
        if (host) free(host);
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0, "invalid TLS hostname");
        return RXSOCK_ERR_ARGUMENT;
    }

#ifdef CREXX_TLS_OPENSSL
    if (OPENSSL_init_ssl(0, 0) != 1) {
        free(host);
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_STATE, 0, "OpenSSL initialization failed");
        return RXSOCK_ERR_TLS_STATE;
    }

    entry->tls_ctx = SSL_CTX_new(TLS_client_method());
    if (!entry->tls_ctx) {
        free(host);
        rxvm_socket_entry_tls_error(entry, RXSOCK_ERR_TLS_STATE, 0, 0, "TLS context creation failed");
        return RXSOCK_ERR_TLS_STATE;
    }

    SSL_CTX_set_verify(entry->tls_ctx, SSL_VERIFY_PEER, 0);
    if (SSL_CTX_set_default_verify_paths(entry->tls_ctx) != 1) {
        free(host);
        rxvm_socket_tls_clear(entry);
        rxvm_socket_entry_tls_error(entry, RXSOCK_ERR_TLS_VERIFY, 0, 0, "TLS trust store setup failed");
        return RXSOCK_ERR_TLS_VERIFY;
    }

    entry->tls_ssl = SSL_new(entry->tls_ctx);
    if (!entry->tls_ssl) {
        free(host);
        rxvm_socket_tls_clear(entry);
        rxvm_socket_entry_tls_error(entry, RXSOCK_ERR_TLS_STATE, 0, 0, "TLS session creation failed");
        return RXSOCK_ERR_TLS_STATE;
    }

    if (!rxvm_socket_tls_set_peer_name(entry->tls_ssl, host)) {
        free(host);
        rxvm_socket_tls_clear(entry);
        rxvm_socket_entry_tls_error(entry, RXSOCK_ERR_TLS_STATE, 0, 0, "TLS peer name setup failed");
        return RXSOCK_ERR_TLS_STATE;
    }
    free(host);

    if (SSL_set_fd(entry->tls_ssl, (int)entry->fd) != 1) {
        rxvm_socket_tls_clear(entry);
        rxvm_socket_entry_tls_error(entry, RXSOCK_ERR_TLS_STATE, 0, 0, "TLS socket attachment failed");
        return RXSOCK_ERR_TLS_STATE;
    }

    {
        int rc = SSL_connect(entry->tls_ssl);
        if (rc != 1) {
            rxvm_socket_entry_tls_error(entry, RXSOCK_ERR_TLS_HANDSHAKE,
                                        entry->tls_ssl, rc, "TLS handshake failed");
            rxvm_socket_tls_clear(entry);
            return RXSOCK_ERR_TLS_HANDSHAKE;
        }
    }

    {
        long verify_result = SSL_get_verify_result(entry->tls_ssl);
        if (verify_result != X509_V_OK) {
            rxvm_socket_entry_tls_verify_error(entry, verify_result);
            rxvm_socket_tls_clear(entry);
            return RXSOCK_ERR_TLS_VERIFY;
        }
    }

    entry->tls_active = 1;
    rxvm_socket_entry_ok(entry);
    return RXSOCK_OK;
#elif defined(CREXX_TLS_NETWORK)
    free(host);
    rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_UNAVAILABLE, 0,
                             "true STARTTLS is not supported by the Network.framework backend; use socketconnecttls");
    return RXSOCK_ERR_TLS_UNAVAILABLE;
#elif defined(CREXX_TLS_SCHANNEL)
    {
        rxinteger rc = rxvm_socket_starttls_schannel(entry, host);
        free(host);
        return rc;
    }
#else
    free(host);
    rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_UNAVAILABLE, 0,
                             "TLS support is not enabled in this cREXX build");
    return RXSOCK_ERR_TLS_UNAVAILABLE;
#endif
}

#ifdef CREXX_TLS_NETWORK
static int rxvm_socket_network_send(rxvm_socket_entry *entry,
                                    const char *data,
                                    size_t length,
                                    size_t *sent_out) {
    dispatch_data_t content;
    __block rxvm_socket_network_wait wait;
    int timed_out = 0;
    int had_error = 0;

    if (!entry->nw_conn) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_STATE, 0,
                                 "Network.framework TLS connection is not active");
        return -1;
    }
    if (!entry->blocking) {
        rxvm_socket_entry_status(entry, RXSOCK_WOULDBLOCK, 0,
                                 "Network.framework TLS requires blocking socket mode");
        return -1;
    }

    content = dispatch_data_create(data, length, 0, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    if (!content) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0, "out of memory");
        return -1;
    }

    memset(&wait, 0, sizeof(wait));
    wait.done = dispatch_semaphore_create(0);
    if (!wait.done) {
        dispatch_release(content);
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0, "out of memory");
        return -1;
    }

    nw_connection_send(entry->nw_conn, content, NW_CONNECTION_DEFAULT_STREAM_CONTEXT,
                       false, ^(nw_error_t error) {
        if (error) wait.error = nw_retain(error);
        wait.complete = 1;
        dispatch_semaphore_signal(wait.done);
    });

    if (rxvm_socket_network_wait_for(entry, wait.done) != 0) {
        timed_out = 1;
        nw_connection_cancel(entry->nw_conn);
        dispatch_semaphore_wait(wait.done, DISPATCH_TIME_FOREVER);
    }

    if (timed_out) {
        rxvm_socket_entry_status(entry, RXSOCK_TIMEOUT, ETIMEDOUT, "TLS send timeout");
        rxvm_socket_tls_clear(entry);
    } else if (wait.error) {
        rxvm_socket_entry_network_error(entry, RXSOCK_ERR_OS, wait.error, "TLS send failed");
    }

    had_error = wait.error != 0;
    if (wait.error) nw_release(wait.error);
    dispatch_release(wait.done);
    dispatch_release(content);

    if (timed_out || had_error) return -1;
    if (sent_out) *sent_out = length;
    rxvm_socket_entry_ok(entry);
    return 0;
}

static rxinteger rxvm_socket_network_recv(rxvm_socket_entry *entry,
                                          char *buffer,
                                          size_t max_bytes) {
    __block rxvm_socket_network_recv_state recv_state;
    uint32_t receive_limit;
    int timed_out = 0;
    int had_error = 0;

    if (!entry->nw_conn) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_STATE, 0,
                                 "Network.framework TLS connection is not active");
        return RXSOCK_ERR_TLS_STATE;
    }
    if (!entry->blocking) {
        rxvm_socket_entry_status(entry, RXSOCK_WOULDBLOCK, 0,
                                 "Network.framework TLS requires blocking socket mode");
        return RXSOCK_WOULDBLOCK;
    }

    if (max_bytes > INT_MAX) max_bytes = INT_MAX;
    receive_limit = (uint32_t)max_bytes;
    memset(&recv_state, 0, sizeof(recv_state));
    recv_state.done = dispatch_semaphore_create(0);
    if (!recv_state.done) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0, "out of memory");
        return RXSOCK_ERR_NO_MEMORY;
    }

    nw_connection_receive(entry->nw_conn, 1, receive_limit,
                          ^(dispatch_data_t content,
                            nw_content_context_t context,
                            bool is_complete,
                            nw_error_t error) {
        (void)context;
        if (content) {
            dispatch_data_apply(content, ^bool(dispatch_data_t region,
                                              size_t offset,
                                              const void *region_buffer,
                                              size_t size) {
                size_t available;
                size_t copy_size;

                (void)region;
                (void)offset;
                if (recv_state.copied >= max_bytes) return false;
                available = max_bytes - recv_state.copied;
                copy_size = size < available ? size : available;
                memcpy(buffer + recv_state.copied, region_buffer, copy_size);
                recv_state.copied += copy_size;
                return recv_state.copied < max_bytes;
            });
        }
        recv_state.stream_complete = is_complete;
        if (error) recv_state.error = nw_retain(error);
        recv_state.complete = 1;
        dispatch_semaphore_signal(recv_state.done);
    });

    if (rxvm_socket_network_wait_for(entry, recv_state.done) != 0) {
        timed_out = 1;
        nw_connection_cancel(entry->nw_conn);
        dispatch_semaphore_wait(recv_state.done, DISPATCH_TIME_FOREVER);
    }

    if (timed_out) {
        rxvm_socket_entry_status(entry, RXSOCK_TIMEOUT, ETIMEDOUT, "TLS receive timeout");
        rxvm_socket_tls_clear(entry);
    } else if (recv_state.error && recv_state.copied == 0) {
        rxvm_socket_entry_network_error(entry, RXSOCK_ERR_OS, recv_state.error,
                                        "TLS receive failed");
    } else if (recv_state.copied == 0 && recv_state.stream_complete) {
        rxvm_socket_entry_status(entry, RXSOCK_EOF, 0, "TLS connection closed");
    } else {
        rxvm_socket_entry_ok(entry);
    }

    had_error = recv_state.error != 0;
    if (recv_state.error) nw_release(recv_state.error);
    dispatch_release(recv_state.done);

    if (timed_out) return RXSOCK_TIMEOUT;
    if (had_error && recv_state.copied == 0) return entry->last_status;
    return (rxinteger)recv_state.copied;
}
#endif

#ifdef CREXX_TLS_SCHANNEL
static size_t rxvm_socket_schannel_copy_decrypted(rxvm_socket_entry *entry,
                                                  char *buffer,
                                                  size_t max_bytes) {
    size_t available;
    size_t copy_size;

    if (!entry->schannel_decrypted ||
        entry->schannel_decrypted_pos >= entry->schannel_decrypted_len) {
        return 0;
    }

    available = entry->schannel_decrypted_len - entry->schannel_decrypted_pos;
    copy_size = available < max_bytes ? available : max_bytes;
    memcpy(buffer, entry->schannel_decrypted + entry->schannel_decrypted_pos, copy_size);
    entry->schannel_decrypted_pos += copy_size;
    if (entry->schannel_decrypted_pos >= entry->schannel_decrypted_len) {
        free(entry->schannel_decrypted);
        entry->schannel_decrypted = 0;
        entry->schannel_decrypted_len = 0;
        entry->schannel_decrypted_pos = 0;
    }
    return copy_size;
}

static int rxvm_socket_schannel_store_decrypted(rxvm_socket_entry *entry,
                                                const char *data,
                                                size_t data_len) {
    char *copy;

    if (!data_len) return 0;
    copy = malloc(data_len);
    if (!copy) return -1;
    memcpy(copy, data, data_len);
    free(entry->schannel_decrypted);
    entry->schannel_decrypted = copy;
    entry->schannel_decrypted_len = data_len;
    entry->schannel_decrypted_pos = 0;
    return 0;
}

static void rxvm_socket_schannel_preserve_encrypted_extra(rxvm_socket_entry *entry,
                                                          SecBuffer *extra) {
    if (extra && extra->pvBuffer && extra->cbBuffer > 0) {
        memmove(entry->schannel_encrypted, extra->pvBuffer, extra->cbBuffer);
        entry->schannel_encrypted_len = extra->cbBuffer;
        return;
    }

    free(entry->schannel_encrypted);
    entry->schannel_encrypted = 0;
    entry->schannel_encrypted_len = 0;
}

static int rxvm_socket_schannel_send(rxvm_socket_entry *entry,
                                     const char *data,
                                     size_t length,
                                     size_t *sent_out) {
    SecBuffer buffers[4];
    SecBufferDesc desc;
    SECURITY_STATUS status;
    DWORD max_message;
    size_t chunk;
    size_t wire_len;
    size_t encrypted_len;
    char *wire;

    if (!entry->schannel_have_ctx) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_STATE, 0,
                                 "SChannel TLS connection is not active");
        return -1;
    }
    if (!entry->blocking) {
        rxvm_socket_entry_status(entry, RXSOCK_WOULDBLOCK, 0,
                                 "SChannel TLS requires blocking socket mode");
        return -1;
    }

    max_message = entry->schannel_sizes.cbMaximumMessage ?
                  entry->schannel_sizes.cbMaximumMessage : 16384;
    chunk = length;
    if (chunk > max_message) chunk = max_message;
    if (chunk > INT_MAX) chunk = INT_MAX;

    if ((size_t)entry->schannel_sizes.cbHeader > ((size_t)-1) - chunk ||
        (size_t)entry->schannel_sizes.cbTrailer >
            ((size_t)-1) - (size_t)entry->schannel_sizes.cbHeader - chunk) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0, "TLS record too large");
        return -1;
    }

    wire_len = (size_t)entry->schannel_sizes.cbHeader + chunk +
               (size_t)entry->schannel_sizes.cbTrailer;
    wire = malloc(wire_len);
    if (!wire) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0, "out of memory");
        return -1;
    }
    memcpy(wire + entry->schannel_sizes.cbHeader, data, chunk);

    memset(buffers, 0, sizeof(buffers));
    buffers[0].BufferType = SECBUFFER_STREAM_HEADER;
    buffers[0].pvBuffer = wire;
    buffers[0].cbBuffer = entry->schannel_sizes.cbHeader;
    buffers[1].BufferType = SECBUFFER_DATA;
    buffers[1].pvBuffer = wire + entry->schannel_sizes.cbHeader;
    buffers[1].cbBuffer = (unsigned long)chunk;
    buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;
    buffers[2].pvBuffer = wire + entry->schannel_sizes.cbHeader + chunk;
    buffers[2].cbBuffer = entry->schannel_sizes.cbTrailer;
    buffers[3].BufferType = SECBUFFER_EMPTY;
    desc.ulVersion = SECBUFFER_VERSION;
    desc.cBuffers = 4;
    desc.pBuffers = buffers;

    status = EncryptMessage(&entry->schannel_ctx, 0, &desc, 0);
    if (status != SEC_E_OK) {
        free(wire);
        rxvm_socket_entry_schannel_error(entry, RXSOCK_ERR_OS, status,
                                         "TLS send failed");
        return -1;
    }

    encrypted_len = (size_t)buffers[0].cbBuffer +
                    (size_t)buffers[1].cbBuffer +
                    (size_t)buffers[2].cbBuffer;
    if (rxvm_socket_schannel_send_all_fd(entry, wire, encrypted_len,
                                         "TLS send failed") != 0) {
        free(wire);
        return -1;
    }

    free(wire);
    if (sent_out) *sent_out = chunk;
    rxvm_socket_entry_ok(entry);
    return 0;
}

static rxinteger rxvm_socket_schannel_recv(rxvm_socket_entry *entry,
                                           char *buffer,
                                           size_t max_bytes) {
    size_t copied;

    if (!entry->schannel_have_ctx) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_STATE, 0,
                                 "SChannel TLS connection is not active");
        return RXSOCK_ERR_TLS_STATE;
    }
    if (!entry->blocking) {
        rxvm_socket_entry_status(entry, RXSOCK_WOULDBLOCK, 0,
                                 "SChannel TLS requires blocking socket mode");
        return RXSOCK_WOULDBLOCK;
    }

    copied = rxvm_socket_schannel_copy_decrypted(entry, buffer, max_bytes);
    if (copied > 0) {
        rxvm_socket_entry_ok(entry);
        return (rxinteger)copied;
    }

    for (;;) {
        SecBuffer buffers[4];
        SecBufferDesc desc;
        SecBuffer *data_buffer;
        SecBuffer *extra_buffer;
        SECURITY_STATUS status;

        if (entry->schannel_encrypted_len == 0) {
            int received = rxvm_socket_schannel_read_encrypted(entry,
                                                               &entry->schannel_encrypted,
                                                               &entry->schannel_encrypted_len,
                                                               "TLS receive failed");
            if (received <= 0) {
                return received == 0 ? 0 : entry->last_status;
            }
        }

        memset(buffers, 0, sizeof(buffers));
        buffers[0].BufferType = SECBUFFER_DATA;
        buffers[0].pvBuffer = entry->schannel_encrypted;
        buffers[0].cbBuffer = (unsigned long)entry->schannel_encrypted_len;
        buffers[1].BufferType = SECBUFFER_EMPTY;
        buffers[2].BufferType = SECBUFFER_EMPTY;
        buffers[3].BufferType = SECBUFFER_EMPTY;
        desc.ulVersion = SECBUFFER_VERSION;
        desc.cBuffers = 4;
        desc.pBuffers = buffers;

        status = DecryptMessage(&entry->schannel_ctx, &desc, 0, 0);
        if (status == SEC_E_INCOMPLETE_MESSAGE) {
            int received = rxvm_socket_schannel_read_encrypted(entry,
                                                               &entry->schannel_encrypted,
                                                               &entry->schannel_encrypted_len,
                                                               "TLS receive failed");
            if (received <= 0) return received == 0 ? 0 : entry->last_status;
            continue;
        }
        if (status == SEC_I_CONTEXT_EXPIRED) {
            rxvm_socket_schannel_preserve_encrypted_extra(entry, 0);
            rxvm_socket_entry_status(entry, RXSOCK_EOF, 0, "TLS connection closed");
            return 0;
        }
        if (status == SEC_I_RENEGOTIATE) {
            rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_STATE, 0,
                                     "TLS renegotiation is not supported");
            return RXSOCK_ERR_TLS_STATE;
        }
        if (status != SEC_E_OK) {
            rxvm_socket_entry_schannel_error(entry, RXSOCK_ERR_OS, status,
                                             "TLS receive failed");
            return RXSOCK_ERR_OS;
        }

        data_buffer = rxvm_socket_schannel_find_buffer(buffers, 4, SECBUFFER_DATA);
        extra_buffer = rxvm_socket_schannel_find_buffer(buffers, 4, SECBUFFER_EXTRA);
        if (!data_buffer) {
            rxvm_socket_schannel_preserve_encrypted_extra(entry, extra_buffer);
            continue;
        }

        copied = data_buffer->cbBuffer < max_bytes ? data_buffer->cbBuffer : max_bytes;
        if (data_buffer->cbBuffer > copied &&
            rxvm_socket_schannel_store_decrypted(entry,
                                                 (const char *)data_buffer->pvBuffer + copied,
                                                 data_buffer->cbBuffer - copied) != 0) {
            rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0, "out of memory");
            return RXSOCK_ERR_NO_MEMORY;
        }
        memcpy(buffer, data_buffer->pvBuffer, copied);
        rxvm_socket_schannel_preserve_encrypted_extra(entry, extra_buffer);
        rxvm_socket_entry_ok(entry);
        return (rxinteger)copied;
    }
}
#endif

static rxinteger rxvm_socket_send_bytes(rxvm_socket_entry *entry, const char *data, size_t length) {
    size_t total = 0;

    if (!entry->tls_active && rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
    }

    while (total < length) {
        int sent;
        if (entry->tls_active) {
#ifdef CREXX_TLS_OPENSSL
            size_t chunk = length - total;
            if (chunk > INT_MAX) chunk = INT_MAX;
            sent = SSL_write(entry->tls_ssl, data + total, (int)chunk);
            if (sent <= 0) {
                int ssl_error = SSL_get_error(entry->tls_ssl, sent);
                if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
                    rxvm_socket_entry_status(entry, RXSOCK_WOULDBLOCK, 0, "TLS operation would block");
                    return total ? (rxinteger)total : RXSOCK_WOULDBLOCK;
                }
                rxvm_socket_entry_tls_error(entry, RXSOCK_ERR_OS,
                                            entry->tls_ssl, sent, "TLS send failed");
                return total ? (rxinteger)total : RXSOCK_ERR_OS;
            }
#elif defined(CREXX_TLS_NETWORK)
            {
                size_t chunk = length - total;
                size_t network_sent = 0;
                if (chunk > INT_MAX) chunk = INT_MAX;
                if (rxvm_socket_network_send(entry, data + total, chunk, &network_sent) != 0) {
                    return total ? (rxinteger)total : entry->last_status;
                }
                sent = (int)network_sent;
            }
#elif defined(CREXX_TLS_SCHANNEL)
            {
                size_t chunk = length - total;
                size_t schannel_sent = 0;
                if (chunk > INT_MAX) chunk = INT_MAX;
                if (rxvm_socket_schannel_send(entry, data + total, chunk, &schannel_sent) != 0) {
                    return total ? (rxinteger)total : entry->last_status;
                }
                sent = (int)schannel_sent;
            }
#else
            rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_UNAVAILABLE, 0,
                                     "TLS support is not enabled in this cREXX build");
            return total ? (rxinteger)total : RXSOCK_ERR_TLS_UNAVAILABLE;
#endif
        } else {
#ifdef MSG_NOSIGNAL
            int flags = MSG_NOSIGNAL;
#else
            int flags = 0;
#endif
#ifdef _WIN32
            size_t chunk = length - total;
            if (chunk > INT_MAX) chunk = INT_MAX;
            sent = send(entry->fd, data + total, (int)chunk, flags);
            if (sent == SOCKET_ERROR) {
#else
            ssize_t rc = send(entry->fd, data + total, length - total, flags);
            if (rc < 0) {
#endif
                rxinteger status;
                const char *message;
                int err = rxvm_socket_last_errno();
                if (rxvm_socket_error_is_timeout_or_wouldblock(entry, err, &status, &message)) {
                    rxvm_socket_entry_status(entry, status, err, message);
                    return total ? (rxinteger)total : status;
                }
                rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "send failed");
                return total ? (rxinteger)total : RXSOCK_ERR_OS;
            }
#ifndef _WIN32
            sent = (int)rc;
#endif
        }
        if (sent == 0) break;
        total += (size_t)sent;
    }

    rxvm_socket_entry_ok(entry);
    return (rxinteger)total;
}

rxinteger rxvm_socket_send_string(struct rxvm_context *context, rxinteger handle, value *data) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (!data || !data->string_length) {
        rxvm_socket_entry_ok(entry);
        return 0;
    }
    return rxvm_socket_send_bytes(entry, data->string_value, data->string_length);
}

rxinteger rxvm_socket_send_binary(struct rxvm_context *context, rxinteger handle, value *data) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (!data || !data->binary_length) {
        rxvm_socket_entry_ok(entry);
        return 0;
    }
    return rxvm_socket_send_bytes(entry, data->binary_value, data->binary_length);
}

static rxinteger rxvm_socket_recv_bytes(rxvm_socket_entry *entry, char *buffer, size_t max_bytes) {
    int received;

    if (!entry->tls_active && rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
    }
    if (max_bytes == 0) {
        rxvm_socket_entry_ok(entry);
        return 0;
    }

    if (entry->tls_active) {
#ifdef CREXX_TLS_OPENSSL
        if (max_bytes > INT_MAX) max_bytes = INT_MAX;
        received = SSL_read(entry->tls_ssl, buffer, (int)max_bytes);
        if (received <= 0) {
            int ssl_error = SSL_get_error(entry->tls_ssl, received);
            if (ssl_error == SSL_ERROR_ZERO_RETURN) {
                rxvm_socket_entry_status(entry, RXSOCK_EOF, 0, "TLS connection closed");
                return 0;
            }
            if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
                rxvm_socket_entry_status(entry, RXSOCK_WOULDBLOCK, 0, "TLS operation would block");
                return RXSOCK_WOULDBLOCK;
            }
            rxvm_socket_entry_tls_error(entry, RXSOCK_ERR_OS,
                                        entry->tls_ssl, received, "TLS receive failed");
            return RXSOCK_ERR_OS;
        }
        rxvm_socket_entry_ok(entry);
        return received;
#elif defined(CREXX_TLS_NETWORK)
        return rxvm_socket_network_recv(entry, buffer, max_bytes);
#elif defined(CREXX_TLS_SCHANNEL)
        return rxvm_socket_schannel_recv(entry, buffer, max_bytes);
#else
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_UNAVAILABLE, 0,
                                 "TLS support is not enabled in this cREXX build");
        return RXSOCK_ERR_TLS_UNAVAILABLE;
#endif
    }

#ifdef _WIN32
    if (max_bytes > INT_MAX) max_bytes = INT_MAX;
    received = recv(entry->fd, buffer, (int)max_bytes, 0);
    if (received == SOCKET_ERROR) {
#else
    {
        ssize_t rc = recv(entry->fd, buffer, max_bytes, 0);
        if (rc < 0) {
#endif
            rxinteger status;
            const char *message;
            int err = rxvm_socket_last_errno();
            if (rxvm_socket_error_is_timeout_or_wouldblock(entry, err, &status, &message)) {
                rxvm_socket_entry_status(entry, status, err, message);
                return status;
            }
            rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "recv failed");
            return RXSOCK_ERR_OS;
        }
#ifndef _WIN32
        received = (int)rc;
    }
#endif

    if (received == 0) {
        rxvm_socket_entry_status(entry, RXSOCK_EOF, 0, "connection closed");
        return 0;
    }

    rxvm_socket_entry_ok(entry);
    return received;
}

rxinteger rxvm_socket_recv_string(struct rxvm_context *context, value *out, rxinteger handle, rxinteger max_bytes) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    char *buffer;
    rxinteger received;

    if (!out) return RXSOCK_ERR_ARGUMENT;
    set_null_string(out, "");
    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (max_bytes < 0) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0, "invalid receive size");
        return RXSOCK_ERR_ARGUMENT;
    }
    if (max_bytes == 0) {
        rxvm_socket_entry_ok(entry);
        return 0;
    }

    buffer = malloc((size_t)max_bytes);
    if (!buffer) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0, "out of memory");
        return RXSOCK_ERR_NO_MEMORY;
    }

    received = rxvm_socket_recv_bytes(entry, buffer, (size_t)max_bytes);
    if (received > 0 && set_string_validated(out, buffer, (size_t)received) != 0) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0, "received text is not valid UTF-8");
        free(buffer);
        return RXSOCK_ERR_ARGUMENT;
    }
    free(buffer);
    return received < 0 ? received : received;
}

rxinteger rxvm_socket_recv_binary(struct rxvm_context *context, value *out, rxinteger handle, rxinteger max_bytes) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    rxinteger received;

    if (!out) return RXSOCK_ERR_ARGUMENT;
    if (out->native_payload_ops) clear_binary_payload(out);
    out->binary_length = 0;
    out->binary_pos = 0;
    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (max_bytes < 0) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_ARGUMENT, 0, "invalid receive size");
        return RXSOCK_ERR_ARGUMENT;
    }
    if (max_bytes == 0) {
        rxvm_socket_entry_ok(entry);
        return 0;
    }

    if (reserve_binary_buffer(out, (size_t)max_bytes) != 0) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0, "out of memory");
        return RXSOCK_ERR_NO_MEMORY;
    }

    received = rxvm_socket_recv_bytes(entry, out->binary_value, (size_t)max_bytes);
    if (received > 0) {
        out->binary_length = (size_t)received;
        out->binary_pos = 0;
        clear_vm_private_flags(out);
    }
    return received < 0 ? received : received;
}

rxinteger rxvm_socket_pending(struct rxvm_context *context, rxinteger handle) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    unsigned long count = 0;

    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (!entry->tls_active && rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
    }

    if (entry->tls_active) {
#ifdef CREXX_TLS_OPENSSL
        int pending = SSL_pending(entry->tls_ssl);
        if (pending < 0) {
            rxvm_socket_entry_tls_error(entry, RXSOCK_ERR_OS,
                                        entry->tls_ssl, pending, "TLS pending-byte query failed");
            return RXSOCK_ERR_OS;
        }
        rxvm_socket_entry_ok(entry);
        return (rxinteger)pending;
#elif defined(CREXX_TLS_NETWORK)
        rxvm_socket_entry_ok(entry);
        return 0;
#elif defined(CREXX_TLS_SCHANNEL)
        if (entry->schannel_decrypted &&
            entry->schannel_decrypted_len > entry->schannel_decrypted_pos) {
            count = (unsigned long)(entry->schannel_decrypted_len -
                                    entry->schannel_decrypted_pos);
        }
        rxvm_socket_entry_ok(entry);
        return (rxinteger)count;
#else
        rxvm_socket_entry_status(entry, RXSOCK_ERR_TLS_UNAVAILABLE, 0,
                                 "TLS support is not enabled in this cREXX build");
        return RXSOCK_ERR_TLS_UNAVAILABLE;
#endif
    }

#ifdef _WIN32
    if (ioctlsocket(entry->fd, FIONREAD, &count) != 0) {
#else
    {
        int local_count = 0;
        if (ioctl(entry->fd, FIONREAD, &local_count) != 0) {
#endif
            rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "pending-byte query failed");
            return RXSOCK_ERR_OS;
#ifndef _WIN32
        }
        count = (unsigned long)local_count;
    }
#else
    }
#endif

    rxvm_socket_entry_ok(entry);
    return (rxinteger)count;
}

rxinteger rxvm_socket_timeout(struct rxvm_context *context, rxinteger handle, rxinteger timeout_ms) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);

    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (timeout_ms < 0) timeout_ms = 0;
    if (timeout_ms > INT_MAX) timeout_ms = INT_MAX;
    entry->timeout_ms = (int)timeout_ms;
    if (rxvm_socket_apply_timeout(entry) != 0) {
        rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "timeout setup failed");
        return RXSOCK_ERR_OS;
    }
    rxvm_socket_entry_ok(entry);
    return RXSOCK_OK;
}

rxinteger rxvm_socket_blocking(struct rxvm_context *context, rxinteger handle, rxinteger blocking) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);

    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    entry->blocking = blocking ? 1 : 0;
    if (rxvm_socket_apply_blocking(entry) != 0) {
        rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "blocking-mode setup failed");
        return RXSOCK_ERR_OS;
    }
    rxvm_socket_entry_ok(entry);
    return RXSOCK_OK;
}

rxinteger rxvm_socket_nodelay(struct rxvm_context *context, rxinteger handle, rxinteger enable) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    int flag = enable ? 1 : 0;

    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
    }
    if (setsockopt(entry->fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&flag, sizeof(flag)) != 0) {
        rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "TCP_NODELAY setup failed");
        return RXSOCK_ERR_OS;
    }
    rxvm_socket_entry_ok(entry);
    return RXSOCK_OK;
}

rxinteger rxvm_socket_keepalive(struct rxvm_context *context, rxinteger handle, rxinteger enable) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    int flag = enable ? 1 : 0;

    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    if (rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
    }
    if (setsockopt(entry->fd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&flag, sizeof(flag)) != 0) {
        rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "SO_KEEPALIVE setup failed");
        return RXSOCK_ERR_OS;
    }
    rxvm_socket_entry_ok(entry);
    return RXSOCK_OK;
}

static rxinteger rxvm_socket_endpoint(rxvm_socket_entry *entry, value *out, int peer) {
    struct sockaddr_storage addr;
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    char endpoint[NI_MAXHOST + NI_MAXSERV + 4];
#ifdef _WIN32
    int addr_len = sizeof(addr);
#else
    socklen_t addr_len = sizeof(addr);
#endif

    set_null_string(out, "");
    if (rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
    }

    if ((peer ? getpeername(entry->fd, (struct sockaddr *)&addr, &addr_len)
              : getsockname(entry->fd, (struct sockaddr *)&addr, &addr_len)) != 0) {
        rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, peer ? "getpeername failed" : "getsockname failed");
        return RXSOCK_ERR_OS;
    }

    if (getnameinfo((struct sockaddr *)&addr, addr_len, host, sizeof(host),
                    service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_OS, 0, "endpoint formatting failed");
        return RXSOCK_ERR_OS;
    }

    snprintf(endpoint, sizeof(endpoint), "%s:%s", host, service);
    set_null_string(out, endpoint);
    rxvm_socket_entry_ok(entry);
    return RXSOCK_OK;
}

rxinteger rxvm_socket_peer(struct rxvm_context *context, value *out, rxinteger handle) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    if (!out) return RXSOCK_ERR_ARGUMENT;
    if (!entry) {
        set_null_string(out, "");
        return RXSOCK_ERR_INVALID_HANDLE;
    }
    return rxvm_socket_endpoint(entry, out, 1);
}

rxinteger rxvm_socket_local(struct rxvm_context *context, value *out, rxinteger handle) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    if (!out) return RXSOCK_ERR_ARGUMENT;
    if (!entry) {
        set_null_string(out, "");
        return RXSOCK_ERR_INVALID_HANDLE;
    }
    return rxvm_socket_endpoint(entry, out, 0);
}

rxinteger rxvm_socket_status(struct rxvm_context *context, rxinteger handle) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    if (!entry) return RXSOCK_ERR_INVALID_HANDLE;
    return entry->last_status;
}

rxinteger rxvm_socket_error(struct rxvm_context *context, value *out, rxinteger handle) {
    rxvm_socket_entry *entry;

    if (!out) return RXSOCK_ERR_ARGUMENT;
    entry = rxvm_socket_lookup(context, handle);
    if (!entry) {
        set_null_string(out, "-1 invalid socket handle");
        return RXSOCK_ERR_INVALID_HANDLE;
    }
    set_null_string(out, entry->last_error);
    return RXSOCK_OK;
}
