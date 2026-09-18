# LLM Demos

These demos use the Rexx `rxfnsg` LLM provider layer. Hosted providers read
keys from the environment:

- OpenAI: `OPENAI_API_KEY`
- Anthropic: `ANTHROPIC_API_KEY`
- Gemini: `GOOGLE_API_KEY` or `GEMINI_API_KEY`
- Ollama: local server, with `OLLAMA_MODEL` set to the model to demo

## ADDRESS Demo

Build and run the ADDRESS demo. It checks the environment variables above and
runs each provider that is configured:

```sh
cmake --build cmake-build-debug --target llm_address_demo_bin && \
./cmake-build-debug/bin/rxvm \
  ./cmake-build-debug/bin/library.rxbin \
  ./cmake-build-debug/bin/rxfnsg.rxbin \
  ./cmake-build-debug/demos/llm/llm_address_environment.rxbin \
  ./cmake-build-debug/demos/llm/llm_address_demo.rxbin \
  -a "Say hello from cREXX"
```

The demo uses `ADDRESS ... GENERATE :prompt INTO ${answer}` internally.

## Native ADDRESS driver

`ADDRESS LLM_NATIVE` selects local in-process llama. Set `CREXX_LLAMA_MODEL` to
a GGUF path. Optionally set `CREXX_LLAMA_SHA256` to verify an expected hash;
otherwise the bridge calculates the file's identity. Other optional settings are
`CREXX_LLAMA_PROFILE`, `CREXX_LLAMA_CHAT_TEMPLATE`, `CREXX_LLAMA_MEMORY_BYTES`
and `CREXX_LLAMA_PROVIDER_PATH`. The model path is never guessed from the
environment name. Use a supported GGUF template or an explicit template/raw
override, as described in the [common guide](../../lib/plugins/llama/common.md).

`GENERATE` retains one common `.llm` client across calls; changing the model
replaces it. The `CLOSE` command/function releases it. Missing or incompatible
plugins raise application-catchable `NOTREADY` on selection. Existing environment
aliases retain their meanings, including `LLAMA...` aliases for Ollama models.
`BODY`, `REQUEST` and `EXTRACT` on the native driver report unsupported operation;
there is no fabricated HTTP exchange. Embeddings use the separate typed API.

## Function Demos

OpenAI:

```sh
cmake --build cmake-build-debug --target llm_openai_generate_demo_bin && \
./cmake-build-debug/bin/rxvm \
  ./cmake-build-debug/bin/library.rxbin \
  ./cmake-build-debug/bin/rxfnsg.rxbin \
  ./cmake-build-debug/demos/llm/openai_generate.rxbin \
  -a "Say hello from cREXX" "${OPENAI_MODEL:-gpt-4.1}"
```

Anthropic:

```sh
cmake --build cmake-build-debug --target llm_anthropic_generate_demo_bin && \
./cmake-build-debug/bin/rxvm \
  ./cmake-build-debug/bin/library.rxbin \
  ./cmake-build-debug/bin/rxfnsg.rxbin \
  ./cmake-build-debug/demos/llm/anthropic_generate.rxbin \
  -a "Say hello from cREXX" "${ANTHROPIC_MODEL:-claude-sonnet-4-5}"
```

Gemini:

```sh
cmake --build cmake-build-debug --target llm_gemini_generate_demo_bin && \
./cmake-build-debug/bin/rxvm \
  ./cmake-build-debug/bin/library.rxbin \
  ./cmake-build-debug/bin/rxfnsg.rxbin \
  ./cmake-build-debug/demos/llm/gemini_generate.rxbin \
  -a "Say hello from cREXX" "${GEMINI_MODEL:-gemini-2.5-flash}"
```

Ollama:

```sh
cmake --build cmake-build-debug --target llm_ollama_generate_demo_bin && \
./cmake-build-debug/bin/rxvm \
  ./cmake-build-debug/bin/library.rxbin \
  ./cmake-build-debug/bin/rxfnsg.rxbin \
  ./cmake-build-debug/demos/llm/ollama_generate.rxbin \
  -a "Say hello from cREXX" "${OLLAMA_MODEL:-gemma4:latest}"
```
