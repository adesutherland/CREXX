# llama.rexx: STEP-01 investigation and contract for review

Recorded 2026-09-14 against CREXX `037e7939bc29eb91b29ed41e9b1b8debdef6353d`.
The [approved plan](native-inference-backlog.md) owns OUT-01–05, NI-01–07,
AC-01–14 and STEP-01–07. This supporting record does not replace that scope.
Status: output approved by Adrian, 2026-09-14; STEP-02 authorized. The concrete
API, loader and package proposal below are the accepted direction. No provider implementation, inference execution,
performance verdict or platform qualification is claimed. All ACs remain open.

## Dependency and model pins

Select llama.cpp **v0.4.0**, peeled commit
`5266f24da75dc449bd56cbed7addb9c8e4a6a73e`, including its in-tree GGML.
The annotated tag object is `5ac847190e979e0da7c4a21806630805f396d487`.
The release API's `target_commitish` is different: do not use that field as the
source pin. The [tag](https://github.com/ggml-org/llama.cpp/releases/tag/v0.4.0)
and [commit](https://github.com/ggml-org/llama.cpp/commit/5266f24da75dc449bd56cbed7addb9c8e4a6a73e)
were resolved separately. Pin the commit, not a moving branch or latest release.

| Role | Artifact | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Embedding reference | BGE-small-en-v1.5 F16 GGUF | 67,308,128 | `f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999` |
| Generation reference | SmolLM2-360M-Instruct Q8_0 GGUF | 386,404,992 | `48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201` |
| Fast generation smoke candidate | Stories260K GGUF | 1,185,376 | `270cba1bd5109f42d03350f60406024560464db173c0e387d91f0426d3bd256d` |

All three files were downloaded outside the repository and independently
checked against the upstream LFS size/hash. The two reference GGUF headers and
metadata were also parsed without executing inference. Exact repository
revisions, resolve URLs, inspected metadata, licences reported by the model
cards, and inspected upstream source hashes are in the
[pin record](native-inference-step-01-lock.json). Model data is not checked in.

**BGE profile:** GGUF v3, BERT architecture, 197 tensors, 384 dimensions,
512-token maximum, WordPiece vocabulary of 30,522 entries, CLS pooling.
Use L2-normalized vectors; document input is unchanged, query input prepends
`Represent this sentence for searching relevant passages: `.
Count the instruction and tokenizer special tokens against the limit. Reject
oversize input; preserve UTF-8 and input order. Compare batched encode against
single-input encode. Return row-major `.packedfloat` values using the existing
packed/vector contracts; float32 inference values widen to the existing packed
float representation. Weight quantization is distinct from output precision.
See the [source model card](https://huggingface.co/BAAI/bge-small-en-v1.5/tree/5c38ec7c405ec4b44b94cc5a9bb96e735b38267a)
and [GGUF converter card](https://huggingface.co/CompendiumLabs/bge-small-en-v1.5-gguf/tree/d32f8c040ea3b516330eeb75b72bcc2d3a780ab7).

**SmolLM2 profile:** GGUF v3, llama architecture, 290 tensors, 8,192 maximum
context, 49,152-token vocabulary, Q8_0 weights. Preserve the exact GGUF chat
template, including its default SmolLM system message when none is supplied.
Pin the template and tokenizer through the artifact hash and retain the literal
template in the pin record. Establish token-level golden controls for rendering
before choosing the wrapper implementation: llama.cpp's core template API is
not a general Jinja evaluator. Start with explicit small context/output budgets
and a greedy control; sampled mode requires explicit sampler settings and seed.
The GGUF's internal training name differs from its public model name; expose
both rather than rewriting its metadata. See the
[publisher GGUF](https://huggingface.co/HuggingFaceTB/SmolLM2-360M-Instruct-GGUF/tree/593b5a2e04c8f3e4ee880263f93e0bd2901ad47f).

**Provenance limit:** exact distributed bytes are pinned, but the reviewed GGUF
cards do not establish the precise converter revision and original source
checkpoint used for those bytes. A current source-model revision is a comparison
input, not proof of conversion ancestry. Resolve this by obtaining provenance
or making a recorded conversion and re-pinning it before AC-10 qualification.
Record converter command, tool/dependency versions, source hashes and output
hash. Q8 BGE remains a separate comparison, not a silent replacement for F16.

## Scratch models and smoke coverage

The pinned upstream
[test-llama-archs](https://github.com/ggml-org/llama.cpp/blob/5266f24da75dc449bd56cbed7addb9c8e4a6a73e/tests/test-llama-archs.cpp)
initializes tiny random-weight models and can save supported architectures as
GGUF. A candidate fixture command, to validate in STEP-02, is:

```sh
test-llama-archs --arch llama --seed 1234 --out FIXTURE_DIRECTORY
```

This is a test-build tool, not a model trainer or product runtime dependency.
Record its build/toolchain and output hash: the initializer also uses C++
`std::hash`, so a seed alone does not establish byte identity across toolchains.
Require the expected output file to exist and load; a successful exit after
skipping an architecture is not a pass. At this pin `arch_supported()` explicitly
skips BERT-family fixtures with `TODO vocab`. Do not claim that it supplies our
BGE embedding fixture. Vocabulary-only GGUFs are also not inference models.

The upstream [tinyllamas collection](https://huggingface.co/ggml-org/models-moved/tree/499bc8821c6b12b4e53c5bffcb21ec206f212d81/tinyllamas)
contains Stories260K (about 1.19 MB) and Stories15M Q4_0 (about 19.08 MB).
Upstream `tests/CMakeLists.txt` uses the latter. Stories260K is our smallest
downloaded smoke candidate; loading/tokenizer compatibility must still be run
against the selected pin. Retain model redistribution provenance/notices before
bundling an upstream test weight file; its presence in a test collection is not
itself a licence grant. A generated fixture avoids a model-weight download.

STEP-02 should establish three complementary levels:

1. Generated/tiny model tests for provider discovery, load failure, lifecycle,
   repeated work, bounded generation, cancellation and installed packaging.
2. BGE F16 and SmolLM2 Q8 controls for real preprocessing, dimensions, batch
   equivalence, generation and private-state isolation. BGE F16 is the initial
   small embedding smoke model until a suitable scratch encoder is validated.
3. Both real models on each required CPU/GPU cell for numeric, memory,
   concurrency and performance acceptance. Toy throughput is not product
   throughput; mocked GPU discovery does not establish GPU execution.

## Proposed public contract

Keep the approved brand `llama.rexx`, native namespace/provider ID `rxllama`,
and canonical `rxllama.rxplugin` / `rxllama.a` / `rxllama.lib`. Propose a thin
Level-G `llama` library with ordinary typed configuration, runtime, model,
embedding-session, generation-session and request classes. Factories construct
these in Rexx and delegate to native functions; no native object-creation ABI
or language change is needed. Distinct names keep the façade import separate
from the plugin. Preserve RexxDoc tags on every exposed operation.

The following is a signature specification for review, not executable Rexx.
Native functions return `.int` status (`0` means the operation succeeded).
Lifecycle state is separate from call status. `out` means an RXPA `expose`
argument. Handles are opaque `.binary` values with checked kind, owner and
generation; the public façade retains them privately. `h` below always has
type `.binary`; keys/text/path/profile/SHA fields are `.string`.

| Native operation | Inputs and outputs |
| --- | --- |
| `configcreate` | `out config: h` |
| `configint`, `configfloat`, `configtext` | `config: h, key: .string, value: .int / .float / .string` respectively |
| `runtimeopen` | `config: h, out runtime: h` |
| `devicecount`, `deviceinfo` | `runtime: h, out count: .int`; `runtime: h, index: .int, key: .string, out value: .string` respectively |
| `modelopen` | `runtime: h, path: .string, sha256: .string, profile: .string, config: h, out model: h` |
| `modelstate`, `modelcancel` | `model: h, out state: .string`; `model: h` respectively |
| `sessionopen` | `model: h, capability: .string, config: h, out session: h` |
| `prepare` | `session: h, work_tokens: .int, out state: .string` |
| `requestopen` | `session: h, config: h, out request: h` |
| `addembedding` | `request: h, text: .string, role: .string, out row: .int` (role `query` or `document`) |
| `addprompt` | `request: h, system: .string, prompt: .string, out row: .int` |
| `submit` | `request: h` |
| `process` | `request: h, work_tokens: .int, out state: .string` |
| `embeddings` | `request: h, out values: .packedfloat, out rows: .int, out dimensions: .int` |
| `readtext` | `request: h, row: .int, out text: .string, out tokens: .int, out finish: .string` |
| `cancel` | `request: h` |
| `infotext`, `infoint` | `resource: h, key: .string, out value: .string / .int` respectively |
| `diagnostic` | `out code: .int, out operation: .string, out message: .string` for the calling VM's last call |
| `close` | `resource: h` (validated kind; explicit, idempotent close of that resource) |

Public class methods expose the corresponding operations with typed owners and
status/results; embedding batch results contain the packed values, row count
and dimension. The façade can supply bulk `.string[]` conveniences by filling
one bounded request. It must not create a model/session for each row. Final
class spelling and the above native signatures are review items.

Contract details:

1. Configuration is validated and snapshotted at creation. Unknown keys, wrong
   types and unsupported combinations fail. Required controls cover hardware
   mode (`auto`, `cpu`, `required-gpu`), backend/devices, RAM/VRAM limits, total
   concurrent sessions, CPU threads for generation and batches, context,
   per-request rows/tokens/bytes, physical compute-batch tokens, output tokens,
   sampler and seed. STEP-02 fixes numeric defaults/thresholds before testing.
2. Model states are `loading`, `ready`, `failed`, `cancelled`, `closed`.
   Propose a bounded private loader job so `modelopen` returns a pending handle
   and `modelcancel` can set a flag consumed by the upstream load-progress
   callback. It invokes no VM APIs from the loader thread; polling publishes
   results in the owning VM. Join/drain before destruction. This private job
   is a concrete review decision, not a new public worker/service abstraction.
3. Sessions require a ready model, own private inference contexts and become
   ready after explicit bounded warm-up through `prepare`. An active session
   accepts one request at a time. Applications use existing workers and
   messages for concurrency; the provider does not create cREXX workers.
4. A request is `building`, `running`, `complete`, `cancelled`, `failed` or
   `closed`. `submit` atomically validates and admits all rows before work.
   Rows preserve insertion order using the existing one-based array convention.
   Batches must use backend batching where supported, with retained evidence
   of the chosen compute layout. Token limits account for every row.
5. `process` advances a bounded unit, then returns control so the worker can
   service cancellation/messages. For noncausal BERT, never split one sequence's
   attention computation across artificial chunks: admit a whole sequence or
   bounded batch. Generation prefill/decode uses bounded token units. A work
   budget is not a hard wall-clock preemption guarantee.
6. Embeddings publish one complete ordered result, not a partially successful
   batch disguised as complete. Generation returns new UTF-8 text per row,
   buffering incomplete token-byte sequences until valid. Finish reasons
   distinguish EOS, output limit, cancellation and error. Independent prompts
   clear prior sequence state and reset private sampler state without unloading
   weights; the pinned template supplies the default system text when absent.
7. Cancellation is observed between supported work units. The pinned abort
   callback is documented as CPU-only; GPU kernels must finish/drain before
   resources are freed. STEP-02 must measure preparation, encode, prefill,
   decode and loader cancellation, including the longest indivisible unit.
8. Closing a parent with live children reports `busy`; explicit shutdown drains
   requests, closes sessions, models, then runtime. VM teardown cancels/drains
   and closes remaining owned resources in reverse order. Copies within a VM
   retain the same resource; stale/foreign-VM handles are rejected. Native
   failures return diagnostics and never cross the C adapter as C++ exceptions.
9. Inspection includes exact identities, effective options, backend/device and
   placement, fallback reason, load count, shared-model identity, active owner
   count, allocated/reserved memory, token counts and preparation/timing state.
   Diagnostics are VM-local; request failures also remain inspectable on the
   request. Do not log prompts or model data by default.

## Ownership and runtime integration evidence

Use RXPA V2 session-aware callbacks and `SESSION_AFFINE` procedures, following
the [existing contract](../ai-context/CREXX_LIBS.md) and
[`rxsqlite` implementation](../../lib/plugins/sqlite/rxsqlite.c). The legacy
process-wide compatibility lock would serialize independent VM calls. Payload
copy/finalize callbacks still need short safe retain/release operations; never
run inference or perform a long GPU drain under a global compatibility lock.

The pinned `llama_context` retains a reference to a `const llama_model`, and
`llama_init_from_model` creates contexts from a loaded model. This supports the
proposed separation; it does not prove concurrent safety on every backend.
Use a synchronized process registry keyed by artifact hash, runtime build,
backend/device placement and model-affecting options. Contexts, KV caches,
samplers, scratch buffers and diagnostics remain private. Deduplicate concurrent
loads; acquire a reference before publishing a handle and retain it through
all in-flight work. Count shared weights once and per-context memory separately.

Audit initialization, lazy allocation and concurrent compute on each backend
before enabling shared entry. Incompatible placement may require another model
copy. OS file mapping across processes is separate from one in-process model
allocation. Use one selected provider implementation per process; accidentally
loading static and dynamic copies can create separate registries. Backend/log
registry initialization and shutdown also need process-wide ownership.

For hardware discovery, load explicit package-owned backend files, enumerate
devices, deduplicate physical CUDA/Vulkan identities where available, and report
each included/excluded device. Apply the approved budget/selection policy and
CPU fallback controls. The pinned `ggml_backend_load_all_from_path` also honors
`GGML_BACKEND_PATH`; merely passing a directory is insufficient for the trusted
package lookup rule. Prefer explicit `ggml_backend_load` of manifest-listed
files, with CPU variant scoring; do not mutate process environment as a fix.
Multi-device placement remains subject to supported split modes and real tests.

## Required package-helper change for review

Current [`add_rxpa_provider_package`](../../rxpa/RXPluginFunction.cmake) copies
the dynamic plugin and static archive. The native
[`crexx` wrapper](../../bin/crexx.crexx) selects those archives from provider
requirements; it does not carry provider-specific dependent libraries or runtime
bundles. Do not assume a CMake target's transitive links survive archive copying.
The global `crexx_native_libs` list is not the right place for optional llama
dependencies needed only by an application selecting `rxllama`.

Recommend a bounded, generic **per-provider native dependency manifest**:

1. Extend the existing CMake helper with declared dependent link targets and
   runtime files/targets. Emit platform/configuration-specific metadata beside
   the canonical provider, retaining current behavior for providers without it.
2. Proposed format: versioned JSON `rxllama.native.json` containing provider ID,
   target platform/architecture, engine build identity, relative link-library
   paths, relative runtime paths and hashes. The wrapper maps these to known
   compiler-driver arguments. Do not accept shell commands or raw executable
   hooks; reject traversal, wrong-platform and missing dependency entries.
3. Resolve the manifest only for selected providers, deduplicate dependencies,
   link the provider archive plus the shared llama import/library dependency,
   and copy its runtime bundle into the native output's package directory.
   Add appropriate relative loader paths on Unix/macOS and packaged DLL search
   handling on Windows. Dynamic provider discovery uses the same bundle.
4. Keep the RXBIN requirement record, RXPA ABI, provider ID and existing
   canonical/legacy resolution intact. Cover metadata absence, malformed data,
   spaces/non-ASCII paths, relocation, missing backends and native consumers
   with no inference dependency. Qualify bundle collisions with other GGML
   users; explicit lookup alone does not prove OS shared-library isolation.

Use upstream shared `llama`/GGML libraries for this initial integration:
`GGML_BACKEND_DL` requires `BUILD_SHARED_LIBS`, and
`GGML_CPU_ALL_VARIANTS` requires backend dynamic loading. The RXPA static
archive can use llama's C exports; its C++ runtime dependency remains in the
shared engine bundle. Avoid the upstream CMake target name `llama` for the RXPA
target; give the provider an internal target name and explicit `PROVIDER_ID`.

Build CPU plus Metal on macOS, and CPU plus CUDA/Vulkan for Windows/Linux
packages from the same pin. Disable `GGML_NATIVE` for portable distributions;
qualify an older-Intel baseline and runtime-dispatched variants. Disable unused
upstream tools/examples/server/network features in the product build; test
utilities belong in a separate QA build. Backend libraries must remain optional
at runtime so missing GPU drivers do not prevent CPU startup. Audit every
redistributed runtime and its notices (including optional OpenMP/vendor
dependencies); SDK/compiler/driver versions and exact bundle hashes remain
platform-build evidence to establish, not a blanket redistribution claim.

## Hardware and remaining review work

Read-only local inventory: Apple M5, arm64, 10 logical CPUs, 24 GiB unified
memory; system profiler reports a 10-core GPU with Metal 4 support. This is
device availability, not evidence that llama.cpp has run on it. Adrian reports
slower Intel Linux/Windows machines; GPU/driver inventory there is pending.
Current CI uses Ubuntu 24.04, Windows 2025, macOS 15 arm64 and macOS 15 Intel
labels. A runner label does not establish usable GPU access; probe actual
capabilities before assigning a cell. No extra hosted GPU resource was started.

Adrian approved the STEP-01 output and authorized STEP-02 on 2026-09-14.
The recorded conversion-provenance gap remains an AC-10 qualification item.
[STEP-02](native-inference-step-02.md) establishes direct builds/runs, fixture viability, exact preprocessing,
CPU/Metal controls on the M5, other-host inventory and predeclared thresholds.
No direct upstream inference build has been run in STEP-01. Source support and
GGUF metadata are feasibility inputs, not compatibility qualification.

STEP-07 may progress or finish before STEP-06 under Adrian's amendment. It must
retain open hardware cells, ACs and evidence gaps. Existing first-Release
performance verdict and sanitizer gates remain in force; no documentation
milestone can close them or reduce the GPU/concurrency outcome.
