# Native inference requirements backlog

Status: open requirements captured from the 10–11 September 2026 embedding and
query-engine discussion. This is a proposed capability backlog, not implemented
support or an implementation approval. Scheduling remains with
[the project roadmap](../ROADMAP.md); this entry does not change its priority
order or the Release 1 scope.

## Outcome and ownership

A cREXX application should be able to load a supported embedding model, retain
it in a long-lived worker, and embed text locally on an ordinary CPU. A Mac
ingestion workload should be able to use GPU acceleration through the same
application contract. The model weights remain separate data; the application
does not have to run or administer a model server.

CREXX owns the generic native inference provider, typed cREXX interface,
resource lifecycle, backend integration and packaging. Applications own model
selection policy, retrieval, graph traversal, databases, prompts, budgets and
work scheduling. No RAG vocabulary or orchestration belongs in the native
provider. Use the existing RXPA and worker facilities; this backlog does not
require a new plugin ABI, language syntax or durable-service framework.

The companion
[crexx-rag query-engine backlog](https://github.com/adesutherland/crexx-rag/blob/main/docs/query-engine-backlog.md)
owns the consuming product requirements. IDs below are requirement references,
not allocated GitHub issue numbers. All entries are open.

## CREXX-NI-01 — In-process embedding capability

Provide a typed Level-G interface to load a supported model, embed one input or
a bounded batch, inspect its capabilities and release it. Use a generic native
library behind a C-facing adapter. Evaluate llama.cpp as the first backend;
final API spelling, provider ID and dependency version require design review.

Acceptance: an installed cREXX program produces validated finite vectors from
local weights with no API key, Python environment, HTTP endpoint or model-server
child process. An application-owned worker may host the provider. Missing or
unsupported models, oversized inputs and incompatible options produce typed
diagnostics; truncation must never be silent.
Batch results retain input order and dimensions, and use the existing packed
numeric/vector facilities without introducing a competing vector format.

## CREXX-NI-02 — CPU baseline and optional GPU acceleration

Make CPU execution the baseline. Provide optional Apple Metal acceleration and
explicit automatic/CPU/GPU selection through one contract. Report the actual
backend and any fallback. Decide at packaging time whether a platform supports
one selectable build or separate CPU and accelerated artifacts. CUDA and other
accelerators remain separately qualified extensions.

Acceptance: a CPU-only machine can install and run the embedding capability
without a GPU SDK. On a qualified Mac, the same model and preprocessing run on
CPU and Metal. Numeric tolerance and downstream retrieval compatibility are
measured; bit-identical results are not assumed. A required unavailable backend
fails clearly instead of silently ignoring the request.

## CREXX-NI-03 — Persistent model ownership and bounded requests

Keep weights and inference state loaded across requests in an existing
long-lived CREXX worker. Define VM-local handle ownership, request completion,
batch limits, cancellation behavior and explicit shutdown. Applications choose
worker counts and memory budgets. Share immutable weights only if the backend
and ownership contract prove it safe; do not assume that extra workers share
their allocations.

Acceptance: repeated requests load the model once per owner; completed request
state is reclaimed and memory does not grow with request history. Orderly
shutdown and errors release native resources. Concurrent owners cannot mix
inputs, outputs or diagnostics. Requests cross worker boundaries as supported
values/messages, never transferable raw model handles.

## CREXX-NI-04 — Canonical installation and small consumer packages

Use CREXX's canonical dynamic `.rxplugin` and native static-archive packaging
and dependency metadata. Keep inference optional for applications that do not
use it. Support a standalone embedding executable as a thin consumer of the
same provider when useful for diagnostics or integration; it must not own a
second inference implementation.

Acceptance: a scratch installation runs both linked and native consumers with
declared dependencies and no sibling source checkout. Record provider size,
model-file size and peak working memory separately. Model distribution or
acquisition must retain the applicable licence, notices and exact artifact
identity; a prepared offline installation needs no download at query time.

## CREXX-NI-05 — Reproducible model and preprocessing contract

Expose exact model/artifact identity, tokenizer/configuration identity,
parameter and weight-precision information where available, input limit,
pooling/normalisation options, task instructions and output dimensions. Treat
parameter count, weight quantization and vector dimensions as separate choices.
Support the query/document preparation required by each qualified model.
Backend upgrades must not silently change the reported embedding specification.

Acceptance: pinned artifacts and settings reproduce vectors within a stated
tolerance. Incorrect dimensions, missing required preparation and unsupported
pooling or architecture are diagnosed. A compatibility record identifies the
runtime build and numeric backend as well as the model; a familiar model name
or a GGUF filename alone is insufficient qualification.

## CREXX-NI-06 — Qualification and performance evidence

Qualify a small supported-model set before advertising broad compatibility.
Retain runtime/model identities and distinguish CPU-only, Metal and other
platform results. Measure cold load, warm single-query latency, sustained batch
throughput and peak memory at representative input lengths. Include repeated
request cleanup, malformed-model/input handling, both supported CREXX VM paths,
dynamic/static packaging and applicable native sanitizer coverage.

Acceptance: a published capability matrix states supported platforms, model
architectures, precision choices and limitations. Performance claims identify
hardware, batch size, input tokens and whether loading is included. crexx-rag
owns corpus-level retrieval quality; a fast inference microbenchmark does not
close its quality requirement. No hosted model call is needed for this gate.

## CREXX-NI-07 — Optional local text generation

Assess a later, separate capability for generation models supported by the
selected backend. Reuse provider packaging and model ownership while defining
generation-specific context limits, token output, cancellation and, if selected,
streaming. Preserve the existing hosted/local HTTP LLM interfaces.

Acceptance for selecting this extension: document the supported architectures,
available weights, licences and memory requirements, then demonstrate one
bounded local-generation consumer. Do not promise that every LLM can run, or
that an embedding model can generate text. This extension must not delay the
embedding-only component or become a dependency of crexx-rag querying.

## Suggested sequence and design inputs

First review NI-01, NI-03 and NI-05 together; then qualify the CPU package under
NI-04/06 and optional Metal under NI-02. NI-07 is a separately selected extension.
The application benchmark may change the preferred model or backend before the
public interface is frozen.

Primary backend references from the discussion are the
[llama.cpp C API](https://github.com/ggml-org/llama.cpp/blob/master/include/llama.h),
[library and backend build instructions](https://github.com/ggml-org/llama.cpp/blob/master/docs/build.md)
and [embedding example](https://github.com/ggml-org/llama.cpp/tree/master/examples/embedding).
These establish a candidate integration route, not CREXX implementation proof.
