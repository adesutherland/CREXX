# llama.rexx

`import llama` exposes typed owners backed by the optional `rxllama` provider.
Import `rxfnsg` for packed results and `rxvector` for vector operations. The
inference runtime and packaged backends run inside the cREXX process. Provision
model files separately; query execution needs no model server or Python.

The typed contracts, factories and methods are declared and implemented in
the repository's `lib/plugins/llama/typed.h` through C RXPA. There is no separate Rexx facade module to
maintain or copy. Application examples below use `import llama`; the procedural
`rxllama` entry points remain compatibility and low-level acceptance interfaces.

For ordinary generation, use `import rxfnsg` and `.llm.open(config)` with driver
`llama`. Ollama and the three hosted providers use the same `.llm` contract.
The plugin is optional: selecting it when absent raises application-catchable
`NOTREADY`. [The common-driver guide](common.md) covers setup, errors, requests,
owned results and the separate `.embedding.open(config)` capability.

General `generation` and `embedding` profiles accept compatible local GGUFs.
The common client needs only the filename for model selection; an optional
caller-supplied SHA-256 verifies an expected artifact, otherwise the bridge
calculates its identity. Embeddings still require explicit preprocessing.
Dimensions and admission
reservations come from validated model properties. The original BGE-small F16
and SmolLM2 Q8_0 profiles remain exact, reproducible presets. They are no longer
an allowlist for all model use. Compatibility still depends on the pinned
engine, supported dense model layout, tokenizer and template; see [models](models.md).

Local trained-model CPU/Metal checks and generated-fixture package checks have
passed on the configurations recorded in [qualification status](qualification.md).
SAN-009 is closed. Remaining real-device/model and provenance acceptance stays
open; package smoke tests do not establish trained-model quality or full release
readiness. Use that status page for the evidence and its limits.

## Start here

1. [Install the optional provider](installation.md), including GPU build choices
   and Windows instructions.
2. [Download and verify the model you need](models.md); keep weights as separate
   data and reuse them offline.
3. [Build and run the examples](examples/README.md) from the installed package,
   first the common client, then explicit persistent batches and shared workers.
4. Use the [operating reference](reference.md) for every configuration option,
   device/diagnostic key, memory/ownership rule and error-handling contract.
5. Check [capability and qualification status](qualification.md) before relying
   on a platform claim.

These pages are installed together under `share/crexx/llama`. The repository's
`docs/planning/native-inference-backlog.md` retains the full plan and acceptance
criteria; `docs/qa/native-inference-step07/README.md` maps them to documentation,
tests, evidence and remaining QA. Historical STEP-01 Rexx-facade proposals are
superseded by the implemented C factories documented here.

## Advanced typed embeddings

1. Construct `.llama..configuration()`, check `status()` and set options.
2. Construct `.llama..runtime(config)` and check its status.
3. Call `runtime.model(path, sha256, "bge-small-en-v1.5", config)`. Check status,
   then poll `model.state()` until `ready`; `failed`/`cancelled` are terminal.
4. Create `model.embedding_session(config)`. Call `prepare(work_tokens)` until
   `ready`, checking status each time. Preparation and loading belong at startup.
5. Create `session.request(config)`. Use `add(text, role)` for one row or
   `add_all(texts, role)` for an ordered array. Roles are `query`/`document`.
   `add_all` does not submit. Submit once and call `process(work_tokens)`.
6. On `complete`, call `request.result()` and check its status. Close the request;
   the result remains usable. `result.values()` returns an independent owned
   `.packedfloat` in row-major order. Reuse the prepared session for further batches.
7. Close requests, sessions, models, runtime and configuration in that order.

`examples/persistent_embeddings.crexx` performs twenty batches with one model
load and existing rxvector storage conversion. `examples/shared_embeddings.crexx`
uses four actual workers, one compatible model allocation and private sessions,
comparing each worker's twenty batches with its isolated reference. Both examples
are installed under `share/crexx/llama/examples`. Run with hardware mode, local
GGUF path and its SHA256. They are functional demonstrations, not speed benchmarks.

For example, use the installed `crexx` build command:

```sh
crexx --program embeddings persistent_embeddings.crexx --native
./embeddings auto /path/to/bge-small-en-v1.5-f16.gguf f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999
```

## Advanced typed generation

Use the same configuration/runtime/load lifecycle with profile
`smollm2-360m-instruct`, then call `model.generation_session(config)`. Poll
`session.prepare(128)` until `ready`. For each request:

1. Create `session.request(config)` and check `status()`.
2. Use `request.add(system,prompt)` or `request.add_all(prompts,system)`.
   An empty system string selects the pinned SmolLM2 default instruction.
3. Submit once, then repeatedly call `request.process(128)` and check status.
   Each call performs at most one native prefill/decode unit, bounded by the
   caller's token budget and the configured physical batch. Ready rows are
   decoded together. Application work and cancellation can run between calls.
4. After each process call, use `request.read(row)` for each one-based input row.
   Check the returned chunk's status, then consume `text()`, `tokens()` and
   `finish()`. `tokens()` counts newly sampled non-EOG tokens since the prior read;
   `text()` contains only new complete UTF-8. A partial scalar is buffered, so
   a positive token count can accompany empty text. Re-reading does not replay
   earlier output. Each chunk owns its text and survives later reads and closure.
5. A row's `finish()` is empty while running, then `eos`, `output_limit`,
   `cancelled` or `error`. An immediate EOS can validly produce no text. A cancelled
   or failed batch preserves already completed rows' finish reasons. Request
   state is separate from the last native-call status; capture failure diagnostics
   before subsequent calls. Close the request, reuse the prepared session, then
   close children before parents at shutdown.

The defaults reserve 32 output tokens within each 512-token sequence; the rendered
system/user template and special tokens count against that sequence budget.
All rows are validated before any compute. The 1 MiB request byte option bounds
rendered input and accumulated generated output separately. Request-specific
output limits can decrease. Independent requests clear private KV state; model
weights and prepared contexts persist. No one-shot server or hidden worker pool
is started. Greedy sampling is the initial supported policy.

`examples/persistent_generation.crexx` processes twenty four-row batches and
shows how to collect incremental output. `examples/shared_generation.crexx`
opens four real cREXX workers, each processing twenty four-row batches with
private contexts and distinct ordered inputs. It compares each worker with its
isolated reference and checks the shared allocation identity. Like the embedding
examples, both are installed under `share/crexx/llama/examples` and take hardware
mode, local GGUF and SHA256. For example:

```sh
crexx --program generation persistent_generation.crexx --native
./generation auto /path/to/smollm2-360m-instruct-q8_0.gguf 48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201
```

The low-level equivalents are `rxllama..addprompt(request,system,prompt,row)`
and `rxllama..readtext(request,row,text,tokens,finish)` with exposed output
arguments and an integer status. They use the same request state as the typed
API. Complete input/output includes U+0000; applications need no terminator,
float conversion or byte-to-text wrapper. Older hosts without the negotiated
counted output service reject generation explicitly. Existing embedding services
remain usable when their original host requirements are present.

## Contracts

| Type | Operations |
| --- | --- |
| `configuration` | `set_int(key,value)`, `set_float(key,value)`, `set_text(key,value)` return status. Options are validated after each setter; reduce `request_tokens` before reducing `request_rows`. |
| `runtime` | `device_count()`, `device_info(index,key)` (one-based), `model(path,sha256,profile,config)`. |
| `model` | `state()`, `cancel()`, `embedding_session(config)`, `generation_session(config)`. |
| `embedding_session` | `prepare(work_tokens)`, `request(config)`. |
| `embedding_request` | `add(text,role)` returns a one-based row or zero on failure; `add_all(texts,role)`, `submit()`, `process(work_tokens)`, `state()`, `cancel()`, `result()`. |
| `embedding_result` | `values()`, `rows()`, `dimensions()`, `status()`, `diagnostic()`. No native close is needed. |
| `generation_session` | `prepare(work_tokens)`, `request(config)`. |
| `generation_request` | `add(system,prompt)`, `add_all(prompts,system)`, `submit()`, `process(work_tokens)`, `state()`, `cancel()`, `read(row)`. |
| `generation_chunk` | `text()`, `tokens()`, `finish()`, `row()`, `status()`, `diagnostic()`. Owned snapshot; no close. |
| `diagnostic` | `code()`, `operation()`, `message()`. An independent value snapshot, with no close. |

Every native owner has `status()`, `diagnostic()` and `close()`. Runtime, model,
session and request also have `info_int(key)` and `info_text(key)` for the existing
bridge inspection keys. Status zero means the operation succeeded; nonzero is the
unchanged negative rxllama error code. State is separate from status. Integer/text
getters return zero/empty on failure, so inspect status when either is a valid value.
Inspecting status or taking a diagnostic snapshot preserves the recorded outcome.

Failed construction returns an initialized typed object with nonzero status and
no native resource. Closing it succeeds, as does repeated close. Copying an owner
retains the same checked native resource; closing one copy affects all aliases.
The last-operation diagnostic belongs to the particular value, and snapshots
remain unchanged by later operations. Child creation records the outcome in both
the parent and returned child. Result capture and `values()` make owned copies;
they do not introduce a new vector representation or convert through text.

Configuration is copied at construction. Request options must match the prepared
session except for reduced row/token/byte limits and generation output tokens. The current defaults allow up
to eight rows, 512 tokens per row (including special tokens and the BGE query
prefix or generation chat template), 4096 aggregate tokens and 1 MiB of input. Invalid UTF-8, NUL in selectors/paths,
unsupported options and exceeded limits are rejected without truncation. A row
failure in `add_all` snapshots the original error and cancels the building request;
it cannot yield a silently shortened batch. Embedding results require a complete
batch; generation publishes incremental text from submitted rows.

Positive embedding `process` budgets cannot split a noncausal attention batch. One admitted
batch is the indivisible compute unit; callers schedule repeated batches themselves.
Applications choose their worker count and the `threads`/`batch_threads` overrides.
For generation, a positive token budget bounds the work submitted by one call;
it is not a wall-clock deadline or GPU preemption guarantee. Other workers can
delay a call, so choose worker count and native thread counts together. The local
four-owner Release check recorded maximum calls of 2.64 seconds on CPU and
462 ms on Metal; the separately gated isolated calls stayed below 250 ms.
Keep owners inside their creating VM. Pass text, configuration values and output
bytes to workers, then construct local owners there. Compatible owners share weights;
contexts, admission, inputs, outputs and diagnostics remain private.

`hardware_mode` selects `auto`, `cpu` or `required-gpu`; `backend` and `devices`
provide explicit overrides. Inspect model `backend`, `device`, `placement`,
`selection`, `embedding_spec`/`generation_spec`, `load_count` and `gpu_layers`. Required unavailable
GPU selection fails clearly. The accepted STEP-04 Metal timing variation remains
NI-S4-P01 with unresolved cause; these examples do not reopen or explain it.

`typed.h` contains RexxDoc contracts and canonical RXPA declarations. Native array
member signatures use `[*]`, the compiler's unbounded-array descriptor spelling;
ordinary callers can declare `.string[]`. The existing low-level `rxllama` API
remains available and uses the same bridge, checked handles and payload hooks.
