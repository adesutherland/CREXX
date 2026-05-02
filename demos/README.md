# cREXX Demos

This directory is for user-facing demonstrations that are useful to run by
hand, but that should not be treated as normal hosted CI tests. It is a better
home for connectivity examples than `tests/demo`, which can gradually become
pure regression coverage over time.

## Current Demos

- `cms/`: a Rexx-implemented CMS ADDRESS environment that demonstrates
  ADDRESS instructions, environment instance objects, and ADDRESS functions.
- `native/kv/`: a native C ADDRESS environment with a small key/value store,
  matching the same command/function protocol as the Rexx CMS demo.
- `llm/ollama_generate.rexx`: calls a local Ollama model through the Level G
  `rxfnsg` LLM interface and the VM core socket instructions.
