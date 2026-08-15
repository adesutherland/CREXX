# cREXX Level G Concurrent HTTP Library

`http.crexx` is the Gate F Level G HTTP/1.1 client built from the common task,
channel and byte-endpoint surface. It does not add an HTTP RXAS instruction or
an HTTP channel-provider type. Each connection is owned by one long-lived
`.taskwork` execution, and socket integers never cross worker boundaries.

This document describes the F1g-B owner/reuse slice. The later F1g policy and
streaming slices add configurable headers, redirect/retry rules, finer-grained
deadlines and explicitly streamed request/response bodies. The synchronous
Level B `rxhttp` client remains available and source compatible.

## Import and runtime images

```rexx
options levelg
import rxfnsg
```

Direct VM runs load `library.rxbin`, `classlib.rxbin` and `rxfnsg.rxbin`.
`rxfnsg` exposes the new `.httpclient` and `.httpresponse`; code that also
imports legacy `rxhttp` can write `.rxhttp..httpclient` when it needs that
synchronous interface.

## Pooled client

```rexx
client = .httpclient.pooled("https://api.example", 8, 32, 1048576)
scope = .taskscope.failfast(.taskpool.local(4, 64), 30000)

do parallel using scope
  generation = client.post("/generate", generation_body, "application/json")
  embedding = client.post("/embed", embedding_body, "application/json")
end

say generation.status()
say embedding.status()
call client.close()
```

The factory arguments are:

- `origin`: an `http://` or `https://` origin, with an optional explicit port;
- `connections`: `1..32` single-owner reusable connections;
- `admission`: `1..256` queued request descriptors;
- `maximum_response`: `1..8388608` buffered response-body bytes.

`post(path, body, content_type)` is a Level G task method. Calls participate in
ordinary task expressions and `DO PARALLEL`; the caller chooses task-pool
capacity independently of HTTP connection-pool capacity. F1g-B supports POST
with a scalar body and content type. Per-request header collections are part of
the next policy slice.

## Ownership and backpressure

The client creates one bounded type-4 admission endpoint and one `.taskwork`
connection owner per configured connection. Each request creates bounded
request and response endpoints, then writes one canonical fixed-size admission
descriptor containing only provider references and the request length. Socket
handles and live VM objects are never encoded into that descriptor.

An owner reads one request at a time, reuses its connection while HTTP permits,
and materializes an independent `.httpresponse` in the caller. A connection
pool of one therefore serializes network ownership even when several caller
tasks are ready. The admission endpoint applies bounded backpressure before
unbounded work can accumulate.

Only the controller-owned `.httpclient.close()` closes admission, joins the
owners and closes their pool. A per-call transferable proxy must not close that
shared endpoint when its request completes.

## Response API

`.httpresponse` is a concrete transferable class with:

- `transport_status()`: `0` when transport and framing succeeded, otherwise a
  negative client status;
- `status()`: parsed HTTP status, or `0` when no valid response status exists;
- `succeeded()`: true only for transport success and HTTP `2xx`;
- `body()`: buffered response body;
- `header(name)`: case-insensitive response-header lookup;
- `error()`: transport/protocol diagnostic or a non-2xx status diagnostic.

The worker transfers canonical RXCV produced by `to_channel()`; the controller
constructs a new object through `from_channel()`. No response object identity
or socket state crosses a worker boundary.

F1g-B handles fixed `Content-Length` and chunked response framing and asks the
server for `Accept-Encoding: identity`. It preserves a bounded response-body
ceiling and a bounded header allowance. Compressed bodies, redirects, retry
classification and stream-returning APIs are deliberately not claimed by this
slice.

## TLS and verification

An `https://` origin routes through `socketconnecttls(sock, host, port)`. The VM
core TLS backend performs certificate-chain and hostname verification using the
platform trust store: Network.framework/Security.framework on Apple platforms,
OpenSSL verification paths on supported Unix-like platforms, and SChannel on
Windows. A build without a TLS backend reports a transport failure; it does not
downgrade an HTTPS origin to plaintext.

## Regression proof

`lib/rxfnsg/tests_functional/ts_http_pooled.crexx` is a pure-cREXX loopback
fixture. Its server is `.taskwork`; one client owner serves two parallel pairs,
and the server must report one accepted socket and four requests. CTest compiles
and assembles optimized and unoptimized callers, links the optimized forms, and
runs both `rxbvm` and `rxtvm`.
