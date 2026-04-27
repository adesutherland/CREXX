# cREXX Level B HTTP Library

`rxhttp.rexx` provides a reusable plain-HTTP client built on the core
`rxsocket` library. It handles HTTP framing so higher layers do not need to
parse socket responses directly.

The library is deliberately HTTP-only. TLS/SSL remains out of scope until the
core TLS layer exists.

## Import

```rexx
options levelb
import rxhttp
```

## API

The namespace exposes:

- `httpclient`: provider-selecting interface
- `http`: concrete socket-backed HTTP/1.1 implementation

Factory arguments:

- `host = "127.0.0.1"`
- `port = 80`
- `timeout = 30000`

Primary methods:

- `send(verb, path, body, contentType) = .string`: sends a request and
  returns the decoded response body
- `post(path, body, contentType) = .string`: convenience wrapper for POST
- `buildRequest(method, path, body, contentType) = .string`: builds the raw
  HTTP request text
- `extractBody(response) = .string`: decodes a raw HTTP response
- `header(name) = .string`: reads a header from the last response
- `status() = .int`: `0` on success, HTTP status for non-2xx responses, and
  negative values for local socket/protocol failures
- `httpStatus() = .int`: parsed HTTP status code, or `0` if unavailable
- `error() = .string`: last diagnostic message
- `lastHttp() = .string`: last raw HTTP response
- `lastBody() = .string`: last decoded response body

## Behaviour

The request builder calculates `Content-Length` as UTF-8 bytes, not cREXX
characters. The response parser supports:

- `Content-Length` bodies, trimmed to the advertised byte count
- HTTP/1.1 `Transfer-Encoding: chunked` bodies, decoded byte-wise
- close-delimited bodies when no length framing is present
- case-insensitive header lookup
- non-2xx HTTP responses while preserving the response body for diagnostics

The client sends `Connection: close` and `Accept-Encoding: identity` so callers
do not need to handle persistent connections or compressed bodies yet.
