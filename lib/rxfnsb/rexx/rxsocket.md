# `rxsocket` Core TCP Library

`rxsocket.rexx` is the Level B wrapper around the VM's core socket
instructions. It is built into `library.rxbin`, so scripts can `import
rxsocket` without loading the older dynamic socket plugin. That deprecated
OpenSSL-backed plugin is now build-time opt-in via
`CREXX_BUILD_LEGACY_SOCKET_PLUGIN=ON`.

The implementation uses the operating system socket API directly: POSIX sockets
on Unix-like systems and Winsock2 on Windows. Client TLS is provided by the VM
socket TLS backend when one is selected.

## Handles and Status

`socketcreate()` returns a positive VM-managed socket handle. Handles are not
OS file descriptors and should only be used with `rxsocket` functions. The VM
closes any live handles when the VM context is freed, but programs should still
call `socketclose()` when finished.

Status codes are:

- `0`: success
- `1`: EOF / peer closed
- `2`: timeout
- `3`: operation would block
- negative values: invalid handle, allocation failure, OS error, DNS failure,
  bad argument, socket-not-open, or TLS setup/availability failure

Use `socketstatus(sock)` after operations that do not directly return a status,
and `socketerror(sock)` for a short diagnostic string.

## API

- `socketcreate() = .int`
- `socketclose(sock) = .int`
- `socketconnect(sock, host, port) = .int`
- `socketconnecttls(sock, host, port) = .int`
- `socketbind(sock, host, port) = .int`
- `socketlisten(sock, backlog) = .int`
- `socketaccept(sock) = .int`
- `socketshutdown(sock, how) = .int`
- `socketsend(sock, data) = .int`
- `socketsendb(sock, data) = .int`
- `socketrecv(sock, maxbytes) = .string`
- `socketrecvb(sock, maxbytes) = .binary`
- `socketpending(sock) = .int`
- `sockettimeout(sock, milliseconds) = .int`
- `socketblocking(sock, enabled) = .int`
- `socketnodelay(sock, enabled) = .int`
- `socketkeepalive(sock, enabled) = .int`
- `socketpeer(sock) = .string`
- `socketlocal(sock) = .string`
- `socketstatus(sock) = .int`
- `socketerror(sock) = .string`

`socketshutdown()` uses the portable mode values used by the VM instruction:
`0` for receive, `1` for send, and `2` for both.

`socketconnecttls()` connects to `host:port` and starts client TLS before any
application bytes are exchanged, using `host` for SNI and certificate hostname
verification. After a successful secure connect, the existing `socketsend()`
and `socketrecv()` calls operate over the encrypted stream. In builds without a
TLS backend, the instruction still exists and fails with a negative status so
programs can probe capability with `socketstatus()` / `socketerror()`.

The public Level B wrapper does not expose true STARTTLS. A lower-level RXAS
instruction exists for future protocol-specific libraries that need to exchange
clear-text bytes before TLS, but backends may return unsupported if they cannot
upgrade an existing connection in place.

TLS is selected at CMake configure time. Fresh build directories default to
`NETWORK` on Apple platforms, `OPENSSL` on non-Windows Unix-like platforms, and
`SCHANNEL` on Windows:

```sh
cmake -S . -B cmake-build-tls -DCREXX_ENABLE_TLS=OPENSSL
cmake -S . -B cmake-build-tls-network -DCREXX_ENABLE_TLS=NETWORK
cmake -S . -B cmake-build-tls-windows -DCREXX_ENABLE_TLS=SCHANNEL
cmake -S . -B cmake-build-notls -DCREXX_ENABLE_TLS=OFF
```

`NETWORK` is the macOS backend. It links against Apple's Network.framework,
Security.framework, and CoreFoundation.framework, and uses the operating system
trust store, so the VM binaries do not acquire an OpenSSL runtime dependency.
It implements `socketconnecttls()` as a native secure connection from byte zero.
The lower-level true STARTTLS instruction reports unsupported on this backend.

`OPENSSL` is the portable backend for platforms where system TLS is not wired in
yet, such as Linux. It uses OpenSSL's default verification paths and performs
hostname verification.

`SCHANNEL` is the Windows backend. It uses SChannel/SSPI with the Windows trust
store and hostname verification, so Windows VM binaries do not need OpenSSL for
core socket TLS.

Use `-DCREXX_TLS_STATIC_OPENSSL=ON` when the OpenSSL package and platform
toolchain support static linking. This option only applies to
`CREXX_ENABLE_TLS=OPENSSL`.

`socketlocal()` and `socketpeer()` return numeric endpoint text in
`host:port` form. IPv6 addresses are currently emitted in the same simple form,
so higher-level parsers should treat the final colon-separated field as the
port if they need to split it.

## Loopback Example

```rexx
options levelb
import rxsocket

host = "127.0.0.1"
port = 32164

server = socketcreate()
if socketbind(server, host, port) <> 0 then exit 1
if socketlisten(server, 5) <> 0 then exit 1

client = socketcreate()
if socketconnect(client, host, port) <> 0 then exit 1

accepted = socketaccept(server)
if accepted <= 0 then exit 1

if socketsend(client, "ping") <> 4 then exit 1
if socketrecv(accepted, 4) <> "ping" then exit 1

drop_rc = socketclose(client)
drop_rc = socketclose(accepted)
drop_rc = socketclose(server)
```

The functional coverage lives in `lib/rxfnsb/tests_functional/ts_rxsocket.rexx`.
`lib/rxfnsb/tests_functional/ts_socket_tls_live.rexx` is a default-skipped live
TLS handshake smoke test; set `CREXX_TLS_LIVE_SMOKE=1` to enable it on a runner
with network access. The lower-level RXAS instruction coverage lives in
`interpreter/tests/tests_socket.rxas`.
