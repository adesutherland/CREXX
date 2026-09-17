# cREXX Level G LLM library

Import `rxfnsg` and use `.llm.open(.llmconfig(driver,model))` to select Ollama,
OpenAI, Anthropic, Gemini or optional native llama. Processing code uses the same
`.llm` contract for `generate`, status/error, owned results and capability checks.
The [common-driver guide](../../plugins/llama/common.md) is the API walkthrough,
including request lifecycle, separate embeddings and application-level exceptions
when the optional plugin is unavailable. Runnable examples live beside that guide.

`llm.crexx` is `options levelg`, lives in namespace `rxfnsg`, and builds into
`rxfnsg.rxbin`. HTTP drivers use `.httpclient`, the shared `_rxhttpcore` framing
and TLS transport, and `rxjson`. The native C RXPA adapter uses the existing
llama owners and has no HTTP dependency of its own. Core imports do not require
the optional inference engine.

## Existing callers

`.llm(model="gemma4:latest",host="127.0.0.1",port=11434,timeout=120000)`
always selects Ollama. Direct `.ollama`, `.openai`, `.anthropic` and `.gemini`
constructors remain available. Hosted constructors retain their existing three
arguments and append an optional origin URL for endpoint configuration.
Use `.llm.open(config)` when assigning any selected driver to an `.llm` variable.

HTTP diagnostics retain `generateJson`, `buildBody`, `buildRequest`, `extractBody`,
`extractText`, `lastJson` and `lastHttp`. Native calls to those methods return
unsupported-operation status `-4`; `generateJson` is not constrained generation.

The [ADDRESS demo](../../../demos/llm/README.md) uses the common client for
`GENERATE`, preserves existing aliases and HTTP helpers, and adds `LLM_NATIVE`
with explicit local model configuration. `CLOSE` releases its persistent client.

## Ollama Contract

The implementation posts to `/api/generate` with `stream:false`. Ollama serves
the local API at `http://localhost:11434/api`, and local access does not require
authentication. The response text is read from the `response` field in the JSON
body. HTTP framing, including byte-counted `Content-Length` and chunked
responses, is handled by the shared HTTP core before the decoded body is passed
to `rxjson`.

## Hosted Provider Contracts

OpenAI posts to `https://api.openai.com/v1/responses` with an
`Authorization: Bearer ...` header and a body containing `model` and `input`.
Text extraction supports both `output_text` and
`output.1.content.1.text`.

Anthropic posts to `https://api.anthropic.com/v1/messages` with `x-api-key` and
`anthropic-version: 2023-06-01` headers. The request body contains `model`,
`max_tokens`, and one user message. Text is read from `content.1.text`.

Gemini posts to
`https://generativelanguage.googleapis.com/v1beta/models/<model>:generateContent`
with an `x-goog-api-key` header. The request body uses the
`contents.1.parts.1.text` shape and text is read from
`candidates.1.content.parts.1.text`.

The hosted providers do not print or store API keys in diagnostics. Their
synthetic tests validate request construction and response parsing without
network access or secrets.
