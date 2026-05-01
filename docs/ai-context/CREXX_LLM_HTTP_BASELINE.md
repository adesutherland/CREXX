# cREXX LLM/HTTP Baseline Status

Status date: 2026-05-01

This note captures the first plain-HTTP LLM integration baseline after bringing
core socket communication into the VM and adding a reusable HTTP layer above
`rxsocket`.

## Baseline Scope

- Level B `rxjson` provides string-oriented JSON validation, quoting, path
  lookup, object construction, and array construction.
- Level B `rxsocket` exposes the VM core TCP socket opcodes through a stable
  Rexx API.
- Level B `rxhttp` provides a reusable plain-HTTP client over `rxsocket`.
  It handles UTF-8 byte `Content-Length`, HTTP/1.1 chunked transfer decoding,
  header lookup, non-2xx response bodies, and raw/body diagnostics.
- Level G `rxfnsg` provides the first class-shaped LLM interface.
- `rxfnsg.llm` currently selects the local Ollama provider and posts to
  `/api/generate` with `stream:false`.
- `demos/llm/ollama_generate.rexx` demonstrates calling a local Ollama model
  through `rxfnsg`, `rxhttp`, `rxjson`, and the core socket VM support.

## Verified Locally

The following commands passed locally on macOS:

```sh
cmake --build cmake-build-debug --target library rxfnsg testbifs testrxfnsg -- -j2
ctest --test-dir cmake-build-debug -R 'ts_rxhttp|ts_llm_ollama|ts_rxjson|ts_rxsocket' --output-on-failure
cmake --build cmake-build-debug --target rxvme rxbvme crexx -- -j2
./cmake-build-debug/bin/crexx -lrxfnsg demos/llm/ollama_generate.rexx
```

The live Ollama check was run against `gemma4:latest` on
`http://localhost:11434`.

## CI Expectations

The hosted CI validation should rely on synthetic tests that do not require
Ollama or external network access:

- `ts_rxsocket`: socket loopback and status coverage
- `ts_rxjson`: JSON helper coverage
- `ts_rxhttp`: HTTP framing, byte lengths, chunked transfer, headers, error
  statuses, and truncated response coverage
- `ts_llm_ollama`: Ollama request/body extraction and provider JSON handling
  using fake HTTP responses

## Known Limits

- The original baseline was plain-text only. Client TLS now exists below the
  VM-managed socket API as `socketconnecttls(sock, host, port)`. Fresh macOS
  builds default to `CREXX_ENABLE_TLS=NETWORK` to use Network.framework,
  Security.framework, CoreFoundation.framework, and the system trust store.
  Fresh Linux/Unix builds default to `CREXX_ENABLE_TLS=OPENSSL`; Windows remains
  `OFF` until a native backend is added. Provider work still needs to opt into
  HTTPS.
- `rxhttp` intentionally sends `Accept-Encoding: identity`; compressed response
  bodies are not supported yet.
- Redirects, cookies, proxies, persistent connections, and chunk trailers are
  not implemented in this baseline.
- `rxjson` is still a focused helper library rather than a full JSON object
  model. Escaped non-ASCII `\uXXXX` values are accepted, but full Unicode
  unescape fidelity remains future work.
- The live Ollama demo is a manual smoke test, not a CI dependency.

## Next Work

- Validate this baseline on GitHub runners for Windows, Linux, and macOS.
- Add provider-specific HTTPS clients without changing the Level G LLM API.
- Add a richer Ollama demo/library surface after the CI socket/HTTP baseline is
  stable.
- Expand `rxjson` only where real provider responses expose gaps.
