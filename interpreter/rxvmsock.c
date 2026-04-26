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

static void rxvm_socket_close_entry_fd(rxvm_socket_entry *entry) {
    if (!entry) return;
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
    snprintf(port_text, sizeof(port_text), "%ld", (long)port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    rc = getaddrinfo(host, port_text, &hints, &res);
    free(host);
    if (rc != 0 || !res) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_DNS, rc, "name resolution failed");
        return RXSOCK_ERR_DNS;
    }

    for (ai = res; ai; ai = ai->ai_next) {
        rxvm_socket_fd fd = rxvm_socket_open_addrinfo(entry, ai);
        if (rxvm_socket_is_invalid(fd)) continue;
        if (connect(fd, ai->ai_addr, (socklen_t)ai->ai_addrlen) == 0) {
            entry->is_server = 0;
            rxvm_socket_entry_ok(entry);
            freeaddrinfo(res);
            return RXSOCK_OK;
        }
        {
            rxinteger status;
            const char *message;
            int err = rxvm_socket_last_errno();
            if (rxvm_socket_error_is_timeout_or_wouldblock(entry, err, &status, &message)) {
                rxvm_socket_entry_status(entry, status, err, message);
                freeaddrinfo(res);
                return status;
            }
            rxvm_socket_entry_os_error(entry, RXSOCK_ERR_OS, "connect failed");
            rxvm_socket_close_entry_fd(entry);
        }
    }

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
    if (rxvm_socket_is_invalid(entry->fd)) {
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

static rxinteger rxvm_socket_send_bytes(rxvm_socket_entry *entry, const char *data, size_t length) {
    size_t total = 0;

    if (rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
    }

    while (total < length) {
        int sent;
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

    if (rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
    }
    if (max_bytes == 0) {
        rxvm_socket_entry_ok(entry);
        return 0;
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
    if (rxvm_socket_is_invalid(entry->fd)) {
        rxvm_socket_entry_status(entry, RXSOCK_ERR_NOT_OPEN, 0, "socket is not open");
        return RXSOCK_ERR_NOT_OPEN;
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
