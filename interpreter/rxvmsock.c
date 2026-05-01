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
    if (received > 0) set_string(out, buffer, (size_t)received);
    free(buffer);
    return received < 0 ? received : received;
}

rxinteger rxvm_socket_recv_binary(struct rxvm_context *context, value *out, rxinteger handle, rxinteger max_bytes) {
    rxvm_socket_entry *entry = rxvm_socket_lookup(context, handle);
    char *buffer;
    rxinteger received;

    if (!out) return RXSOCK_ERR_ARGUMENT;
    if (out->native_payload_ops) clear_binary_payload(out);
    out->binary_length = 0;
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
    if (received > 0) {
        if (out->binary_buffer_length < (size_t)received) {
            char *new_buffer = realloc(out->binary_value, (size_t)received);
            if (!new_buffer) {
                free(buffer);
                rxvm_socket_entry_status(entry, RXSOCK_ERR_NO_MEMORY, 0, "out of memory");
                return RXSOCK_ERR_NO_MEMORY;
            }
            out->binary_value = new_buffer;
            out->binary_buffer_length = (size_t)received;
        }
        memcpy(out->binary_value, buffer, (size_t)received);
        out->binary_length = (size_t)received;
    }
    free(buffer);
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
