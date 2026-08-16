# Concurrent HTTP client and server {#library-concurrent-http}

The `rxfnsg` namespace supplies a bounded HTTP/1.1 client and a clear-text
buffered server for Level G programs. The client combines structured task
scopes, single-owner reusable connections, safe transferable headers and
endpoint-backed streaming. The server keeps sockets on one controller and
gives complete request values to ordinary task classes. HTTP is a Rexx library
built on concurrency; it is not a special RXAS instruction or channel provider
type.

The client is initial on `develop` and currently macOS-qualified. Linux
and Windows publication still requires the portable concurrency conformance
matrix.

## One public surface, one private backend

User programs import `rxfnsg` and use these public Level G types:

| Purpose | Public types | Execution model |
| --- | --- | --- |
| client | `.httpclient`, `.httpheaders`, `.httppolicy`, `.httpbody`, `.httpresponse` | task calls over bounded single-owner connections |
| server | `.httpserver`, `.httprequest`, `.httpservice`, `.httpresponse` | controller-owned sockets and sealed handler tasks |
| LLM providers | `.ollama`, `.openai`, `.anthropic`, `.gemini` | the same Level G client and owner pool |

All three use `_rxhttpcore`, one private binary-oriented Level B framing,
parsing and codec backend. It is deliberately not a second public HTTP client:
Level B supplies the socket and binary machinery, while Level G owns HTTP
policy, resource management and typed values. The former public `rxhttp`
convenience client has been removed from the pre-release surface.

## Import and first parallel requests

```rexx
options levelg comments_dash
import rxfnsb
import concurrency
import rxfnsg
```

Create headers, a per-origin client and a task scope, then use ordinary method
syntax inside `DO PARALLEL`:

```rexx
headers = .httpheaders.json()
call headers.add("Authorization", "Bearer " || token)
call headers.add("Idempotency-Key", request_id)

client = .httpclient.pooled("https://api.example", 4, 16, 1048576)
request_pool = .taskpool.local(2, 8)
request_scope = .taskscope.failfast(request_pool, 60000)

do parallel using request_scope
  generation = client.post("/v1/chat/completions", generation_json, headers)
  embedding = client.post("/v1/embeddings", embedding_json, headers)
end

if generation.succeeded() then say generation.body()
else say generation.transport_status() generation.error()

call request_pool.close()
call client.close()
```

`post()` is a task method. The task pool bounds Rexx task executions while the
HTTP client independently bounds reusable connections and queued admissions.
The complete loopback generation/embedding fixture is
[`ts_http_crexx_rag.crexx`](../../../lib/rxfnsg/tests_functional/ts_http_crexx_rag.crexx).
It verifies authorization and idempotency headers, request shapes, parallel
admission and bounded gzip/DEFLATE response decoding.

For a buffered GET use `client.get(path, headers)`. For another HTTP method or
a binary request body use
`client.request(verb, path, body_bytes, headers)`. All three forms return the
same `.httpresponse`; use `body()` for text and `body_bytes()` for exact bytes.

## Client factory and lifecycle

```rexx
client = .httpclient.pooled(origin, connections, admission, maximum_response)
client = .httpclient.pooled(origin, connections, admission, maximum_response, policy)
```

| Argument | Contract |
| --- | --- |
| `origin` | `http://` or `https://` origin; IPv6 literals use brackets |
| `connections` | `1..32` single-owner reusable connections |
| `admission` | `1..256` queued fixed-size request descriptors |
| `maximum_response` | `1..8388608` decoded bytes for buffered responses |
| `policy` | optional `.httppolicy`, default `.httppolicy.safe()` |

Request paths must begin with `/`, occupy at most 8192 UTF-8 bytes and contain
no CR or LF. Each configured connection has one long-lived `.taskwork` owner;
its socket integer never crosses an execution boundary.

Only the controller-owned client should perform final `close()`. That closes
admission, joins the connection owners and releases their pool. A client proxy
reconstructed for a task closes only its local endpoint view. Close the request
task pool separately, after every scope using it has closed.

`from_channel()` and `to_channel()` appear in the generated API because they
are the exact compiler transfer contract. Ordinary callers use `pooled()`,
`post()`, `post_stream()` and `close()`.

The same rule applies to `from_channel()` / `to_channel()` on headers, policy,
body and response objects. `httpresponse.new()`, `from_wire()`,
`from_stream_wire()` and `attach_stream()` are library construction/framing
bridges. They remain documented because they are callable in the current
pre-release class surface, but normal request code should consume responses
returned by the client.

## Safe headers

Construct a validated transferable snapshot with:

```rexx
headers = .httpheaders.empty()
headers = .httpheaders.json()
headers = .httpheaders.media_type("application/cbor")

call headers.add("Authorization", "Bearer " || token)
call headers.add("Idempotency-Key", request_id)
```

Header names must be HTTP tokens. Values and `Content-Type` cannot contain CR
or LF. `Host`, `Content-Length`, `Connection`, `Transfer-Encoding` and
`Accept-Encoding` are library-owned. The absolute snapshot ceiling is 256
custom fields and 262144 bytes. The active policy may impose smaller limits.

Useful accessors are `content_type()`, `count()`, `bytes()` and
`header(name)`. `wire()` is the validated CR/LF-terminated internal wire block.
`to_channel()` / `from_channel()` are transfer-contract members; a reconstructed
snapshot is fully validated again, so forged canonical data cannot bypass the
header rules.

## Policy, limits and deadlines

```rexx
policy = .httppolicy.safe()
call policy.set_timeouts(10000, 10000, 10000, 30000, 30000, 60000)
call policy.set_limits(64, 65536, 8388608)
call policy.set_replay(0, 0)
```

The timeout order is DNS, connect, TLS, request, response and total. The limit
order is custom-header count, header bytes and buffered request bytes. Safe
defaults are:

| Policy | Default | Valid range |
| --- | ---: | ---: |
| DNS, connect, TLS | 10000 ms each | `1..600000` |
| request, response | 30000 ms each | `1..600000` |
| total observation | 60000 ms | `1..1800000` |
| custom header fields | 64 | `1..256` |
| header bytes | 65536 | `256..262144` |
| buffered request bytes | 8388608 | `1..8388608` |
| redirects, retries | 0 | `0..8` each |

Validation happens before mutation, so a rejected setter does not leave a
partially changed policy. The present socket substrate performs name
resolution, connect and TLS handshake as one owner-local operation; the client
uses the sum of those three phase budgets for that atomic call. Request and
response socket work use their individual budgets. The containing task scope
should still express the caller's real end-to-end deadline.

## Buffered responses and compression

`post(path, body, headers)` accepts a buffered `.string` body and returns an
independent `.httpresponse`. It sends `Accept-Encoding: gzip, deflate` and
performs bounded decoding for:

- gzip;
- zlib-wrapped DEFLATE; and
- raw DEFLATE.

The decoded result may not exceed `maximum_response`. Malformed streams,
unsupported content coding and decoded-size overflow are explicit transport
failures; the client does not return partial decoded data as success.

Inspect responses as follows:

```rexx
if response.transport_status() <> 0 then do
  say "transport/protocol failure:" response.transport_status() response.error()
end
else do
  say "HTTP status:" response.status()
  say "Content-Type:" response.header("Content-Type")
  if response.succeeded() then say response.body()
end
```

`succeeded()` means transport status zero and HTTP status in `200..299`.
`attempts()` and `redirects()` report network history.
`ambiguous_outcome()` remains true when bytes may have reached the peer without
a definitive response, even if an explicitly permitted retry later succeeds.

## Redirects, retries and ambiguous POST outcomes

Automatic replay is disabled by default. `set_replay()` sets only bounded
maximums; it cannot make an unsafe POST idempotent.

- a connect failure before any bytes are sent may be retried within policy;
- a retry after sending requires a non-empty `Idempotency-Key`;
- retryable statuses are 408, 425, 429, 500, 502, 503 and 504, again requiring
  the key;
- only same-origin 307 and 308 retain POST and may be followed;
- 301, 302 and 303 POST rewriting is deliberately disabled; and
- cross-origin redirects are returned and never followed.

Callers must treat `ambiguous_outcome()` as delivery-history information. A
successful later response does not prove the earlier attempt was unseen.

## Streaming request and response bodies

Use `.httpbody.fixed(content_length, capacity, response_capacity)` when the
producer knows the exact length. Use
`.httpbody.chunked(capacity, response_capacity)` otherwise. Request capacity is
`1..16777216`; response capacity is `256..16777216`.

The producer must run concurrently with `post_stream()` or a small endpoint
can correctly backpressure both sides. This fixed-length example is adapted
from the executable streaming fixture:

```rexx
produce: task = .int
  arg body = .httpbody

  if body.write("hello " as .binary, 5000) <> 6 then return 1
  if body.write("world" as .binary, 5000) <> 5 then return 2
  call body.finish()
  return 0

body = .httpbody.fixed(11, 8, 512)
scope = .taskscope.failfast(request_pool, 60000)

do parallel using scope
  produced = produce(body)
  response = client.post_stream("/upload", body, headers, 512)
end

if produced <> 0 then say "producer failed" produced
```

The `response_capacity` argument to `post_stream()` must exactly equal
`body.response_capacity()`. `write(bytes, milliseconds)` returns accepted bytes
or `-1` after cancellation, deadline or failure. `finish()` half-closes the
request write side and publishes EOF. `cancel(reason)` cancels both paired
endpoints; `close()` releases both adapters.

A successful streaming response contains metadata plus a byte-endpoint
reference. Drain it explicitly:

```rexx
drain_response: procedure = .binary
  arg response = .httpresponse

  endpoint = .byteendpoint.from_encoded_reference(response.body_reference())
  result = ''x as .binary

  do forever
    request = endpoint.start_read(4096, 5000)
    outcome = request.wait(5000)
    if outcome.available() = 0 | outcome.succeeded() = 0 then do
      call endpoint.close()
      return ''x as .binary
    end

    chunk = outcome.value().as_binary()
    if binlength(chunk) > 0 then result = binconcat(result, chunk)
    if outcome.details().field("eof").as_boolean() then leave
  end

  call endpoint.close()
  return result
```

Call `streaming()` before choosing `body()` or `body_reference()`; using the
wrong accessor signals an error. Streaming responses currently require
identity content encoding. Buffered responses provide the bounded compression
support described above. The complete fixed/chunked request and streamed
response proof is
[`ts_http_streaming.crexx`](../../../lib/rxfnsg/tests_functional/ts_http_streaming.crexx).

## TLS and verification

`https://` origins use `socketconnecttls(sock, host, port)`. Supported builds
verify the certificate chain and hostname using the configured platform
backend: Apple system TLS, OpenSSL on supported Unix-like systems, or SChannel
on Windows. A build without TLS support fails the request; it does not
downgrade to plaintext.

The live TLS fixture is environment-gated because it depends on external
network and trust state. The ordinary loopback suite remains deterministic and
covers pool ownership, policy, redirects, retries, ambiguity, streaming,
compression and the cREXX-RAG request shapes.

## A Rexx task class as an HTTP service

An HTTP handler is a normal user class implementing `.httpservice` and
`.taskwork`. Write the typed `handle(request, context)` method for application
logic. The explicit `run` method is the bridge to the standard `.taskwork` ABI;
`.httpservice.dispatch` performs the checked request/response conversion.

```rexx
options levelg
import rxfnsb
import concurrency
import rxfnsg

health_service: class implements .httpservice .taskwork
  *: factory
    return

  handle: method = .httpresponse
    arg request = .httprequest, context = .taskcontext
    if request.verb() = "GET" & request.target() = "/health" then do
      return .httpresponse.text(200, "ok")
    end
    return .httpresponse.text(404, "not found")

  run: method = .channelvalue
    arg encoded = .channelvalue, context = .taskcontext
    service = self as .httpservice
    return service.dispatch(encoded, context)

main: procedure = .int
  server = .httpserver.local("127.0.0.1", 8080)
  served = server.serve(task .health_service(), 1)
  call server.close()
  return 0

exit main()
```

This complete example deliberately serves one request so that normal cleanup
is visible. Omitting the second `serve` argument, or passing `-1`, keeps the
controller loop serving until its host is stopped. Detached/background server
lifecycle is not part of the current surface.

The `task .health_service()` expression is a compiler/linker-sealed factory
target. Each accepted request constructs task work according to that target;
the pool is a bounded executor, not a user-object factory. Factory arguments
may be added in the usual class syntax, subject to normal transferable task
argument rules.

## Server request and response values

`.httprequest` gives a handler these read-only views:

| Method | Result |
| --- | --- |
| `verb()` | validated uppercase request method |
| `target()` | origin-form target, including any query |
| `header(name)` | case-insensitive header lookup |
| `headers()` | raw validated header field block |
| `body()` / `body_bytes()` | buffered body as text / exact bytes |
| `remote()` | numeric peer endpoint text |

`inbound()`, `routed()`, `from_channel()`, `to_channel()`, `routed_reply()` and
`reply_reference()` appear in the generated pre-release API because they are
the exact controller/task transfer bridge. Normal handlers receive a request
from `dispatch`; they do not construct routed requests or use the private reply
capability themselves.

Return `.httpresponse.text(status, text)` for text or
`.httpresponse.binary(status, bytes)` for exact bytes. Response headers may be
supplied with the optional `.httpheaders` argument. The controller validates
the returned status, headers and body against server limits before any socket
write. A raised handler or an unusable response becomes a bounded HTTP 500; a
handler deadline becomes 504.

## Server factory, limits and lifecycle

```rexx
server = .httpserver.local(host, port, workers, admission, connections, maximum_header_bytes, maximum_headers, maximum_body_bytes, handler_timeout, request_timeout)
```

Defaults are `4` workers, `64` admitted tasks, `64` accepted connections,
65536 header bytes, `100` headers, 8388608 body bytes and 30000 ms for both
handler and request-read deadlines. Port `0` asks the operating system for an
ephemeral port; `port()` returns the selected value.

`serve(target, maximum_requests)` owns the controller loop and returns the
number completed by that call. Only complete requests cross to workers. The
controller retains the listener and every accepted socket, permits one
in-flight request per connection and uses bounded nonblocking scans with a
short idle wait. `served()`, `scans()` and `idle_scans()` are diagnostic
counters, not throughput guarantees. Call `close()` after a bounded `serve`
returns; it closes sockets and joins the private handler scope and pool.

The initial server accepts buffered clear-text HTTP/1.1 requests, with
HTTP/1.0 compatibility, canonical `Content-Length`, keep-alive and pipelining
boundaries. It rejects request `Transfer-Encoding` with 501. Server TLS,
HTTP/2, WebSockets, detached/background lifecycle, request streaming and
streaming handler responses are outside the present contract.

The executable service, parallel-client, binary-body, malformed-message,
deadline and failure examples are
[`ts_http_server.crexx`](../../../lib/rxfnsg/tests_functional/ts_http_server.crexx)
and
[`ts_http_server_failures.crexx`](../../../lib/rxfnsg/tests_functional/ts_http_server_failures.crexx).
