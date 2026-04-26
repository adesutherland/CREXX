# `rxsocket` Core TCP Library

`rxsocket.rexx` is the Level B wrapper around the VM's core socket
instructions. It is built into `library.rxbin`, so scripts can `import
rxsocket` without loading the older dynamic socket plugin.

The implementation is raw TCP. It deliberately avoids TLS, HTTP parsing, and
third-party libraries so a clean install can use the operating system socket
API directly: POSIX sockets on Unix-like systems and Winsock2 on Windows.

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
  bad argument, or socket-not-open

Use `socketstatus(sock)` after operations that do not directly return a status,
and `socketerror(sock)` for a short diagnostic string.

## API

- `socketcreate() = .int`
- `socketclose(sock) = .int`
- `socketconnect(sock, host, port) = .int`
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
The lower-level RXAS instruction coverage lives in
`interpreter/tests/tests_socket.rxas`.
