# CREXX/PA Socket Module Documentation

This module provides a cross-platform, portable, and script-friendly TCP client socket API for use from Rexx or C. All functions are available on Windows and Linux/Unix, abstracting away OS differences.

---

## Table of Contents

- [Overview](#overview)
- [API Quick Reference](#api-quick-reference)
- [Usage Example](#usage-example)
- [Function Details](#function-details)
    - [socketcreate](#socketcreate)
    - [socketconnect](#socketconnect)
    - [socketsend](#socketsend)
    - [socketsendall](#socketsendall)
    - [socketrecv](#socketrecv)
    - [socketrecvline](#socketrecvline)
    - [socketclose](#socketclose)
    - [socketlasterror](#socketlasterror)
    - [socketsettimeout](#socketsettimeout)
    - [socketgettimeout](#socketgettimeout)
    - [socketlocalinfo](#socketlocalinfo)
    - [socketshutdown](#socketshutdown)
    - [socketnodelay](#socketnodelay)
    - [socketkeepalive](#socketkeepalive)
- [Constants and Return Codes](#constants-and-return-codes)
- [Error Handling](#error-handling)
- [Typical Patterns](#typical-patterns)

---

## Overview

- **TCP client sockets**: connect to servers, send/receive data, handle lines or binary.
- Works on **Windows and Linux**.
- Uses integer socket tokens (handles), so you can manage multiple connections.
- All options (timeouts, nodelay, keepalive, shutdown) are controllable.

---

## API Quick Reference

| Function            | Description                                 |
|---------------------|---------------------------------------------|
| socketcreate        | Create a socket handle (token)              |
| socketconnect       | Connect to host/port                        |
| socketsend          | Send bytes (may be partial)                 |
| socketsendall       | Send all bytes (retries until done)         |
| socketrecv          | Receive up to N bytes                       |
| socketrecvline      | Receive a line (\n-terminated or until EOF) |
| socketclose         | Close socket and free handle                |
| socketlasterror     | Get last error message                      |
| socketsettimeout    | Set per-socket timeout (ms)                 |
| socketgettimeout    | Get per-socket timeout (ms)                 |
| socketlocalinfo     | Return local IP:port string                 |
| socketshutdown      | Shutdown read, write, or both               |
| socketnodelay       | Toggle TCP_NODELAY (Nagle's algorithm)      |
| socketkeepalive     | Toggle SO_KEEPALIVE                         |

---

## Usage Example

```rexx
-- Rexx Example: HTTP GET
s = socketCreate()
call socketConnect s, "example.com", 80
call socketSetTimeout s, 2000         -- 2s timeout for all operations
call socketNoDelay s, 1               -- Turn off Nagle's algorithm
call socketSendAll s, 'GET / HTTP/1.0\r\n\r\n'
call socketShutdown s, 1              -- No more sends
lines = ''
do forever
  line = socketRecvLine(s)
  if line = '' then leave
  lines = lines || line || '\n'
end
say lines
call socketClose s
```

---

## Function Details

### 1. socketcreate
**Syntax:**
```rexx
token = socketCreate()
```
- Allocates and returns a socket handle (integer token).

### 2. socketconnect
**Syntax:**
```rexx
rc = socketConnect(token, host, port)
```
- Connects the socket to given host and port. Returns 0 on success, negative on error.

### 3. socketsend
**Syntax:**
```rexx
rc = socketSend(token, data)
```
- Sends `data` string (may not send all bytes, use `socketSendAll` for complete transmission). Returns number of bytes sent.

### 4. socketsendall
**Syntax:**
```rexx
rc = socketSendAll(token, data)
```
- Sends *all* bytes in `data`, retrying partial sends as needed. Returns total bytes sent or negative error code.

### 5. socketrecv
**Syntax:**
```rexx
data = socketRecv(token, nbytes)
```
- Receives up to `nbytes` bytes as string (binary-safe). Returns '' on EOF or error.

### 6. socketrecvline
**Syntax:**
```rexx
line = socketRecvLine(token)
```
- Receives a line (up to newline or buffer full). Returns '' on EOF or error.

### 7. socketclose
**Syntax:**
```rexx
call socketClose token
```
- Closes socket and frees resources. Safe to call multiple times.

### 8. socketlasterror
**Syntax:**
```rexx
msg = socketLastError(token)
```
- Returns last error message and code.

### 9. socketsettimeout / socketgettimeout
**Syntax:**
```rexx
call socketSetTimeout token, ms
ms = socketGetTimeout(token)
```
- Sets/gets default receive timeout in milliseconds for all operations.

### 10. socketlocalinfo
**Syntax:**
```rexx
str = socketLocalInfo(token)
```
- Returns "ip:port" of the local (client) endpoint as a string.

### 11. socketshutdown
**Syntax:**
```rexx
call socketShutdown token, how
```
- `how=0`: shut down reading (no more recv)
- `how=1`: shut down writing (no more send)
- `how=2`: shut down both (send FIN)

### 12. socketnodelay
**Syntax:**
```rexx
call socketNoDelay token, onoff
```
- Enables (`1`) or disables (`0`) TCP_NODELAY (Nagle's algorithm).

### 13. socketkeepalive
**Syntax:**
```rexx
call socketKeepAlive token, onoff
```
- Enables (`1`) or disables (`0`) SO_KEEPALIVE for TCP connections.

---

## Constants and Return Codes

- All functions return 0 on success, negative values on error.
- `socketRecvLine` and `socketRecv` return '' (empty string) on EOF or error.
- `socketShutdown` how-values:
    - 0 = shutdown read
    - 1 = shutdown write
    - 2 = shutdown both

---

## Error Handling

- On error, use `socketLastError(token)` to get the last code and message.
- All errors are also reported as negative return values for script logic.

---

## Typical Patterns

- Set timeout after connect for reliability:
  ```rexx
  call socketSetTimeout token, 2000
  ```
- Always close the socket when done:
  ```rexx
  call socketClose token
  ```
- For low latency, enable TCP_NODELAY:
  ```rexx
  call socketNoDelay token, 1
  ```

---

## Authors & Credits

- Original C scaffolding: [Your Name/Team]
- Socket API & helpers: ChatGPT & [You]

---

## License

Specify your preferred license here (MIT, BSD, Apache, etc).

---

**Happy scripting!**

