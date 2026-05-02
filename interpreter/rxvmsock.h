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

#ifndef CREXX_RXVMSOCK_H
#define CREXX_RXVMSOCK_H

#include "rxvalue.h"

struct rxvm_context;

void rxvm_socket_free_registry(struct rxvm_context *context);

rxinteger rxvm_socket_new(struct rxvm_context *context);
rxinteger rxvm_socket_close(struct rxvm_context *context, rxinteger handle);
rxinteger rxvm_socket_connect(struct rxvm_context *context, rxinteger handle, value *host, rxinteger port);
rxinteger rxvm_socket_connect_tls(struct rxvm_context *context, rxinteger handle, value *host, rxinteger port);
rxinteger rxvm_socket_bind(struct rxvm_context *context, rxinteger handle, value *host, rxinteger port);
rxinteger rxvm_socket_listen(struct rxvm_context *context, rxinteger handle, rxinteger backlog);
rxinteger rxvm_socket_accept(struct rxvm_context *context, rxinteger server_handle);
rxinteger rxvm_socket_shutdown(struct rxvm_context *context, rxinteger handle, rxinteger how);
rxinteger rxvm_socket_starttls(struct rxvm_context *context, rxinteger handle, value *host);

rxinteger rxvm_socket_send_string(struct rxvm_context *context, rxinteger handle, value *data);
rxinteger rxvm_socket_send_binary(struct rxvm_context *context, rxinteger handle, value *data);
rxinteger rxvm_socket_recv_string(struct rxvm_context *context, value *out, rxinteger handle, rxinteger max_bytes);
rxinteger rxvm_socket_recv_binary(struct rxvm_context *context, value *out, rxinteger handle, rxinteger max_bytes);

rxinteger rxvm_socket_pending(struct rxvm_context *context, rxinteger handle);
rxinteger rxvm_socket_timeout(struct rxvm_context *context, rxinteger handle, rxinteger timeout_ms);
rxinteger rxvm_socket_blocking(struct rxvm_context *context, rxinteger handle, rxinteger blocking);
rxinteger rxvm_socket_nodelay(struct rxvm_context *context, rxinteger handle, rxinteger enable);
rxinteger rxvm_socket_keepalive(struct rxvm_context *context, rxinteger handle, rxinteger enable);

rxinteger rxvm_socket_peer(struct rxvm_context *context, value *out, rxinteger handle);
rxinteger rxvm_socket_local(struct rxvm_context *context, value *out, rxinteger handle);
rxinteger rxvm_socket_status(struct rxvm_context *context, rxinteger handle);
rxinteger rxvm_socket_error(struct rxvm_context *context, value *out, rxinteger handle);

#endif /* CREXX_RXVMSOCK_H */
