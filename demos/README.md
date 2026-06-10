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
- `native/sqlite/`: a native C ADDRESS environment backed by SQLite, using a
  small driver table so later database providers can share the same shape.
- `llm/`: a Rexx-implemented LLM ADDRESS environment and live demo that route
  model-shaped ADDRESS names to the Level G LLM providers.
- `rexxscript/`: a small `REXXSCRIPT` compiler-exit playground showing exposed
  variables and captured script output.
