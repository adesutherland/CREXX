# llama.rexx operating reference

[Guide index](README.md) · [Models](models.md) · [Examples](examples/README.md)

For the common client, start with [one interface, five drivers](common.md).
For direct ownership control, use `import llama`. The [guide index](README.md#contracts) lists the public
typed factories/methods. They are C RXPA bindings; applications do not need
Rexx factory wrappers, opaque-handle manipulation or float/text conversion
facades. The procedural `rxllama` interface uses the same engine and ownership
rules and remains available for low-level integration.

## Model selection

`runtime.model(path, sha256, profile, config)` accepts general `generation` and
`embedding` profiles, plus exact reference presets `bge-small-en-v1.5` and
`smollm2-360m-instruct`. Unknown profiles fail immediately. All paths verify the
actual file against a nonempty supplied SHA-256. An empty SHA-256 calculates
the file's identity synchronously during opening, before the shared-weight
lookup; tensor loading remains asynchronous. The common client permits omitting
the `sha256` option altogether. Automatically identified and explicitly pinned
owners use the same content identity. Only the reference presets also check
a built-in reference hash, even when the caller omits it. General model metadata is validated before tensor
allocation; see [model compatibility and preprocessing](models.md).

A valid constructor can return a loading owner which later enters `failed`;
inspect `model.info_text("error")`. A successful constructor alone does not
establish readiness. The common driver performs this lifecycle through
`prepare()` / `generate()` and retains the diagnostic.

There is no two-model limit. Compatible owners can share weights while retaining
private contexts, memory admission and session limits. General model identity
includes preprocessing and engine identity so incompatible configurations do not
share an assumed embedding space or generation setup.

## Status, state and lifetime

Construct configuration, runtime, model, prepared session and request in that
order. Check `status()` after each constructor and operation. Zero means the
call succeeded; it does not mean an asynchronous model is ready or a request
is complete. `model.state()` reports `loading`, `ready`, `failed` or `cancelled`.
Load failure details are available from `model.info_text("error")`. Poll the
state during startup, handling failures and an application deadline.

`session.prepare(positive_work_tokens)` may need multiple calls; stop when it
returns `ready`. Context allocation is one preparation unit, then warm-up
compute follows. Retain that session for repeated requests. For each request,
add rows while `building`, submit once, process while `running`, and inspect
the terminal `complete`, `failed` or `cancelled` state. Do not call processing
again after completion. Check both return state and last-operation status.

A failed factory returns a typed value with a nonzero status and no native
resource. `diagnostic()` captures `code()`, `operation()` and `message()` into
an independent value. Capture it before another operation overwrites the last
outcome. Reading `status()` or taking a diagnostic snapshot preserves that
outcome; ordinary getters are operations and can replace it. Child construction
records its outcome on both parent and child. Zero/empty getter results can be
valid, so status is the way to distinguish them from failure.

Close requests before sessions, sessions before models, then runtimes and
configurations. Repeated close succeeds, as does closing a failed construction.
A parent with live children rejects close. Copying a native owner aliases the
same resource: closing one alias closes it for all aliases. The last-operation
diagnostic remains local to each value. Results, packed-value copies, generation
chunks and diagnostic snapshots own their data and outlive request closure;
they have no native `close()` method. VM teardown is a final cleanup path, not
a substitute for explicit shutdown in a long-lived application.

## Configuration

Set options before constructing the resource that should use them. Each factory
copies configuration; later setters do not mutate already-created resources.
Failed setters leave the previous configuration unchanged. Validation occurs
after each setter, so reduce aggregate `request_tokens` before reducing
`request_rows` or `context_tokens`. Requests must match their prepared session
except that row/token/byte limits and generation output-token limits may decrease.

| Integer key | Default | Meaning and supported limits |
| --- | ---: | --- |
| `memory_bytes` | 4,294,967,296 | Positive RAM admission budget per runtime, at most 2^40 bytes. Conservative reservations, not a process RSS hard limit. |
| `vram_bytes` | 4,294,967,296 | Positive GPU admission budget, at most 2^40; selection also checks reported available device memory. |
| `max_sessions` | 8 | Positive private-session limit per runtime, at most 64. |
| `threads` | 0 | 0 selects the capability default: 2 for embeddings or GPU generation, 4 for CPU generation; explicit 1–256. |
| `batch_threads` | 0 | 0 uses the effective `threads`; explicit 1–256. |
| `context_tokens` | 512 | Tokens per sequence: multiple of 256, at most 8192 and no larger than model training context. The BGE preset allows 256/512. |
| `request_rows` | 8 | Maximum rows, 1–8; row × context capacity must not exceed 65,536. |
| `request_tokens` | 4096 | Aggregate input-token limit, positive and no larger than row × context capacity or 65,536. Includes special tokens/instructions/template. |
| `request_bytes` | 1,048,576 | Input-byte limit, 1–67,108,864. For generation this separately caps rendered input and accumulated output bytes. |
| `batch_tokens` | 0 | 0 selects the capability physical batch. Embeddings need the entire admitted token batch (explicit value >= `request_tokens`, <=4096); generation permits explicit 1–128. |
| `output_tokens` | 32 | Positive per-row generation output limit, no larger than `context_tokens`; input plus this reservation must fit each sequence. |
| `seed` | 1234 | Nonnegative, <=2^40; retained configuration identity. Greedy sampling does not use randomness. |

| Setter/key | Default | Supported values |
| --- | --- | --- |
| `set_text("hardware_mode", ...)` | `auto` | `auto`, `cpu`, `required-gpu`. |
| `set_text("backend", ...)` | `auto` | `auto`, `cpu`, `metal`, `cuda`, `vulkan`; a GPU name is an explicit requirement. |
| `set_text("devices", ...)` | empty | One inventory device name or ID; no comma-separated multi-device placement. |
| `set_text("sampler", ...)` | `greedy` | Only `greedy`. |
| `set_text("pooling", ...)` | unset | General embeddings require `cls`, `mean` or `last`. |
| `set_text("normalization", ...)` | unset | General embeddings require `l2` or `none`. |
| `set_text("query_prefix", ...)`, `document_prefix` | unset | General embeddings require both explicitly; empty is valid. |
| `set_text("chat_template", ...)` | GGUF metadata | General generation: supported engine template or explicit `raw`. |
| `set_text("system_prompt", ...)` | empty | Default system message for general chat generation; forbidden with raw. |
| `set_float("temperature", ...)` | 0.0 | Only 0.0. |

Unknown options, nonfinite/unsupported floats, negative integers and conflicting
hardware settings fail. Model context, batch and actual allocation constraints
are also checked when sessions are created/prepared. A syntactically valid
setting is not a promise that hardware can allocate it.

General profiles reserve `2 × file_bytes + 128 MiB` for weights, then a
geometry-dependent envelope for KV state, activations, attention, feed-forward
scratch and vocabulary output. The estimate uses admitted context/rows and
validated dimensions, layers, heads and state widths. Bounds are checked before
engine allocation; exceeding the configured budget returns `-8`. The original
presets retain their historical reservation formulas. These reservations govern
admission and cleanup accounting, not the engine's peak allocator consumption.

## Hardware selection and inspection

The runtime loads verified packaged backends once per process and inventories
devices. x86 CPU packages contain variants; the runtime selects a usable scored
variant rather than assuming a new CPU instruction set. GPU inventory is ordered
and deduplicated where the backend supplies identity. `auto` chooses the first
eligible packaged GPU whose reported free memory and configured reservation
budget fit the model plus context estimate. It otherwise reports CPU fallback.
This is a bounded policy with fixed defaults, not an autotuner or a guarantee
of the fastest possible configuration. There is no automatic multi-GPU split.

`required-gpu` fails if it cannot use a GPU. An explicit GPU `backend` or `devices`
override also fails if unavailable instead of silently falling back. A runtime
package/CPU backend integrity failure is an error, not a fallback opportunity.
Configuration must remain compatible with the selected model at session creation.

Use `runtime.device_count()` and one-based `runtime.device_info(index,key)`:
`name`, `backend`, `description`, `device_id`, `type`, `memory_free`,
`memory_total`, `exclusion`. These are strings; memory values are decimal byte
counts captured during inventory. Model selection refreshes free-memory checks.
An exclusion explains why automatic selection skips a device; an explicit
backend can bypass automatic duplicate preference, not hardware constraints.

After model readiness, inspect `backend`, `device`, `placement`, `selection`
and `gpu_layers`. Offloaded layers and actual computation are required for GPU
qualification; the existence of a backend file or a GPU name alone is not proof.

| Owner / getter | Useful keys |
| --- | --- |
| Runtime `info_int` | `reserved_bytes`, `reserved_vram_bytes`, `active_sessions` and integer configuration keys. |
| Model `info_text` | `runtime_build`, `state`, `error`, `sha256`, `profile`, `backend`, `device`, `selection`, `placement`, `shared_model_id`, matching `embedding_spec` or `generation_spec`. |
| Model `info_int` | `load_count`, `load_us`, `gpu_layers`, `dimensions`, `model_bytes`, `owner_count`, `peak_owner_count`, reservations and configuration keys. |
| Session `info_int` | `prepare_us`, `completed_requests`, `effective_threads`, `effective_batch_threads`, `effective_context_tokens`, `effective_batch_tokens`, reservations and configuration keys. Effective values require a context. |
| Request `info_text` | `state`, `error`, `runtime_build`. |
| Request `info_int` | `rows`, `input_tokens`, `input_bytes`, `output_bytes`, `last_work_tokens`, `maximum_work_tokens`, `prefill_calls`, `token_calls`, `decode_calls`, `submit_us`, `process_us`, reservations and configuration keys. |

`effective_context_tokens` is the whole allocated context across rows;
`context_tokens` is per sequence. Timings are local diagnostics, not model
benchmarks. `load_count` is 1 once the shared model is loaded. Allocation IDs are
process-local inspection values, not persistent identifiers for an index.

## Shared weights, admission and scheduling

Compatible owners within one process share a model by artifact hash, profile,
engine and backend/device placement. They retain private contexts, KV state,
requests, inputs, outputs and diagnostics. Keep native owners inside their
creating VM. Pass strings/configuration values and output bytes to workers and
construct their owners there, as the shared examples do. Sharing does not extend
across independent processes and does not make contexts free.

Reservations are conservative: current weight allowances are 160 MiB for BGE
and 800 MiB for Smol, plus configuration-dependent context/request allowances.
Each runtime accounts for its admitted resources, even when another runtime
shares the same weights. Budgets are not an OS-enforced process-wide allocation
ceiling. Applications must coordinate totals across workers/runtimes and across
both resident models; inspect actual RAM/VRAM as well as reservation counters.
There are at most four simultaneous model-loader jobs process-wide. A session
admits one active request; saturation rejects work instead of creating an
unbounded internal queue. Applications own queuing/backpressure and task pools.

Embeddings process one admitted noncausal attention batch as an indivisible
compute unit: even `process(1)` processes that batch. Split work into smaller
requests to reduce this unit. Generation `process(n)` submits at most one
bounded prefill/decode unit, capped by token budget and physical batch; ready
rows can decode together. Neither API's work budget is a wall-clock deadline.
Context allocation, GPU completion and competing workers can delay return.
Use explicit preparation, then handle other work/cancellation between calls.

`request.cancel()` ends a building/running request and releases its active-session
slot. Completed generation rows keep their finish reasons; unfinished rows are
cancelled. Read available complete output, then close. Embedding output is only
available for a complete batch. To stop preparation, close the session at a
returned API boundary. `model.cancel()` requires no live child sessions; it
cancels this owner and only requests shared loading cancellation when no other
owner needs that load. Closing the last model owner drains its loader and frees
weights; engine/backend modules remain loaded for process lifetime.

## Output representation

Embedding `result.values()` is a row-major `.packedfloat` containing native
packed doubles. Row numbers are one-based; packed element indexes are zero-based:
`(row - 1) * dimensions + dimension_index`. BGE produces 384 values per row.
The bridge widens native float32 inference values for this existing cREXX type;
there is no text serialization. `rxvector..encodef32le(values)` produces portable
four-byte little-endian values for SQLite blobs or sidecar storage;
`decodef32le` restores a packed value. The example checks exactly 3072 stored
bytes for two 384-dimensional rows. A raw `.packedfloat.binary()` snapshot is
host-native double storage, not that portable f32 format.

Generation `read(row)` consumes new output for a one-based input row. Its chunk
owns `text()`, newly sampled non-EOG `tokens()`, `finish()` and `row()`. Text is
complete UTF-8, including U+0000; a token extending an incomplete scalar may
produce no text yet. Reading again does not replay already-consumed text/tokens.
An empty finish means the row is running; final values are `eos`, `output_limit`,
`cancelled`, `error`. Immediate EOS may legitimately produce empty text.
Separate requests reset private KV state; this is not an implicit conversation
history service. Applications own history and prompt construction.

C RXPA publishes generated strings with the optional size-checked
`SETSTRINGLENGTH(host,value,data,byte_length)` service. Native callers specify
bytes; the host validates UTF-8, counts codepoints, owns storage and handles
embedded NUL. Consumers do not provide five-NUL padding or use `strlen` on
length-bearing text. The unversioned legacy initializer is unchanged. Hosts
without the required counted service reject generation explicitly. Embeddings
retain their original service requirements.

## Error handling and recovery

| Status | Category / response |
| --- | --- |
| `0` | Call succeeded; separately check lifecycle state. |
| `-1` | Invalid input, option, UTF-8, selector, unsupported profile or property; inspect the diagnostic and correct the call. |
| `-2` | Invalid, stale or foreign-VM handle; keep owners in their creating VM. |
| `-3` | Closed resource; create a new owner or use an unclosed resource. |
| `-4` | Wrong resource kind/capability operation. |
| `-5` | Native/published-output allocation failure. |
| `-6` | Backend, package, upstream execution or native exception; inspect operation/message and model/request error. |
| `-7` | Lifecycle/ownership conflict or unavailable native session; prepare first, finish/cancel active work, or close children before parents as appropriate. |
| `-8` | Admission, memory, row/token/byte limit or context allocation failure; reduce admitted work or provision a suitable budget/device. |

Some load failures arrive asynchronously through state `failed` after a
successful `model()` call. Do not ignore that state because `status()` is zero.
An `add_all` row failure retains the original error and cancels the building
request, preventing a silently shortened batch. Never treat a nonzero error
or absent expected PASS marker as success merely because a VM exited zero.
After cancellation/failure, close the request and create another on the prepared
session where it remains usable. Close/recreate failed model/session owners.
