# cREXX LLM and HTTP architecture

Status date: 2026-08-16

This note records the current transport boundary for Level G LLM providers.
It supersedes the May 2026 baseline that used the former public Level B
`rxhttp` convenience client.

## Current layering

- Level B `rxjson` supplies the focused string-oriented JSON helpers.
- Level B `rxsocket` exposes VM-managed TCP sockets and verified client TLS.
- Private Level B `_rxhttpcore` supplies binary HTTP framing, parsing and
  bounded gzip/zlib/raw-DEFLATE codecs. It is not a user client.
- Public Level G `.httpclient` owns HTTP policy, bounded admission, reusable
  single-owner connections and typed responses.
- Public Level G `.httpserver` owns accepted sockets and dispatches complete
  request values to `.httpservice .taskwork` targets.
- Level G `.ollama`, `.openai`, `.anthropic` and `.gemini` all use the public
  `.httpclient` and the same private core.

There is deliberately no second public Level B HTTP client. HTTP remains a
Rexx library over sockets, tasks, channels and endpoints; it is not an RXAS
instruction or a channel-provider type.

## LLM provider behavior

`.llm(...)` selects the local Ollama implementation. Ollama posts JSON to
`/api/generate` with `stream:false`. Hosted providers construct their own JSON
and authentication headers, use verified HTTPS and obtain keys from the
provider-specific environment variables documented in
`lib/rxfnsg/rexx/llm.md`.

Every provider owns a bounded `.httpclient` connection pool and exposes
`close()`. Diagnostic methods retain the decoded provider JSON, HTTP status and
a reconstructed HTTP response view; they do not expose a live socket or a
shared mutable HTTP object.

## Deterministic QA

Ordinary automated validation is synthetic and does not require external
services or secrets:

- `ts_llm_ollama`: Ollama request/body extraction and provider JSON behavior;
- `ts_llm_providers`: OpenAI, Anthropic and Gemini request/header/body behavior;
- `ts_http_pooled`: connection ownership, reuse and admission;
- `ts_http_policy`: headers, redirects, retries, idempotency and ambiguity;
- `ts_http_streaming`: bounded fixed/chunked requests and response streams;
- `ts_http_codec`: bounded response decoding and malformed inputs;
- `ts_http_crexx_rag`: concurrent generation/embedding shapes required by
  `crexx-rag`;
- `ts_http_server`: task-service dispatch, parallel clients, binary bodies,
  malformed requests, pipelining boundaries and request deadlines;
- `ts_http_server_failures`: raised handlers and unusable response limits; and
- `address_llm_provider`: a normally linked ADDRESS-provider/host image whose
  transient LLM providers are closed after each helper call.

These fixtures run across `rxbvm` and `rxtvm`, optimized and unoptimized where
the matrix applies. Live TLS and provider demos are opt-in checks because they
depend on network, trust-store, local-model or secret state.

## Platform and protocol boundaries

Client TLS uses the configured VM socket backend: Network.framework on Apple
platforms, OpenSSL on supported non-Windows Unix-like platforms and SChannel on
Windows. A build without TLS support fails an HTTPS request rather than
downgrading it.

The client supports bounded buffered and streaming requests, buffered
compression decoding and explicit retry/redirect/ambiguous-outcome policy.
The initial server is clear-text and buffered. Server TLS, HTTP/2, WebSockets,
detached/background lifecycle and streaming handlers require later proposals.

Portable Linux/Windows and package qualification remain open in
`concurrency/WORKLIST.md`; Mac implementation evidence alone is not a release
or portability claim.

## Historical baseline

The superseded May 2026 arrangement proved VM sockets, TLS, JSON and the first
LLM providers with a synchronous connection-close client. That evidence remains
useful provenance, but its two-client architecture is not the current source
contract. Use `docs/ai-context/CREXX_CONCURRENCY.md`, the
[HTTP client/server guide](../books/crexx_library_reference/concurrent_http.md)
and current source/tests for implementation decisions.
