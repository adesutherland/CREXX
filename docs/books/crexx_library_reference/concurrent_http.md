# Concurrent HTTP client {#library-concurrent-http}

The `rxfnsg` namespace supplies a bounded HTTP/1.1 client designed for Level G
task calls. It combines structured task scopes, single-owner reusable
connections, safe transferable headers and endpoint-backed streaming. HTTP is
a Rexx library built on concurrency; it is not a special RXAS instruction or
channel provider type.

The client is experimental on `develop` and currently macOS-qualified. Linux
and Windows publication still requires the portable concurrency conformance
matrix.

## Choose the intended HTTP client

The repository currently contains two independent clients:

| Client | Import | Execution model | Current use |
| --- | --- | --- | --- |
| synchronous Level B client | `import rxhttp` | one request in the calling execution | used by the existing Level G LLM provider classes |
| concurrent Level G client | `import rxfnsg` | task calls over bounded connection owners | generation/embedding, policy, streaming and codec fixtures |

They do not wrap a common framing core. Existing `rxhttp` programs do not need
to migrate merely because this client exists. Future consolidation is an
explicit architecture decision, not an undocumented compatibility promise.

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
