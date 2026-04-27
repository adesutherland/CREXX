# cREXX Demos

This directory is for user-facing demonstrations that are useful to run by
hand, but that should not be treated as normal hosted CI tests. It is a better
home for connectivity examples than `tests/demo`, which can gradually become
pure regression coverage over time.

## Current Demos

- `llm/ollama_generate.rexx`: calls a local Ollama model through the Level G
  `rxfnsg` LLM interface and the VM core socket instructions.
