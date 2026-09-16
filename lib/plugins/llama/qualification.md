# Capabilities and qualification status

[Guide index](README.md) · [Install](installation.md) · [Reference](reference.md)

Status as of 16 September 2026: embeddings and generation have normal and
Apple-ASan macOS CPU/Metal evidence, including installed and relocated native
consumers. Adrian approved STEP-07's completed guides/examples on 15 September;
the broad local Apple-ASan gate now passes 2,349/2,349 tests. STEP-06 remains open
for the remaining platform and acceptance conditions below.
Documentation readiness is not release acceptance. The authoritative repository
plan is `docs/planning/native-inference-backlog.md`; current coverage and review
are tracked in `docs/qa/native-inference-step07/README.md` and the live
`docs/qa/native-inference-step06/README.md` qualification ledger.

The separate remote `temp/llama-release-combined` candidate (local branch
`temp/llama-release-qa`) supplies prebuilt CPU/GPU delivery and a generated-fixture
smoke test. Its numbered outcomes and criteria are tracked in
`docs/planning/native-inference-ci.md`; the [current build-status table](../../../docs/qa/native-inference-ci/README.md#current-overall-status--16-september)
distinguishes tested revisions and outstanding integration/release work.
Routine CI downloads no trained model: its sub-5-MiB fixture checks engine
generation/embedding plumbing, package integrity, public-provider discovery
and profile rejection, plus installed VM and relocated native consumers.
It does not establish BGE retrieval quality, Smol output quality or execution
on unavailable GPUs. Neither `develop` promotion nor a release is implied.

## Generated-fixture package evidence

These checks exercise installed VM and relocated native consumers, provider
integrity/discovery/rejection and bounded engine computations. They do not use
the trained BGE/Smol models. The complete Build matrix, Deep Build and core
Sanitizer gates all passed at `f10e70ee5`. Later inheritance and installer changes
have focused passing evidence on their recorded revisions. The latest candidate
still needs the newer develop #699 fix and CI-D03/signing completion; these
earlier passes are not a final integrated-delivery or signed-release verdict.

| Package | Complete baseline result (`f10e70ee5`) | Actual fixture computation |
| --- | --- | --- |
| Linux x64 CPU/Vulkan | Pass; split archives and native relocation checked. | CPU; runner reports no GPU. |
| Windows x64 MSVC CPU/Vulkan | Pass, including restricted-PATH VM/native consumers and unique runtime metadata. | CPU; runner reports no GPU. |
| macOS arm64 CPU/Metal | Pass; split archives and native relocation checked. | CPU and Metal. |
| macOS x86_64 CPU | Pass; archive has CPU variants and no Metal backend. | CPU. Intel Metal is unsupported for this delivery (CI-D02). |
| Linux x64 CPU/CUDA | Pass; split archive, redistributables, notices and native relocation checked. | CPU; runner reports no GPU. |
| Windows x64 MSVC CPU/CUDA | Pass, including the repaired final native relocation and manifest checks. | CPU; runner reports no GPU. |

[Build 35076278681](https://github.com/adesutherland/CREXX/actions/runs/35076278681)
attempt 2 includes all six plugins, four core archives and the MinGW core gate.
[Deep 35076281099](https://github.com/adesutherland/CREXX/actions/runs/35076281099)
and [Sanitizer 35076283269](https://github.com/adesutherland/CREXX/actions/runs/35076283269)
pass on the same baseline. Subsequent actual unsigned Mac and Windows installer
lifecycle tests also pass using those retained binaries. Signed/notarized and
offline Gatekeeper evidence remains open; see the current status table above.

The pipeline evidence ledger retains job identities, archive hashes and failures.
Historical MinGW plugin checks at `4a0924ee1` remain evidence for that source
configuration; MinGW now has a mandatory core-only QA gate and no binary delivery.
Use `rxvm` as the public entry point: archive inspection verifies its relative
selected-VM symlink on Unix/macOS and identical executable copy on Windows.

## Trained-model platform and delivery matrix

| Target / backend | BGE embeddings and Smol generation | Installed VM / relocated native | Remaining proof |
| --- | --- | --- | --- |
| macOS arm64 Apple M5 / CPU | Verified normal and Apple-ASan persistence, batches, output and sharing; full local ASan gate passes. | Normal and Apple-ASan installed VM / relocated native matrices pass in both optimization modes. | Remaining platform/resource-failure coverage and publication gates. |
| macOS arm64 Apple M5 / Metal | Verified real offload/compute, private contexts and shared weights; focused and full local Apple-ASan gates pass. | Normal and Apple-ASan installed/native matrices pass; documented native examples also pass with networking denied. | Remaining target-device failure/drain coverage. Apple unified memory is not discrete VRAM proof. |
| Linux x86-64 / CPU | Backend/build implementation present; target qualification pending. | Pending. | Slower Intel host, CPU variants, complete install/native and supported-platform leak checks. |
| Windows x64 / CPU | Backend/build implementation present; target qualification pending. | Pending. | Actual Windows/MSVC, DLL/package behavior and slower Intel hardware. |
| Linux / CUDA and Vulkan | Backend integration present; real-device qualification pending for each selected cell. | Pending. | Actual GPU/driver identity, computation, numerics, memory, sharing and packaging. |
| Windows / CUDA and Vulkan | Backend integration present; real-device qualification pending for each selected cell. | Pending. | Actual GPU/driver identity, computation, numerics, memory, sharing and DLL deployment. |

GitHub OS runner labels do not establish GPU capability. The Intel Mac fixture
check above does not supply trained-model evidence. Windows ARM and other
combinations are not implied by another cell's result. Unavailable agreed hardware cells remain open for an explicit
disposition; no documentation claim silently removes them from acceptance.

| Capability | Current contract |
| --- | --- |
| In-process hosting | Packaged llama.cpp/GGML; no separate inference server or runtime CLI. Models provisioned separately. |
| Automatic hardware use | Select one eligible packaged GPU within budget, otherwise CPU; explicit overrides and required-GPU errors. |
| Persistent processing | Explicit model load and context preparation; repeated bounded batches retain state/resources. |
| Shared weights | Compatible owners in one process share immutable weights; contexts and request state stay private. Local 1/2/4-owner evidence. |
| Embeddings | BGE-small EN v1.5 F16, CLS/L2, 384 dimensions, query/document preparation, packed output. |
| Generation | SmolLM2 360M Q8_0, independent system/user prompts, greedy decoding, bounded incremental complete UTF-8 and explicit finishes. |
| Other models/samplers | Not qualified by these examples; arbitrary GGUF support is not promised. |
| Multi-GPU placement / cross-process model registry | Not implemented by this contract; single device and in-process sharing only. |
| Automatic model downloads / conversation history / RAG index management | Application/provisioning responsibilities. |

## Retained indicative evidence

STEP-05 retained 2,347/2,347 ordinary Debug CTests, 24 additional Debug consumers,
32 installed Release VM runs and 16 relocated native runs. STEP-04's typed
embedding proof includes 28 Debug and 42 installed/relocated runs. These counts
describe local historical evidence, not present multi-platform or sanitizer
qualification. The STEP-07 ledger identifies unchanged inputs and checks that
can be reused; new documentation does not require replaying the full suite.

STEP-06 adds local Apple-ASan repeated generation and embedding workloads,
typed/legacy interfaces, native objects and four-worker ownership controls.
Installed/relocated matrices pass 42 embedding and 48 generation executions;
the latter combines 18 retained prefix executions with a successful 30-execution
continuation after correcting an example's whole-workload deadline. Package
dependency, fallback, cache and foreign-handle failure controls also pass.
The functional examples now use wide hang backstops, and deadline-sensitive
QA runs serially while each worker test preserves its deliberate concurrency.
These scheduling costs are not new performance measurements.
The complete local Apple-ASan build, QA preparation and 2,349 CTests also pass,
with no sanitizer diagnostic in the retained logs. The ASan selection adds two
available SQLite-ODBC VM tests to the 2,347 names from the retained normal run.
Apple LeakSanitizer is unsupported. Later Linux core ASan/LSan and separate
first-party bridge leak checks passed as recorded in the SAN-009 closure below;
these do not establish upstream-engine sanitizer coverage.

The accepted matched generation glue comparison recorded mean paired differences
of -0.00%/+2.66% on CPU and -16.82%/-2.52% on Metal for one/four rows. It did not
reproduce the material positive Metal slowdown previously observed in embeddings.
The accepted unexplained embedding observation NI-S4-P01 remains unexplained;
generation evidence does not repair it. These are indicative fixed-workload
figures, not claims that cREXX accelerates the engine, model throughput rankings
or a reason to tune upstream code. Raw evidence is in repository
`performance/evidence/2026-09-15-ni-s5-first-release` and
`docs/qa/native-inference-step04`.

With one/four private generation contexts, recorded cREXX process peak RSS was
approximately 0.97/1.47 GiB on CPU and 0.61/1.10 GiB on Metal. All direct/bridge/VM
comparisons stayed within the predeclared allowance (64 MiB per process plus
16 MiB per active session), and the repeated-work controls found no retained RSS
growth beyond warm-up. These individual M5 observations are not capacity promises
for other hosts. The shared examples check allocation identity; actual RAM/VRAM
measurements remain a separate requirement on each backend.

Generation's isolated Release process calls stayed below the fixed 250 ms target.
Under concurrent four-owner work, observed maxima reached 2.64 s CPU / 462 ms
Metal. Work budgets bound submitted computation, not elapsed time or GPU
preemption. Do not use these examples to claim a hard response-time service level.

## Open acceptance items

SAN-009 closed on 16 September after its permanent regression, original
lifecycle trigger, full Apple-ASan gate, separate Linux first-party bridge
ASan/LSan proof and both broad core sanitizer gates passed. Linux build/test
leak detection remained enabled; the separate bridge probe linked an ordinary
uninstrumented engine. See the [closure record](../../../docs/SANITIZER-WORKLIST.md#san-009--cpu-backend-probe-unloadreload-re-registers-apple-asan-globals).
This does not establish upstream-engine sanitizer coverage.

STEP-06, owned by Codex under Adrian's direction, still owns target-OS/real-GPU coverage, low-memory and driver-failure cases, remaining
platform package checks and exact-head hosted gates before publication. Local
enforced-offline and installed/relocated checks have passed. Remaining device
and model criteria are not closed by the focused Linux ownership probe.

Exact conversion ancestry for the distributed GGUFs also remains open under
AC-10; [model hashes and provenance](models.md) distinguish those questions.
All parent AC-01–14 remain open for full-product disposition. The local phase
closures and these guides do not authorize a sanitizer-clean or release-ready
claim. Final QA will update this page and the coverage ledger with actual evidence.
