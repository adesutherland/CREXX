# Concurrent HTTP source note

`http.crexx` implements the experimental Level G `rxfnsg` HTTP/1.1 client over
the common task, channel and byte-endpoint surface. The enduring user and API
guide is:

- [`docs/books/crexx_library_reference/concurrent_http.md`](../../../docs/books/crexx_library_reference/concurrent_http.md)

The source namespace exposes `.httpclient`, `.httpresponse`, `.httpheaders`,
`.httppolicy` and `.httpbody`. Internal connection-owner and transfer-contract
types are not user APIs.

The implementation currently includes:

- bounded per-origin admission and single-owner reusable connections;
- verified HTTP/TLS origins and safe transferable headers;
- explicit redirect, retry, idempotency and ambiguous-outcome policy;
- bounded buffered responses with gzip, zlib and raw DEFLATE decoding; and
- fixed/chunked request streams with bounded identity-encoded response streams.

The repository also retains the independent synchronous Level B client in
`lib/rxfnsb/rexx/rxhttp.crexx`. Current LLM provider classes use that client.
Any shared-core extraction or migration is tracked as a separate concurrency
architecture decision; the two implementations must not be described as one
stack today.

Executable behavior is covered by the `lib/rxfnsg/tests_functional/ts_http_*`
fixtures, including `ts_http_crexx_rag.crexx` and `ts_http_streaming.crexx`.
