# llama.rexx

`import llama` exposes typed owners backed by the optional `rxllama` provider.
Import `rxfnsg` for packed results and `rxvector` for vector operations. The
inference runtime and packaged backends run inside the cREXX process. Provision
model files separately; query execution needs no model server or Python.

The typed contracts, factories and methods are declared and implemented in
[`typed.h`](typed.h) through C RXPA. There is no separate Rexx facade module to
maintain or copy. Application examples below use `import llama`; the procedural
`rxllama` entry points remain compatibility and low-level acceptance interfaces.

This implementation supports persistent BGE-small EN v1.5 embeddings. Generation
requests are STEP-05. Normal local CPU/Metal evidence does not qualify the pending
Windows/Linux/CUDA/Vulkan or STEP-06 sanitizer matrix. SAN-009 remains open under
that release-QA gate. The pinned upstream/runtime and model identities are in
`docs/planning/native-inference-step-01.md` and its `native-inference-step-01-lock.json`.

## Use

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

## Contracts

| Type | Operations |
| --- | --- |
| `configuration` | `set_int(key,value)`, `set_float(key,value)`, `set_text(key,value)` return status. Options are validated after each setter; reduce `request_tokens` before reducing `request_rows`. |
| `runtime` | `device_count()`, `device_info(index,key)` (one-based), `model(path,sha256,profile,config)`. |
| `model` | `state()`, `cancel()`, `embedding_session(config)`. |
| `embedding_session` | `prepare(work_tokens)`, `request(config)`. |
| `embedding_request` | `add(text,role)` returns a one-based row or zero on failure; `add_all(texts,role)`, `submit()`, `process(work_tokens)`, `state()`, `cancel()`, `result()`. |
| `embedding_result` | `values()`, `rows()`, `dimensions()`, `status()`, `diagnostic()`. No native close is needed. |
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
session except for reduced row/token/byte limits. The current defaults allow up
to eight rows, 512 tokens per row (including special tokens and the BGE query
prefix), 4096 aggregate tokens and 1 MiB of input. Invalid UTF-8, NUL in selectors/paths,
unsupported options and exceeded limits are rejected without truncation. A row
failure in `add_all` snapshots the original error and cancels the building request;
it cannot yield a silently shortened batch. Only completed batches publish results.

Positive `process` budgets cannot split a noncausal attention batch. One admitted
batch is the indivisible compute unit; callers schedule repeated batches themselves.
Applications choose their worker count and the `threads`/`batch_threads` overrides.
Keep owners inside their creating VM. Pass text, configuration values and output
bytes to workers, then construct local owners there. Compatible owners share weights;
contexts, admission, inputs, outputs and diagnostics remain private.

`hardware_mode` selects `auto`, `cpu` or `required-gpu`; `backend` and `devices`
provide explicit overrides. Inspect model `backend`, `device`, `placement`,
`selection`, `embedding_spec`, `load_count` and `gpu_layers`. Required unavailable
GPU selection fails clearly. The accepted STEP-04 Metal timing variation remains
NI-S4-P01 with unresolved cause; these examples do not reopen or explain it.

`typed.h` contains RexxDoc contracts and canonical RXPA declarations. Native array
member signatures use `[*]`, the compiler's unbounded-array descriptor spelling;
ordinary callers can declare `.string[]`. The existing low-level `rxllama` API
remains available and uses the same bridge, checked handles and payload hooks.
