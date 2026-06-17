# cREXX LLM/HTTP Baseline Status

Status date: 2026-05-01

This note captures the first LLM integration baseline after bringing core
socket communication into the VM, adding a reusable HTTP layer above
`rxsocket`, and adding client TLS for hosted providers.

## Baseline Scope

- Level B `rxjson` provides string-oriented JSON validation, quoting, path
  lookup, object construction, and array construction.
- Level B `rxsocket` exposes the VM core TCP socket opcodes through a stable
  Rexx API.
- Level B `rxhttp` provides a reusable HTTP client over `rxsocket`.
  It handles UTF-8 byte `Content-Length`, HTTP/1.1 chunked transfer decoding,
  header lookup, non-2xx response bodies, and raw/body diagnostics.
- `rxhttp` supports additional caller-supplied request headers, allowing
  provider authentication headers without duplicating HTTP framing logic.
- Level G `rxfnsg` provides the first class-shaped LLM interface and concrete
  provider classes.
- `rxfnsg.llm` currently selects the local Ollama provider and posts to
  `/api/generate` with `stream:false`.
- `rxfnsg.openai`, `rxfnsg.anthropic`, and `rxfnsg.gemini` are Rexx
  implementations for hosted providers using `rxhttp` over VM TLS sockets.
- `demos/llm/ollama_generate.crexx` demonstrates calling a local Ollama model
  through `rxfnsg`, `rxhttp`, `rxjson`, and the core socket VM support.
- `demos/llm/openai_generate.crexx`, `anthropic_generate.crexx`, and
  `gemini_generate.crexx` read API keys from environment variables and call
  hosted providers over HTTPS.

## Verified Locally

The following commands passed locally on macOS:

```sh
cmake --build cmake-build-debug --target library rxfnsg testbifs testrxfnsg -- -j2
ctest --test-dir cmake-build-debug -R 'ts_rxhttp|ts_socket_tls_live|ts_llm_ollama|ts_llm_providers' --output-on-failure
cmake --build cmake-build-debug --target rxvme rxbvme crexx -- -j2
./cmake-build-debug/bin/crexx -lrxfnsg demos/llm/ollama_generate.crexx
```

The live Ollama check was run against `gemma4:latest` on
`http://localhost:11434`.

## CI Expectations

The hosted CI validation should rely on synthetic tests that do not require
Ollama or external network access:

- `ts_rxsocket`: socket loopback and status coverage
- `ts_socket_tls_live`: default-skipped live TLS handshake smoke; set
  `CREXX_TLS_LIVE_SMOKE=1` to enable it on a runner with network access
- `ts_rxjson`: JSON helper coverage
- `ts_rxhttp`: HTTP framing, byte lengths, chunked transfer, headers, error
  statuses, and truncated response coverage
- `ts_llm_ollama`: Ollama request/body extraction and provider JSON handling
  using fake HTTP responses
- `ts_llm_providers`: OpenAI, Anthropic, and Gemini request/header/body
  construction plus synthetic response/error parsing, without secrets or
  external network access

## Known Limits

- Client TLS exists below the VM-managed socket API as
  `socketconnecttls(sock, host, port)`. Fresh macOS builds default to
  `CREXX_ENABLE_TLS=NETWORK` to use Network.framework, Security.framework,
  CoreFoundation.framework, and the system trust store. Fresh Linux/Unix builds
  default to `CREXX_ENABLE_TLS=OPENSSL`; Windows builds default to
  `CREXX_ENABLE_TLS=SCHANNEL` to use SChannel/SSPI and the Windows trust store.
- `rxhttp` intentionally sends `Accept-Encoding: identity`; compressed response
  bodies are not supported yet.
- Redirects, cookies, proxies, persistent connections, and chunk trailers are
  not implemented in this baseline.
- `rxjson` is still a focused helper library rather than a full JSON object
  model. Escaped non-ASCII `\uXXXX` values are accepted, but full Unicode
  unescape fidelity remains future work.
- Live LLM demos are manual smoke tests, not CI dependencies. CI should use
  synthetic provider tests unless a secret-bearing workflow explicitly opts in.

## Next Work

- Validate hosted provider demos manually on Windows, Linux, and macOS with
  real keys.
- Add a secret-gated GitHub smoke workflow only after deciding which provider
  should be the canonical CI canary.
- Add a richer LLM demo/library surface after the CI socket/HTTP/TLS baseline is
  stable.
- Expand `rxjson` only where real provider responses expose gaps.
