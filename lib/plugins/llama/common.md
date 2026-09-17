# One generation interface, five drivers

[Guide index](README.md) · [Model configuration](models.md) · [Native reference](reference.md)

Import `rxfnsg`, choose a driver with `.llmconfig`, and open `.llm.open(config)`.
The returned `.llm` has the same processing contract for `ollama`, `openai`,
`anthropic`, `gemini` and optional in-process `llama`. An application that can
switch drivers does not need `import llama` to compile or start.

```rexx
options levelg
import rxfnsg

config = .llmconfig("ollama", "gemma4:latest")
client = .llm.open(config)
answer = client.generate("Reply with one short sentence.")
if client.status() = 0 then say answer
else say client.error()
call client.close()
```

For hosted inference, change setup to `.llmconfig("openai", "gpt-4.1")`,
`.llmconfig("anthropic", "claude-sonnet-4-5")` or
`.llmconfig("gemini", "gemini-2.5-flash")`. Supply `api_key` with `set_text`,
or let the provider read `OPENAI_API_KEY`, `ANTHROPIC_API_KEY`, or
`GOOGLE_API_KEY` / `GEMINI_API_KEY`. These are library defaults, not a claim that
a particular account can access those model names. The provider preserves its
HTTP status and diagnostic when a server rejects the request.

For local inference, replace only setup:

```rexx
config = .llmconfig("llama", "/path/to/model.gguf")
call config.set_text("sha256", "the-model-file-sha256")
call config.set_text("hardware_mode", "cpu")
/* The GGUF must contain a template supported by the pinned engine.
   Otherwise choose an explicit supported chat_template; raw sends literal text. */
call config.set_int("output_tokens", 64)
```

The general native profile defaults to `generation`. It accepts compatible
GGUFs without a compiled-in model-name/hash list; the supplied hash must still
match the file. The two original profile names remain strict reference presets.
See [models](models.md) before selecting pooling, templates or larger models.

The runnable [common generation example](examples/common_generation.crexx)
puts the processing code in a procedure whose argument is `.llm`. It takes
`DRIVER MODEL PROMPT [SHA256] [CHAT_TEMPLATE] [PROVIDER_PATH]`; the same procedure
works for all five drivers. Hosted calls require network access and credentials.

## Optional provider errors

The loader first uses an already registered native factory (including a static
native import), then looks for exactly `rxllama.rxplugin` beside the executing
VM or native executable. Set `provider_path` to select a trusted explicit file.
It does not search arbitrary directories or download a provider or model.

```rexx
do
  client = .llm.open(config)
on signal notready as problem
  say problem.message()
  /* The application can choose another driver here. */
end
```

`NOTREADY` carries a message beginning `provider_unavailable: llama:` when the
plugin file is absent, or `provider_incompatible: llama:` when the file cannot
load, its common factory is missing, or its runtime package cannot initialize.
The handler runs inside the application; HTTP drivers remain usable afterward.
Unknown drivers and invalid setup raise `INVALID_ARGUMENTS`. A missing, altered
or incompatible model is a model failure reported by `prepare()` / `generate()`,
`status()` and `error()`, rather than a provider-unavailable exception. There is
no automatic switch to another driver or model.

The common path keeps llama optional for compiled native programs too. Building
a program which only imports `rxfnsg` does not bundle the engine. To enable its
local driver, provide a compatible plugin and `providers/` runtime beside the
executable, or set `provider_path` to a complete installation. An application
which explicitly imports `llama` retains the existing native bundling behavior.

## Results, capabilities and lifetime

`generate(prompt)` is synchronous. Native loading/preparation happens on its
first call and the client retains the model and private context for subsequent
calls. For startup scheduling, call `prepare(128)` until `ready`; handle terminal
`failed` and check `status()` / `error()`. Preparation can return `loading` or
`preparing`. HTTP preparation returns `ready` without contacting a server.

`lastResult()` is an owned `.llmresult` snapshot: `text()`, `driver()`,
`status()`, `error()`, `category()`, `finish()` and `tokens()`. It survives later
requests and client closure. Zero status and empty category mean success.
Categories distinguish `http` server errors, `provider` HTTP transport/parsing
errors, `native` inference errors and `closed`. Raw status codes remain available.
HTTP calls currently report token usage `-1` and finish `complete` because they
do not extract a provider finish reason; this does not imply EOS. Native finishes
are `eos`, `output_limit`, `cancelled` or `error`; empty means still running.

| `supports(name)` | HTTP drivers | Native llama generation |
| --- | --- | --- |
| `generation` | true | true |
| `http_diagnostics` | true | false |
| `batch`, `incremental`, `cancellation` | false | true |
| unknown capability / `embeddings` | false | false; use the separate embedding client |

Close clients explicitly. The native convenience client closes its issued
requests before session, model, runtime and configuration. Repeated close is
safe; retained results stay valid. Copying a client aliases its owners. For
workers, construct one client per VM; compatible native model weights are shared,
while contexts and mutable request data remain private. The lower-level typed
API exposes allocation counters and explicit ownership control.

## Requests and bounded processing

After native preparation, `client.request()` returns `.llmrequest`:

1. `add(system,prompt)` returns a one-based row; a nonpositive value is failure.
   `add_all(prompts,system)` returns zero on success. Check `status()` / `error()`.
2. `submit()` returns zero on success. While `state()` is `running`, call
   `process(positive_work_tokens)` and check its status.
3. `read(row)` returns an owned `.llmchunk` with `text()`, `tokens()`, `finish()`,
   `row()`, `status()` and `error()`. Text and token counts are deltas, not replayed
   on subsequent reads. Text contains complete UTF-8; native literal text supports
   U+0000. Chat-template input with U+0000 is rejected because the engine's
   template API accepts C strings.
4. `cancel()` requests native cancellation between work units. Capture remaining
   output, then `close()` the request. Cancellation is not GPU preemption.

HTTP requests support one user prompt with an empty system argument and one
blocking `process` call. A work-token argument does not turn the HTTP call into
streaming or limit its elapsed time. Batches and separate system prompts fail
explicitly. A failed request cannot submit a shortened batch; create a new
request to retry. HTTP cancellation can discard work before processing, but
cannot interrupt its blocking network call, so the capability remains false.

## Separate embeddings

Use `.embedding.open(config)`, currently supported by `llama` only. It has
`prepare(work_tokens)`, `embed(texts, role="document")`, `status()`, `error()`
and `close()`. Call preparation until ready before embedding. The general
`embedding` profile requires explicit `pooling` (`cls`, `mean` or `last`),
`normalization` (`l2` or `none`), `query_prefix` and `document_prefix`; empty
prefixes are allowed. These choices must match the model's intended use.

`embed` processes one admitted batch and returns an owned `.embeddingresult`:
`rows()`, `dimensions()`, `values()`, `specification()`, `status()` and `error()`.
Values use the existing row-major `.packedfloat` representation. The specification
includes artifact hash, engine identity and preprocessing; persist it with an
index and reject mismatches when querying. Changing pooling, normalization,
prefixes, context or model changes that identity. The fixed BGE preset retains
its historical specification for compatibility. See the runnable
[embedding example](examples/common_embeddings.crexx).

## Setup and compatibility

The common loader uses the bundled `rxfs` filesystem provider. Ordinary product
installations and native packaging supply it automatically. A custom static VM
that links the complete Level G library must register/link `rxfs` too; this is
a core dependency, independent of whether llama is installed.

Set configuration before opening; changing it later does not reconfigure an
existing client. `set_text` and `set_int` reject unknown keys. Native range and
compatibility checks occur when opening or preparing.

| Setup | Options |
| --- | --- |
| All | `driver`, `model` |
| Ollama | `host` (default `127.0.0.1`), `port` (11434), `timeout` (120000 ms) |
| Hosted HTTP | `api_key`, `endpoint` (origin URL), `timeout` |
| Native | `sha256`, optional `provider_path`, `profile`; hardware, preprocessing and integer limits from the [reference](reference.md#configuration) |

Explicit options belonging to another driver are rejected at opening. Connection settings do not tune
native compute; native token/memory settings do not configure hosted generation.
The legacy `.llm(model,host,port,timeout)` factory always selects Ollama, even
when the plugin is loaded. `.ollama`, `.openai`, `.anthropic` and `.gemini` direct
constructors and the native `import llama` API remain available.

Legacy `generateJson`, `buildBody`, `buildRequest`, `extractBody`, `extractText`,
`lastJson` and `lastHttp` remain on `.llm` for HTTP callers. The native driver
returns an explicit unsupported-operation status (`-4`) for them. They are
diagnostics, not the common processing contract. `generateJson` returns a server
JSON response body; it does not request constrained JSON generation.
