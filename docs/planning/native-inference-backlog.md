# Native inference requirements and implementation plan

Status: plan and STEP-01 output approved by Adrian, 2026-09-14; STEP-02 complete; output ready for review.
The original requirements were captured on 10–11 September 2026. Adrian's
14 September direction requires GPU support from the first delivery, runtime
hardware detection and effective hardware use, persistent startup/preparation,
and repeated/batch processing. These replace the earlier CPU-first delivery
sequence and later-GPU proposal. Embeddings and a lightweight generation
example are both intended outcomes. Adrian approved the plan and authorized
STEP-01, with STEP-07 allowed to start or complete before STEP-06 finishes when
hardware qualification remains outstanding. This is not implemented support.
Scheduling remains with [the project roadmap](../ROADMAP.md); no beta/release date is
assigned by this plan.

## Outcome and ownership

A cREXX application should start and prepare local embedding and generation
models, retain them in long-lived workers, and process repeated requests and
bounded batches using the available qualified CPU/GPU hardware. GPU support is
part of the first complete delivery. Model loading, device setup and warm-up
belong to an explicit preparation stage so their cost can be paid before the
first user request. Model weights remain separate data; the application does
not have to run or administer a model server.

The vision is a small, installable native inference capability that makes
effective use of a laptop or desktop for sustained work from ordinary cREXX
programs. Its intended outcomes are:

1. **OUT-01 — Self-contained execution:** an installed cREXX or native consumer
   performs embeddings and bounded text generation offline after model
   provisioning, with its inference dependencies supplied by the package.
2. **OUT-02 — Hardware use from the outset:** discover usable devices at runtime,
   select and report an effective CPU/GPU configuration, and accelerate both
   supported model capabilities on the initial qualified GPU platforms.
3. **OUT-03 — Persistent, useful processing:** prepare once, process many inputs
   and batches, then close explicitly. Startup and warm-request costs remain
   separately visible. One-shot examples alone do not satisfy this outcome.
4. **OUT-04 — Efficient concurrent ownership:** support bounded concurrent work
   through existing workers; establish whether compatible workers can share
   immutable model weights safely while retaining private inference state.
   Retain measured memory and concurrency evidence, including limitations.
5. **OUT-05 — Reproducible, supported delivery:** qualify small named model
   artifacts, native/dynamic packaging, failure and cleanup behavior, and the
   initial platform matrix. Preserve exact identities and evidence for takeover.

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
library behind a C-facing adapter. The approved backend is llama.cpp and the
provider ID is `rxllama`; concrete API spelling and integration require review
of the [STEP-01 contract and pins](native-inference-step-01.md).

Acceptance: an installed cREXX program produces validated finite vectors from
local weights with no API key, Python environment, HTTP endpoint or model-server
child process. An application-owned worker may host the provider. Missing or
unsupported models, oversized inputs and incompatible options produce typed
diagnostics; truncation must never be silent.
Batch results retain input order and dimensions, and use the existing packed
numeric/vector facilities without introducing a competing vector format.

## CREXX-NI-02 — Runtime hardware selection and initial GPU support

Keep CPU execution available on machines without a usable GPU, and include GPU
acceleration in the initial delivery. The approved initial backends are Metal
on macOS and CUDA/Vulkan on Windows and Linux. Provide automatic/CPU/required-GPU
selection, explicit device overrides and bounded memory/thread/batch settings
through one contract. Discover packaged backends and usable devices at runtime;
report device capabilities, the selected placement and any fallback. The
platform matrix and automatic-selection direction below are approved; concrete
selection still requires the provider calibration/qualification informed by
STEP-02 measurements.

Acceptance: CPU-only machines run without a GPU SDK, and the same packaged
consumer uses the appropriate acceleration on each qualified GPU platform.
Both embedding and generation must exercise real GPU computation. Numeric
tolerance and downstream retrieval compatibility are measured; bit-identical
results are not assumed. A required unavailable backend fails clearly instead
of silently ignoring the request. Missing hardware proof keeps its matrix cell
open; a CPU pass cannot substitute for it.

## CREXX-NI-03 — Persistent model ownership and bounded requests

Keep weights and inference state loaded across requests in an existing
long-lived CREXX worker. Provide explicit startup, model preparation/warm-up,
repeated single/batch processing, request completion and shutdown. Define
VM-local handle ownership, bounded admission, batch/token limits and
cancellation behavior. Applications choose worker counts and memory budgets;
defaults must account for both inference threads and concurrent cREXX workers.
Evaluate shared immutable weights as part of this plan and enable sharing only
where the backend and ownership contract prove it safe; do not assume that
extra workers share their allocations.

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

## CREXX-NI-07 — Initial lightweight local text generation

Deliver a lightweight generation example as a separately typed capability in
the initial scope. Reuse provider packaging, hardware selection and persistent
model ownership while defining generation-specific context limits, independent
prompt batches, token output, cancellation and incremental processing. Preserve
the existing hosted/local HTTP LLM interfaces. A generation session must not
reload the model for every prompt or generated token.

Acceptance: document the supported architecture, pinned weights, licence and
memory requirements, then demonstrate a persistent consumer processing multiple
prompts and a bounded batch on CPU and the qualified GPU paths. Isolate each
request's context, sampler, outputs and diagnostics. Do not promise that every
LLM can run, or that an embedding model can generate text. Embeddings remain
independently usable and do not depend on loading a generation model; generation
is not a dependency of crexx-rag querying.

## Approved design direction

### Name and integration boundary

Use **llama.rexx** as the public-facing name and **rxllama** as the stable
RXPA provider ID and native namespace. Canonical native artifacts would be
`rxllama.rxplugin` and `rxllama.a`/`rxllama.lib`; `llama.rexx` is the project name,
not a replacement native-plugin suffix. Public type and method spelling must
be reviewed before implementation. Use the existing RXPA session and canonical
dependency/packaging facilities, with a C-facing adapter around pinned
llama.cpp. No new language syntax, channel-provider ABI or model-server process
is proposed.

### Initial platform matrix

| Platform | CPU path | GPU paths required for initial qualification | Evidence status |
| --- | --- | --- | --- |
| macOS arm64 | Native CPU | Apple Metal | Open |
| Windows x64 | Native CPU | NVIDIA CUDA; AMD and Intel Vulkan on representative supported devices | Open |
| Linux x64 | Native CPU | NVIDIA CUDA; AMD and Intel Vulkan on representative supported devices | Open |

These are approved qualification targets, not claims about every device in a
vendor's range. Record exact devices, drivers and feature requirements. CPU
support on a platform does not close its GPU rows. Lack of suitable test hardware
must be reported with the cell left open, rather than silently reducing scope.
No release-date commitment is implied by this matrix.

llama.cpp supplies CPU, Metal, CUDA and Vulkan backends, device enumeration and
memory/capability queries, offload controls and multi-GPU placement options.
Dynamic backend loading can let one platform package adapt to different GPUs.
These are integration mechanisms; they do not automatically select an optimal
configuration or supply missing backend binaries/drivers. See the
[backend API](https://github.com/ggml-org/llama.cpp/blob/5266f24da75dc449bd56cbed7addb9c8e4a6a73e/ggml/include/ggml-backend.h),
[model/context API](https://github.com/ggml-org/llama.cpp/blob/5266f24da75dc449bd56cbed7addb9c8e4a6a73e/include/llama.h)
and [build documentation](https://github.com/ggml-org/llama.cpp/blob/5266f24da75dc449bd56cbed7addb9c8e4a6a73e/docs/build.md).

Ship the inference runtime and redistributable backend dependencies with the
provider/application, using only trusted package locations for backend lookup.
The installed machine needs a working supported GPU driver, but no developer
SDK, separate llama.cpp/Ollama installation or runtime Python environment.
GPU dependencies must not prevent the package starting on a CPU-only machine.
Native applications may include packaged backend libraries; static provider
selection must not be advertised as proof of a single-file GPU executable.

### Automatic hardware use and persistent lifecycle

The conceptual lifecycle is **start -> discover/select -> load -> prepare ->
process requests/batches repeatedly -> drain/cancel -> close**. This is a
behavioral outline, not proposed cREXX syntax. Start/preparation belongs inside
the application or its existing long-lived workers; it does not spawn a model
server. Report when preparation is complete and distinguish model load,
backend/kernel warm-up, first result and steady-state processing.

Automatic mode should enumerate usable devices, avoid counting the same GPU
twice through different backends, and choose a qualified configuration for the
model and workload within the application's RAM/VRAM budget. Prefer GPU offload
where it benefits sustained processing; use full offload when it fits, or
supported partial offload with an explicit report. Account for weights, batch
buffers, generation context/KV cache, active worker count and memory headroom.
Do not silently shrink the requested context, truncate inputs, change model
precision or exceed budgets to make the load succeed.

"Maximum use" means effective measured throughput within these constraints.
Simply selecting every device, every CPU thread or the largest batch can be
slower or exhaust memory. Select bounded defaults using retained qualification
profiles and, where needed, bounded preparation-time calibration. Inventory
multiple GPUs and evaluate supported splitting; enable automatic multi-device
placement where the pinned backend proves it useful and safe. Record a reason
when a detected device is not used. Never imply arbitrary cross-backend GPU
pooling is supported.

Expose overrides for CPU, required GPU/backend/devices, context, batch/token
limits, worker count and memory budget. CPU fallback in automatic mode must be
visible; required-GPU mode must fail if it cannot meet its contract. A CPU choice
on a usable GPU host needs an explicit compatibility, memory or measured
performance reason. Selection should remain stable for a live session; a
reconfiguration/load attempt must have an explicit lifecycle and diagnostics.

### Model and concurrent ownership

Propose a synchronized provider-owned model registry keyed by exact artifact
and compatible backend/placement settings. VM-local sessions retain references
to shared models and own private inference contexts, generation caches,
samplers, request buffers and diagnostics. Keep weights alive until all contexts
and in-flight work have finished. Sharing is within one process and compatible
placement; separate processes or different device copies do not imply one
allocation. Account for RAM and VRAM separately, including unified-memory
devices without double-counting them.

Audit model initialization, concurrent execution and destruction on each pinned
backend before enabling shared concurrent entry. Use RXPA session capabilities
appropriately; leaving the provider on the legacy process-wide compatibility
lock must not masquerade as parallel inference. Where independent contexts are
unsafe or slower, evaluate bounded batches on one model-owning worker. Report
that outcome and its tradeoff for Adrian's decision rather than quietly
discarding the shared-model objective. No raw handles cross worker boundaries.

Cancellation must be honest about backend boundaries. The currently reviewed
llama.cpp header describes its decode abort callback as CPU-only. Bound GPU
decode/prefill/batch units and observe cancellation between supported units;
measure cancellation latency and drain outstanding GPU work before releasing
resources. Do not promise immediate interruption of an in-flight GPU kernel.

### First models and examples

The [STEP-01 record](native-inference-step-01.md) pins exact downloaded GGUF
bytes and the upstream revision, records inspected metadata and provenance gaps,
and separates scratch smoke fixtures from real-model qualification.

- **Embedding reference:** `BAAI/bge-small-en-v1.5`, initially a pinned F16 GGUF
  conversion, with Q8 evaluated separately. Its 33.4M parameters, 384 output
  dimensions, 512-token limit and MIT licence make it a small reference
  candidate. Pin the conversion provenance, tokenizer, CLS pooling,
  normalisation and query/document instructions. Reject oversized inputs.
  [Model card](https://huggingface.co/BAAI/bge-small-en-v1.5).
- **Generation reference:** `HuggingFaceTB/SmolLM2-360M-Instruct`, initially the
  publisher's Q8_0 GGUF (approximately 386 MB), with a modest explicit context
  and bounded output budget. Pin its chat template, sampler settings and
  artifact identity. It is a lightweight demonstration model; task quality
  must be measured. [GGUF](https://huggingface.co/HuggingFaceTB/SmolLM2-360M-Instruct-GGUF),
  [model card](https://huggingface.co/HuggingFaceTB/SmolLM2-360M-Instruct).
- Retain Nomic v1.5 as the RAG comparison and BGE-small/EmbeddingGemma as the
  recorded selection candidates. This provider plan does not change a RAG
  library's embedding space, chunking or model default. Corpus retrieval
  quality and any migration remain in the companion RAG workstream.

Examples must include a persistent embedding worker consuming multiple bounded
batches, a prepared generation session processing multiple prompts and a batch,
and a concurrent shared-model demonstration with measured memory. Both model
capabilities use the same discovery and lifecycle contract, with typed
capability-specific operations. Single-input convenience calls may use an
existing session; they must not conceal repeated load/unload cycles.

## Numbered checkable acceptance criteria

All criteria are **open**. Every criterion needs retained evidence linked here
before it can be marked passed. Numeric tolerances, memory allowances and
performance/cancellation thresholds must be fixed in STEP-02 before candidate
results are assessed; they cannot be relaxed retrospectively to make a pass.

| ID | Observable pass condition | Required verification/evidence |
| --- | --- | --- |
| AC-01 | An installed program performs both embeddings and bounded generation offline after model provisioning, without a model server or separate inference-tool installation. | Scratch-install dynamic and native consumer runs; dependency inventory; offline execution. Covers OUT-01/05, NI-01/04/07. |
| AC-02 | The initial matrix runs both supported models on CPU and its named real GPU paths. A GPU run demonstrably offloads and executes computation. | Hardware/driver/build identities, placement diagnostics and native backend execution evidence for each matrix cell; build-only or mocked GPU checks do not close a cell. Covers OUT-02/05, NI-02/06. |
| AC-03 | Automatic selection detects usable packaged devices, chooses a justified configuration within budget and reports actual placement/fallback. Overrides and required-GPU failures behave as specified. | Real GPU/CPU-only controls plus missing-backend, absent-driver, insufficient-memory and unsupported-option tests; device identity/deduplication tests; retained selection decisions. Covers OUT-02, NI-02/03. |
| AC-04 | Explicit preparation makes a persistent session ready; repeated requests and batches do not reload its model. | Load counters/traces; separate startup/warm-up/first-result/warm-request timings; at least 100 repeated requests and 20 batches per model/capability in the qualification workload. Covers OUT-03, NI-03/07. |
| AC-05 | Embedding batches preserve input order and yield finite correctly dimensioned packed vectors with the pinned preparation/pooling/normalisation. No silent truncation occurs. | Single-versus-batch and CPU-versus-GPU comparisons against pinned reference outputs/tolerances; token-boundary, empty-input, malformed-input and dimension controls; existing vector-consumer integration. Covers OUT-01/05, NI-01/05. |
| AC-06 | Generation handles repeated independent prompts, bounded prompt batches and incremental processing with explicit context/output limits, request identity and finish reasons. | Deterministic fixtures where applicable, bounded real-model CPU/GPU examples, end-of-sequence/output-limit tests and cross-request isolation checks; exact CPU/GPU generated text is not assumed. Covers OUT-01/03/05, NI-07. |
| AC-07 | Shared-weight feasibility is established on each GPU backend and CPU. Where enabled, compatible workers use one model allocation per placement with private mutable state and correct teardown. | One/two/four-owner identity and allocation evidence, private/shared RAM/VRAM accounting, isolated-versus-concurrent result checks and failure/teardown stress. Any unsupported case retains evidence and requires Adrian's explicit disposition; it is not silently marked passed. Covers OUT-04, NI-03/06. |
| AC-08 | Admission, queue size, batch/token work, context and total memory remain bounded, including simultaneous embedding and generation model residency. | Saturation/backpressure and rejected-admission tests, low-memory/load-failure controls, measured peak RAM/VRAM and stable post-warm-up memory across repeated workloads. Covers OUT-03/04, NI-03/06. |
| AC-09 | Cancellation, failed loads, failed inference, explicit close and worker/VM teardown release resources without stale handles, cross-request diagnostics or use-after-free. | Normal and maintained sanitizer focused runs, load/close cycles, cancellation at preparation/prefill/decode/batch boundaries and measured GPU cancellation/drain latency. Covers OUT-03/04/05, NI-03/06/07. |
| AC-10 | Exact weights, conversion, tokenizer, preparation, dimensions, quantization, runtime build and backend settings are inspectable and reproducible. | Pinned artifact hashes/manifests; incompatible architecture/pooling/options and altered-artifact tests; model licence/notices in provisioning records. Covers OUT-05, NI-05. |
| AC-11 | Dynamic and native consumers resolve provider and backend dependencies from canonical trusted package locations on every target OS, including CPU-only hosts. | Clean external-consumer install/package checks, missing/wrong backend/provider diagnostics, Windows runtime-dependency inspection and no sibling-checkout dependency. Covers OUT-01/05, NI-04. |
| AC-12 | Automatic hardware/thread/batch choices deliver justified sustained performance without material regression against the agreed direct llama.cpp and explicit-backend controls. | Ordinary Release cold/warm latency, embeddings throughput, prefill/decode rates, CPU/GPU comparisons, one/two/four-worker memory/throughput and bounded multi-GPU evaluation where hardware is available. Retain raw samples and predeclared thresholds; an unexplained negative verdict remains open. Covers OUT-02/03/04/05, NI-06. |
| AC-13 | The full toolchain, both applicable VM modes, supported optimization modes and existing HTTP/vector/concurrency contracts pass their relevant checks. | Regression coverage established before implementation; focused normal Debug and maintained sanitizer runs, required broader gates and exact-head hosted qualification before publication. GPU correctness still needs real-device evidence. Covers OUT-05, NI-01/06/07. |
| AC-14 | Documentation and examples demonstrate preparation, persistent batches, generation, hardware selection, concurrent ownership and shutdown with truthful qualification status. | Runnable installed examples, source documentation tags, capability matrix and evidence links; all unmet ACs and user decisions retained in this plan. Covers OUT-01 through OUT-05, NI-01 through NI-07. |

## Numbered implementation steps

The plan and STEP-01 output are approved; **STEP-02 is complete** and STEP-03
has not started. The steps describe one
complete scope; an intermediate milestone is not permission to omit later
acceptance criteria or GPU/platform work.

1. **STEP-01 — Select the contract and pin dependencies.** Review the proposed
   name, lifecycle, model/session ownership, batching, hardware policy and
   initial platform/model matrix with Adrian. Select an exact llama.cpp
   revision and GGUF artifacts after checking backend/model compatibility and
   redistribution dependencies. Record public API signatures and any required
   package-helper changes for review before implementation. Serves AC-01
   through AC-14; depends on approval of this plan. No ABI/language expansion is
   implied by approval of the provider scope.
2. **STEP-02 — Establish acceptance controls and measurement thresholds.**
   Inventory available real GPU test hosts and leave missing matrix cells open.
   Retain direct pinned-llama.cpp CPU/GPU model/lifecycle controls; define
   numeric tolerance, memory/cancellation budgets, sustained workloads and
   acceptable wrapper overhead before production edits. Add ordinary failing
   acceptance tests with positive controls where practical. Use pinned tiny or
   generated scratch models for fast plumbing smoke tests, while retaining BGE
   and SmolLM2 for real-model acceptance and performance. A scratch pass never
   substitutes for embedding semantics or a required real-GPU cell. Measure any new
   nested aggregate in Debug and sanitizer builds before broad registration and
   set its scheduling/timeouts from that evidence. Serves AC-02 through AC-13;
   depends on STEP-01. **Complete, 2026-09-14:** S2-01 through S2-05 and
   their evidence/limitations are retained in the [STEP-02 record](native-inference-step-02.md).
3. **STEP-03 — Build the packaged CPU/GPU bridge and persistent ownership.**
   Integrate the selected CPU, Metal, CUDA and Vulkan backends; implement
   trusted discovery, memory-aware selection, preparation, shared-model registry
   and VM-local sessions, inspection, admission and cleanup. Build CPU and GPU
   paths together from the first implementation slice. Prove focused lifecycle
   correctness on real CPU/GPU controls. Serves AC-01/02/03/04/07/08/09/10/11;
   depends on STEP-02.
4. **STEP-04 — Deliver persistent embedding batches.** Implement the typed
   single/batch embedding operations, model-specific preparation and packed
   output; demonstrate multiple batches from a prepared worker on CPU/GPU.
   Run focused output/lifecycle/concurrency controls. As soon as the minimum
   correctness checks permit a decisive end-to-end measurement, freeze the
   production slice and run the first ordinary Release comparison against
   STEP-02 evidence. Report to Adrian and stop for direction before broad
   closeout, tuning or the next production slice, as required by the repository
   performance gate. Serves AC-01 through AC-05 and AC-07 through AC-13;
   depends on STEP-03. This is an intermediate embedding verdict, not full
   initial-delivery completion.
5. **STEP-05 — Deliver persistent generation and concurrent processing.**
   After acceptance of the preceding verdict, implement bounded repeated/batch
   generation, incremental token processing and its cancellation contract using
   the same hardware and ownership facilities. Exercise concurrent owners and
   simultaneous model residency; measure sharing and internal-thread/worker
   interactions. Apply the first Release verdict gate to this production slice
   as soon as focused correctness permits; stop for Adrian's verdict before
   broad closeout or further tuning. Serves AC-01/02/03/04 and AC-06 through
   AC-13; depends on the accepted STEP-04 verdict. No performance failure
   silently authorizes dropping GPU, batching or generation requirements.
6. **STEP-06 — Qualify the complete initial matrix and packages.** Following
   accepted implementation verdicts, finish required normal Debug, maintained
   sanitizer, real GPU, dynamic/static installed-consumer, failure and
   concurrency checks. Qualify Windows on actual supported hardware, not from a
   macOS or cross-build result. Reuse unchanged valid evidence; use
   `tools/asan-run.sh` and record first-party sanitizer findings in the canonical
   worklist. Retain exact-head hosted gates for publication. Serves AC-01
   through AC-13; depends on STEP-05 and its verdict.
7. **STEP-07 — Complete examples, documentation and handoff.** Develop and
   finalize reviewed source documentation, persistent examples, backend/model matrix and
   retained measurements in the repository. Reconcile every AC and
   identify remaining work explicitly before requesting any release/publication
   action. Report full completion only when the agreed criteria are verified.
   Serves AC-14 and audits AC-01 through AC-13. Adrian explicitly permits this
   step to start or complete before STEP-06 finishes: develop documentation and
   examples alongside the implementation, and record unavailable hardware and
   pending qualification truthfully. Completing STEP-07 does not close STEP-06,
   its open ACs or platform cells, or authorize a fully qualified release claim.
   Existing first-Release verdict gates still apply to production edits.

## Approval and takeover record

- **Confirmed user direction, 2026-09-14:** GPU from the first delivery,
  runtime hardware detection and effective use, explicit startup/preparation,
  persistent/batch operation, embeddings and a small generation example;
  investigate efficient shared weights across workers.
- **Approved, 2026-09-14:** name/provider ID, initial platform/backend matrix,
  model choices, ownership/selection direction, acceptance criteria and steps.
  STEP-07 may start or complete before STEP-06 finishes; unavailable hardware
  remains an explicit qualification gap. Exact artifact hashes, API details and
  predeclared measurement thresholds remain STEP-01/02 work.
- **Available resources reported by Adrian:** local Apple M5 arm64 Mac and
  slower Intel Linux and Windows machines, plus GitHub runner capabilities.
  Inspect actual runner/device capabilities before assigning proof; do not
  infer a CUDA/Vulkan device from an OS label or a Metal device from arm64.
- **STEP-01 output approved, 2026-09-14:** Adrian approved the recorded API,
  ownership/loader and per-provider package proposal and authorized STEP-02.
  The documented conversion-provenance and platform-evidence gaps remain open
  qualification work; approval does not certify those facts.
- **S2-D01 numerical policy approved, 2026-09-14:** retain GPU acceleration and
  batching; do not pursue upstream determinism changes. Use the version 2 BGE
  tripwire recorded in STEP-02: strict same-layout repeatability, separately
  bounded batch-layout and CPU/GPU variation, plus dimension/finite/norm checks.
  Preserve version 1 failures. Small numerical differences do not by themselves
  invalidate an index; near-tie ranking changes and application-owned corpus
  retrieval qualification remain explicit. No OUT/NI/AC/STEP scope is removed.
- **Current state:** STEP-01 source/artifact investigation and approved contract
  are recorded in [the STEP-01 review](native-inference-step-01.md) and its
  [machine-readable pins](native-inference-step-01-lock.json). Three model files
  have independent size/SHA-256 checks; no inference or build qualification is
  claimed by that record. All ACs remain open. STEP-02 controls, predeclared
  thresholds and current evidence are tracked in
  [the STEP-02 record](native-inference-step-02.md). Conversion
  provenance resolution remains outstanding. The existing uncommitted
  `AGENTS.md` continuity edit is separate guidance work and must be preserved.
- **STEP-02 complete, 2026-09-14:** local CPU/Metal direct controls, twelve
  distinct normal Debug/Apple ASan cases, ordinary red provider tests, positive
  toolchain/worker controls and four final 2+10 Release captures are retained.
  S2-QA01 distinguishes ASan live allocations from allocator-inflated RSS.
  S2-C01/C02 retain whole-prefill deadline failures and establish bounded causal
  prefill at the existing 128-token physical boundary, preserving logical
  prompts/batches, token outputs and deadlines. BGE's noncausal batches and
  approved S2-D01 numeric limits are unchanged. Peak two-model memory is below
  the 4 GiB control budget on local CPU/Metal. Smol CPU absolute timing remains
  too noisy for a 5% overhead verdict; the later matched paired comparison is
  still required. These are control observations, not completed product ACs.
- **Next action:** review the STEP-02 output, then begin STEP-03's packaged
  CPU/GPU bridge and persistent ownership. STEP-03 has not started. Preserve
  GPU integration from the first slice, bounded processing, shared-model/private
  session ownership and all OUT/NI/AC/STEP identities. Read this entire plan,
  STEP-01 and STEP-02 plus repository instructions at takeover. Initial numeric
  and whole-prefill timing failures remain retained; no unavailable platform,
  installed-consumer, provenance or wrapper-performance gate is waived. Do not
  resume the superseded CPU-first/later-GPU sequence.

The linked upstream APIs and examples establish feasibility inputs, not CREXX
implementation proof. GPU presence, GPU execution, useful acceleration and safe
concurrent sharing are separate facts to verify.
