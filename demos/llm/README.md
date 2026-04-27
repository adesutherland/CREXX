# LLM Demos

These demos exercise local LLM integrations over plain HTTP. They assume a
local service is already running.

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
