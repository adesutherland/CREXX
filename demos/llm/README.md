# LLM Demos

These demos exercise LLM integrations through the Rexx `rxfnsg` provider layer.
The local Ollama demo uses plain HTTP. The hosted provider demos use
`rxhttp` over the VM TLS socket support and read API keys from environment
variables.

## Ollama

`ollama_generate.rexx` calls `http://localhost:11434/api/generate` with
`stream:false`. It defaults to `gemma4:latest`, matching the local model used
for the first socket-based integration attempt. If Ollama returns an error body
or a body that the current JSON layer cannot parse, the demo prints the decoded
JSON body to make parser gaps visible.

Example:

```sh
./cmake-build-debug/bin/crexx -lrxfnsg demos/llm/ollama_generate.rexx
```

## Hosted Providers

The hosted demos are implemented in Rexx code and use the same `rxhttp`,
`rxjson`, and secure socket foundation:

- `openai_generate.rexx`: `OPENAI_API_KEY`, optional `OPENAI_MODEL`
- `anthropic_generate.rexx`: `ANTHROPIC_API_KEY`, optional `ANTHROPIC_MODEL`
- `gemini_generate.rexx`: `GOOGLE_API_KEY` or `GEMINI_API_KEY`, optional
  `GEMINI_MODEL`

Examples:

```sh
OPENAI_API_KEY=... ./cmake-build-debug/bin/crexx -lrxfnsg demos/llm/openai_generate.rexx
ANTHROPIC_API_KEY=... ./cmake-build-debug/bin/crexx -lrxfnsg demos/llm/anthropic_generate.rexx
GOOGLE_API_KEY=... ./cmake-build-debug/bin/crexx -lrxfnsg demos/llm/gemini_generate.rexx
```

The demos intentionally do not print API keys. On provider errors they print the
decoded JSON response body for diagnostics.

The current `crexx` driver treats additional command-line words as more source
files, so the checked-in script has safe defaults for the prompt and model. The
script still reads `arg` values when run through `rxvm -a`, which is useful for
manual experiments after compiling it.

```sh
./cmake-build-debug/bin/rxvm \
  ./cmake-build-debug/bin/library.rxbin \
  ./cmake-build-debug/bin/rxfnsg.rxbin \
  demos/llm/ollama_generate.rxbin \
  -a "Say hello from cREXX" gemma4:latest
```
