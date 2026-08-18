# cREXX Level G LLM Library

`llm.rexx` is the Level G LLM integration layer. Level G is an overlay on the
Level B foundation here: this module is `options levelg`, lives in the
`rxfnsg` namespace, and builds as `rxfnsg.rxbin`. It uses the public Level G
`.httpclient` and the private `_rxhttpcore` framing/parser shared with it.

The first provider targets local Ollama. Hosted OpenAI, Anthropic/Claude, and
Gemini providers are also implemented in Rexx code and use the reusable Level G
HTTP owner pool for HTTP framing and TLS transport, plus `rxjson` for JSON.

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
- `openai`: concrete OpenAI Responses API implementation
- `anthropic`: concrete Anthropic Messages API implementation
- `gemini`: concrete Gemini `generateContent` implementation

The `.llm(...)` interface factory currently selects the local Ollama provider.
Its factory arguments are:

- `model = "gemma4:latest"`
- `host = "127.0.0.1"`
- `port = 11434`
- `timeout = 120000`

The hosted providers are constructed directly:

- `.openai(model = "gpt-4.1", apiKey = "", timeout = 120000)`
- `.anthropic(model = "claude-sonnet-4-5", apiKey = "", timeout = 120000)`
- `.gemini(model = "gemini-2.5-flash", apiKey = "", timeout = 120000)`

When `apiKey` is empty, the providers read environment variables:

- OpenAI: `OPENAI_API_KEY`
- Anthropic: `ANTHROPIC_API_KEY`
- Gemini: `GOOGLE_API_KEY`, then `GEMINI_API_KEY`

Primary methods:

- `generate(prompt) = .string`: returns the generated text
- `generateJson(prompt) = .string`: returns the provider JSON body
- `status() = .int`: `0` on success, HTTP status for HTTP failures, negative
  values for local socket/protocol errors
- `error() = .string`: last diagnostic message
- `lastJson() = .string`: last decoded provider JSON body
- `lastHttp() = .string`: reconstructed response head and decoded body for diagnostics
- `close()`: closes the provider's bounded HTTP connection-owner pool

Helper methods, public for tests and diagnostics:

- `buildBody(prompt) = .string`
- `buildRequest(prompt) = .string`
- `extractBody(response) = .string`
- `extractText(responseJson) = .string`

`demos/llm/llm_address_environment.crexx` wraps these helpers in a Rexx
ADDRESS provider. It lets scripts write `ADDRESS LLM_GPT_4_1`, `ADDRESS
CLAUDE_SONNET_4_5`, `ADDRESS GEMINI_2_5_FLASH`, or `ADDRESS GEMMA4_LATEST`
and use `BODY`, `REQUEST`, `EXTRACT`, and live `GENERATE` commands against the
selected driver. Driver selection uses `_rxsysb.addressdriverregistry`, so each
provider contributes exact aliases and prefixes while keeping the lookup policy
in the ADDRESS/Rexx layer.

## Example

```rexx
options levelg
import rxfnsg

client = .llm("gemma4:latest")
answer = client.generate("Reply with one short sentence.")
if client.status() <> 0 then do
  say client.error()
  call client.close()
  exit 1
end
say answer
call client.close()
```

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
