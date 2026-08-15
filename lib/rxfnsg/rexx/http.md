# cREXX Level G Concurrent HTTP Library

`http.crexx` is the Gate F HTTP/1.1 client built from the common task, channel
and byte-endpoint surface. HTTP is a Rexx library, not an RXAS instruction or a
special channel-provider type. Each reusable socket belongs to one long-lived
`.taskwork` owner, and socket integers never cross worker boundaries.

F1g-C supplies bounded headers and request policy, verified HTTPS, explicit
redirect/retry rules and ambiguity diagnostics. F1g-D still owns explicit
request/response streaming, compressed response decoding and the retained
`crexx-rag` generation/embedding fixture. The synchronous Level B `rxhttp`
client remains available and source compatible.

## Import and first request

```rexx
options levelg
import rxfnsg

policy = .httppolicy.safe()
call policy.set_timeouts(10000, 10000, 10000, 30000, 30000, 60000)

headers = .httpheaders.json()
call headers.add("Authorization", "Bearer " || token)

client = .httpclient.pooled("https://api.example", 8, 32, 1048576, policy)
scope = .taskscope.failfast(.taskpool.local(4, 64), 30000)

do parallel using scope
  generation = client.post("/generate", generation_body, headers)
  embedding = client.post("/embed", embedding_body, headers)
end

say generation.status()
say embedding.status()
call client.close()
```

`post(path, body, headers)` is a Level G task method. It participates in normal
task expressions and `DO PARALLEL`; the task pool bounds Rexx executions while
the HTTP pool independently bounds connections and admission.

The `pooled` factory accepts:

- `origin`: an `http://` or `https://` origin, optionally with an explicit
  port; IPv6 literals use bracketed authority syntax;
- `connections`: `1..32` single-owner reusable connections;
- `admission`: `1..256` queued fixed-size request descriptors;
- `maximum_response`: `1..8388608` buffered decoded response bytes; and
- optional `policy`: a `.httppolicy`, defaulting to `.httppolicy.safe()`.

The request target must be an origin-relative path beginning `/`, no larger
than 8192 UTF-8 bytes and free of CR/LF. F1g-C request bodies are buffered
`.string` values; binary and streamed bodies belong to F1g-D.

## Safe headers

Use `.httpheaders.empty()`, `.httpheaders.json()` or
`.httpheaders.media_type(value)`, then add fields fluently or with `CALL`:

```rexx
headers = .httpheaders.json()
call headers.add("Authorization", "Bearer " || token)
call headers.add("Idempotency-Key", request_id)
```

Names must be HTTP tokens. Values and `Content-Type` cannot contain CR or LF.
`Host`, `Content-Length`, `Connection`, `Transfer-Encoding` and
`Accept-Encoding` are library-owned. Every transferred header snapshot is
validated again when reconstructed, so forged RXCV cannot bypass those rules.

## Policy and deadlines

`.httppolicy.safe()` defaults to:

| Policy | Default |
| --- | ---: |
| DNS, connect, TLS | 10000 ms each |
| request, response | 30000 ms each |
| controller completion observation | 60000 ms |
| custom request/response fields | 64 |
| request/response header bytes | 65536 |
| buffered request body | 8388608 bytes |
| redirects, retries | 0 |

Configure with:

```rexx
call policy.set_timeouts(dns, connect, tls, request, response, total)
call policy.set_limits(header_count, header_bytes, request_bytes)
call policy.set_replay(max_redirects, max_retries)
```

All values are validated before mutation, so a rejected update does not leave
the policy partly changed. The current socket substrate performs hostname
resolution, TCP connect and TLS handshake as one synchronous owner-local
operation; F1g-C therefore applies the sum of the three configured phase
budgets to that atomic operation while retaining the separate values in the
transfer contract. Request and response socket operations use their own
budgets. `total` bounds each controller-side admission/completion observation;
the containing `.taskscope` deadline is the strict whole-task monotonic
deadline and should reflect the caller's end-to-end service objective.

## Redirects, retries and ambiguity

Automatic replay is off by default. `set_replay()` only permits bounded replay;
it does not make an unsafe POST safe.

- connect failures before any send may be retried within the configured limit;
- post-send transport retries require a non-empty `Idempotency-Key`;
- retryable HTTP statuses are 408, 425, 429, 500, 502, 503 and 504 and require
  the same key;
- only same-origin 307/308 redirects retain POST and may be followed;
- 301/302/303 POST method rewriting is deliberately disabled; and
- cross-origin redirects are returned, never followed.

`ambiguous_outcome()` is true when bytes may have reached the peer but no
definitive response was obtained. It remains true even when an explicitly safe
retry later succeeds, preserving the complete delivery history.

## Ownership, framing and backpressure

The client creates one bounded type-4 admission endpoint and one `.taskwork`
connection owner per configured connection. A request uses bounded staging and
response endpoints, then admits one canonical 192-byte descriptor containing
two 92-byte endpoint references plus an eight-byte request length. Live VM
objects and socket handles are never encoded.

An owner consumes one request at a time and reuses its connection while HTTP
permits. F1g-C accepts fixed `Content-Length`, chunked, close-delimited and
bodyless 204/304 responses, detects early EOF for declared framing, and bounds
encoded and decoded response storage. It sends `Accept-Encoding: identity`;
a compressed response is returned explicitly as transport status `-24` until
F1g-D supplies bounded decoding.

Only the controller-owned client may call `close()` to close admission, join
all owners and release the pool. A transferred per-task proxy closes only its
view of the endpoint.

## Response API

`.httpresponse` is a concrete transferable value with:

- `transport_status()`: zero for a complete HTTP exchange, otherwise a
  negative client status;
- `status()`: parsed HTTP status or zero;
- `succeeded()`: transport success plus HTTP 2xx;
- `body()` and `header(name)`;
- `error()`;
- `attempts()` and `redirects()`; and
- `ambiguous_outcome()`.

The worker publishes canonical RXCV through `to_channel()` and the controller
constructs a new object through `from_channel()`. Object identity, endpoints
and socket state do not cross the task boundary.

## TLS and regression proof

HTTPS uses `socketconnecttls(sock, host, port)`. The platform backend verifies
the certificate chain and hostname using Security/Network.framework on Apple,
OpenSSL verification on supported Unix-like systems and SChannel on Windows.
A build without TLS support fails the request; it never downgrades to plaintext.

`ts_http_pooled.crexx` proves reusable single-owner connections and bounded
parallel admission. `ts_http_policy.crexx` covers credentials, same-origin
307, status/transport retries, ambiguity, timeout, header limits,
cross-origin refusal and bodyless 204. `ts_http_tls_live.crexx` is
environment-gated and verifies both a trusted host and hostname mismatch.
CTest compiles and assembles optimized/unoptimized images and runs `rxbvm` and
`rxtvm`.
