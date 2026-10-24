# Concurrent HTTP source note

`http.crexx` implements the Level G `rxfnsg` HTTP/1.1 client over the common
task, channel and byte-endpoint surface. The enduring user and API guide is:

- [`docs/books/crexx_library_reference/concurrent_http.md`](../../../docs/books/crexx_library_reference/concurrent_http.md)

The source namespace exposes `.httpclient`, `.httpresponse`, `.httpheaders`,
`.httppolicy`, `.httpbody`, `.httpserver`, `.httprequest` and `.httpservice`.
Internal connection-owner and transfer-contract types are not user APIs.
`_rxhttpcore` is the one private, binary-oriented framing and parsing backend
shared by the Level G HTTP, server and LLM modules; it is not an alternative
public client.

The implementation currently includes:

- bounded per-origin admission and single-owner reusable connections;
- verified HTTP/TLS origins and safe transferable headers;
- explicit redirect, retry, idempotency and ambiguous-outcome policy;
- bounded buffered responses with gzip, zlib and raw DEFLATE decoding; and
- fixed/chunked request streams with bounded identity-encoded response streams.

Buffered callers may use `.get(path, headers)`, `.post(path, body, headers)`,
or the general `.request(verb, path, binaryBody, headers)` task methods. Text
callers use `response.body()`; binary callers use `response.body_bytes()`.
Every client owns bounded task and socket resources and must be closed.

Controlling-execution copies are small proxies for one private, stable
controller. Its scope is anchored before children capture scope/channel
references. Closing any controlling copy closes that controller; other copies
then reject requests, and an old copy cannot close a later recycled controller.
Worker `from_channel()` proxies still close only their local endpoint view.
Dropping a proxy does not run a new destructor or join: explicit `close()` is
required, with existing execution/channel teardown as the abandoned-owner
fallback. Creation scans retained peak controller slots; requests use constant
time slot/generation validation. No controller token enters the transfer record.
Rejected client operations raise the existing `CHANNEL_ERROR` condition with
the HTTP diagnostic; there is no separate HTTP signal code.

The initial server is clear-text, buffered and deliberately bounded. A
controller retains socket ownership and gives complete `.httprequest` values to
sealed `.httpservice .taskwork` handlers. See [`httpserver.md`](httpserver.md)
and the enduring guide for the handler bridge and supported protocol subset.

There is deliberately no public Level B HTTP client. Level B provides the
socket and binary primitives; Level G provides the user-facing HTTP policy,
pooling and typed response surface. The Level G LLM providers use this same
client and backend.

Executable behavior is covered by the `lib/rxfnsg/tests_functional/ts_http_*`
fixtures, including `ts_http_crexx_rag.crexx` and `ts_http_streaming.crexx`.
