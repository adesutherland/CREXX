# cREXX Level G LLM Library

`llm.rexx` is the first Level G LLM integration layer. Level G is an overlay on
the Level B foundation here: this module is `options levelg`, lives in the
`rxfnsg` namespace, and builds as `rxfnsg.rxbin`, while deliberately importing
and depending on the Level B `rxfnsb`, `rxjson`, and `rxhttp` libraries.

The first provider targets local Ollama. It uses the reusable Level B `rxhttp`
client for HTTP framing and JSON helpers from `rxjson`. It is plain HTTP only;
TLS is intentionally deferred.

## Import

```rexx
options levelg
import rxfnsg
```

At runtime, load both the Level B foundation `library.rxbin` and the Level G
overlay `rxfnsg.rxbin`.

## API

The namespace exposes:

- `llm`: provider-selecting interface
- `ollama`: concrete local Ollama implementation

Factory arguments:

- `model = "gemma4:latest"`
- `host = "127.0.0.1"`
- `port = 11434`
- `timeout = 120000`

Primary methods:

- `generate(prompt) = .string`: returns the generated text
- `generateJson(prompt) = .string`: returns the provider JSON body
- `status() = .int`: `0` on success, HTTP status for HTTP failures, negative
  values for local socket/protocol errors
- `error() = .string`: last diagnostic message
- `lastJson() = .string`: last decoded provider JSON body
- `lastHttp() = .string`: last raw HTTP response

Helper methods, public for tests and diagnostics:

- `buildBody(prompt) = .string`
- `buildRequest(prompt) = .string`
- `extractBody(response) = .string`
- `extractText(responseJson) = .string`

## Example

```rexx
options levelg
import rxfnsg

client = .llm("gemma4:latest")
answer = client.generate("Reply with one short sentence.")
if client.status() <> 0 then do
  say client.error()
  exit 1
end
say answer
```

## Ollama Contract

The implementation posts to `/api/generate` with `stream:false`. Ollama serves
the local API at `http://localhost:11434/api`, and local access does not require
authentication. The response text is read from the `response` field in the JSON
body. HTTP framing, including byte-counted `Content-Length` and chunked
responses, is handled by `rxhttp` before the decoded body is passed to
`rxjson`.
