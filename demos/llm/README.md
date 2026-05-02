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
