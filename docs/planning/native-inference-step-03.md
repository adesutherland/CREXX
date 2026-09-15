# STEP-03 — Packaged bridge and persistent ownership

Status: authorized by Adrian on 14 September 2026; lifecycle/package implementation
and S3-D01 repair pass local native/Debug qualification. Adrian approved phase closure
on 2026-09-14, assigning remaining sanitizer proof to STEP-06 native-inference
release QA, owned by Codex under his direction. SAN-009 remains open.
Baseline commit: `c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8`.

## Vision and scope

The authoritative [complete plan](native-inference-backlog.md), OUT-01–05,
CREXX-NI-01–07 and AC-01–14 remain unchanged. Deliver llama.rexx / rxllama
inside cREXX programs: separately provisioned model weights, packaged inference
dependencies, CPU and GPU support from the outset, explicit preparation,
persistent batches and efficient shared weights with private worker state.
This step supplies the packaged bridge and ownership on which STEP-04 embedding
and STEP-05 generation operations depend. Their request APIs and performance
verdicts remain later steps; no stubs will count as implementing them.

## Numbered checkable acceptance criteria

These are STEP-03 checkpoints, not replacements for the full product ACs.
The reconciliation below preserves partial results and outstanding obligations. Missing hardware qualification stays in STEP-06; STEP-07 may
overlap it as already approved.

1. **S3-AC-01:** Optional, pinned source builds provide CPU plus the selected
   Metal/CUDA/Vulkan backends, with portable CPU settings and no product server,
   model download, Python or network runtime dependency.
2. **S3-AC-02:** Trusted, manifest-listed backend discovery reports devices and
   effective placement. CPU, auto and required-GPU modes diagnose missing
   backends, unsupported settings and insufficient memory without silent changes.
3. **S3-AC-03:** Configuration snapshots, asynchronous model loads, private
   sessions and real explicit preparation work for BGE and Smol on CPU/Metal.
   Compatible owners share one immutable model allocation; teardown drains work.
4. **S3-AC-04:** VM-local, kind-checked opaque handles survive ordinary copies,
   reject foreign/stale handles, enforce parent/child close order and preserve
   isolated diagnostics. Finalizers never perform long work under the RXPA lock.
5. **S3-AC-05:** Admission reserves bounded weights/context/working memory across
   co-resident models and sessions, rejects saturation, and releases reservations.
   Inspect exact model/runtime identity, preparation, placement and ownership.
6. **S3-AC-06:** Declarative per-provider native metadata carries validated link
   dependencies and runtime bundles. Dynamic and native consumers run from a
   relocated scratch package; unrelated providers retain their existing behavior.
7. **S3-AC-07:** Focused ordinary red tests precede implementation. Retain full
   rxc/rxas/rxlink/VM lifecycle checks, concurrency/failure controls and matching
   normal Debug/maintained sanitizer results, with exact platform limitations.

## Numbered implementation steps

1. **S3-01:** Commit the approved baseline; record authorization and these
   checkpoints; establish ordinary failing lifecycle/package controls.
2. **S3-02:** Integrate pinned shared engine and optional CPU/GPU build/package
   dependencies, trusted discovery and native dependency metadata.
3. **S3-03:** Implement session-aware C-facing RXPA lifecycle, model registry,
   bounded loader, hardware/memory selection, preparation and inspection.
4. **S3-04:** Exercise real models, copied/foreign handles, concurrent ownership,
   load cancellation, failures, saturation and explicit/VM teardown.
5. **S3-05:** Qualify local dynamic/native relocated packaging and focused
   Debug/sanitizer controls; reconcile every S3 checkpoint and full-plan status.

## Evidence and decisions

- Baseline commit preserves the exact STEP-02 evidence bytes, including upstream
  log whitespace, and its 360 checked file hashes. No unchanged QA was repeated.
- User authorization explicitly approves STEP-02 and implementation of STEP-03.
  All full product ACs remain open. No publication or push has been requested.

### Implementation and current evidence

- S3-01 is complete. The baseline commit includes the accepted plan, repository
  continuity instructions and STEP-02 controls/evidence. The ordinary lifecycle
  test failed with 29 missing-function diagnostics before implementation; its
  [original log](../qa/native-inference-step03/lifecycle-before-provider.log)
  is retained. The later embedding/generation request controls remain open.
- Optional pinned CPU/GPU source integration, trusted manifest discovery,
  asynchronous model loads, VM-local handles, private preparation contexts,
  admission accounting and shared immutable model ownership are implemented.
  See the [provider guide](../../lib/plugins/llama/README.md) for the precise
  supported settings and public native lifecycle functions.
- Both RXPA forms use the same shared C++ bridge. The native C driver consumes
  declarative per-provider metadata, verifies actual dependency bytes, and copies
  a relocatable bundle. Project cache hits revalidate selected metadata and
  dependencies; unrelated provider metadata is ignored. The ordinary existing
  native-project contract passed, including its unchanged-provider paths.
- Normal Debug: eight provider tests plus the existing project-build contract
  passed (9/9, 150.08 seconds). Both models exercised CPU and real Metal offload,
  load sharing, private preparation, cancellation, invalid handles/options,
  auto memory fallback, required-GPU rejection, co-resident admission and cleanup.
  [Focused results](../qa/native-inference-step03/debug-focused-ctest.log) and
  [detailed output](../qa/native-inference-step03/debug-focused-details.log)
  retain the actual checks. Later CMake edits add package-harness prerequisites;
  the provider/driver/test logic from this run is unchanged.
- A complete scratch install outside the checkout passed native relocation for
  BGE/Smol on CPU/Metal, selected-package cache invalidation, corrupted dependency
  rejection without replacing the executable, unrelated metadata isolation,
  missing runtime-manifest rejection, and missing-GPU fallback/required-GPU
  failure despite an ambient `GGML_BACKEND_PATH` pointing to usable modules.
- Actual dynamic cREXX task workers passed binary transport, foreign-handle
  rejection and four independent VM owners of each shared model on CPU/Metal.
  Each worker prepared and closed its own private context. These checks also
  passed from the relocated scratch installation under maintained Apple ASan.
- The isolated complete package target measured 82.100 seconds in normal Debug
  and 175.305 seconds in maintained Apple ASan after its product-install
  prerequisites were built. These are functional harness scheduling measurements,
  not inference performance verdicts. The target stayed explicit pending
  S3-D01's native-worker repair. Those historical measurements do not cover the
  expanded native-worker matrix recorded below; the target still needs its new
  sanitizer measurement before CTest registration.
- The initial sanitizer package run stopped because unrelated installed UI
  bytecode had not been built. The harness now declares the UI/example install
  prerequisites, and the same scratch installation/matrix passed. This was a
  harness preparation error, not a sanitizer finding or skipped installed file.
- Maintained Apple ASan: the same eight provider tests plus the existing
  project-build contract passed (9/9, 487.68 seconds).
- Review subsequently found CPU/device and session/device conflicts were
  silently ignored. A permanent ordinary negative test reproduced the gap;
  the bridge now rejects both conflicts. All eight affected provider/four-tool
  tests pass on the pre-S3-D01 candidate in Debug (56.90 seconds) and maintained
  Apple ASan (158.65 seconds). The native driver and packaging logic are unchanged,
  so their retained broader checks are reused.
  SAN-009 remains open regardless of a local focused pass. Apple LeakSanitizer
  is unsupported; Linux ASan/LSan and broad/platform gates are not claimed.
- After Adrian approved S3-D01 and accepted its measured per-legacy-call cost,
  the expanded Debug scratch-install matrix passed in 114.687 seconds. It adds
  native transport, foreign-handle rejection and four actual cREXX workers
  sharing each real model with private preparation on CPU/Metal, alongside the
  retained dynamic/package/cache controls. See
  [post-approval evidence](../qa/native-inference-s3d01/README.md). Broader normal
  Debug passes 2,314/2,314 in 900.23 seconds after `qa-prep`, excluding the
  `performance-measurement` label. Prior sanitizer results predate the VM repair;
  no sanitizer build or test has been run since Adrian's hold.

### S3 checkpoint reconciliation

| Checkpoint | Current disposition and remaining work |
| --- | --- |
| S3-AC-01 | Implemented and built locally for CPU/Metal. CUDA/Vulkan build integration and x86 CPU variants are present; their Windows/Linux/device/package evidence remains STEP-06 work. |
| S3-AC-02 | Local CPU/auto/required-GPU discovery, placement and failure checks pass. Selection is the measured initial profile plus explicit overrides and conservative memory admission; full adaptive tuning, partial/multiple-GPU placement and unsupported hardware remain open under the original ACs. |
| S3-AC-03 | Real model load, repeated preparation, shared weights/private contexts and teardown pass locally in the bridge and actual dynamic/native cREXX workers on CPU/Metal. Remaining sanitizer/platform proof stays open. |
| S3-AC-04 | Local copied/stale/kind/foreign-handle and parent/child lifecycle checks pass. Actual dynamic and native worker rejection passes; the original timeout and S3-D01 repair remain documented. |
| S3-AC-05 | Co-resident models with four private contexts each pass admission/cleanup. Reserved RAM is 3,690,987,520 bytes; measured process peaks are 1,582,383,104 CPU and 1,262,567,424 Metal bytes in the retained Debug run. These are preparation controls, not complete future request-load or GPU-driver memory qualification. Budgets are per runtime; shared weights are charged once within it. |
| S3-AC-06 | Scratch-installed dynamic/native consumers, runtime relocation, dependency/cache controls, native concurrent consumption and existing project native packaging pass locally. Other platform cells and new sanitizer proof remain open. |
| S3-AC-07 | Permanent ordinary red controls, real-model lifecycle tests, four-tool tests and scratch-install checks are retained. All 2,314 non-measurement Debug CTests pass. Pre-S3-D01 option-conflict/provider regressions pass in both builds; the VM repair and expanded native matrix have Debug proof only. SAN-009's explicit platform disposition remains required; phase closure is approved with sanitizer proof assigned to STEP-06. |

S3-01 through S3-05 are complete as a bounded implementation phase. The remaining
sanitizer acceptance evidence is assigned to STEP-06; it is not marked passed.
The VM repair decision and local native-worker proof are satisfied.
**No full-product AC-01 through AC-14 is closed.**
STEP-04 requests are now authorized and in progress under their
[live record](native-inference-step-04.md); STEP-05 has not started. STEP-07
documentation has begun, as approved, without closing STEP-06.

### Completion decisions

1. **S3-D01:** The native provider-free worker control hangs in the legacy-VM
   cold transition. The parent waits for startup; the child waits for that
   parent's execution boundary. The exact stack and an independently failing
   provider-free executable are retained. Adrian approved the
   [numbered repair](native-inference-worker-transition-proposal.md) on
   2026-09-14. The implemented candidate passes 14 focused Debug checks and the
   original native worker control. Its first Release legacy-call cost is
   +2.85% mean paired elapsed time. Adrian accepted this per-legacy-call cost;
   the remaining native model/package checks and all 2,314 non-measurement
   Debug CTests now pass. Matching sanitizer proof remains held and open.
2. **SAN-009:** A first-party feature-probe `dlclose`/reload cycle re-registers
   Apple-ASan globals. The permanent probe-cycle test reproduces the original
   ODR failure when that old operation is restored and passes with process-lifetime
   probe leases. The original four-tool trigger also passes after repair. See
   [the canonical worklist](../SANITIZER-WORKLIST.md#san-009--cpu-backend-probe-unloadreload-re-registers-apple-asan-globals).
   Approved handoff, 2026-09-14: **STEP-06 native-inference release QA, owned by Codex under
   Adrian's direction**, covering full Apple ASan and supported Linux ASan/LSan
   with the provider enabled and real model controls. Adrian explicitly approved STEP-03 closure and continuation to STEP-04.
   The handoff does not close SAN-009 or authorize any sanitizer suppression.
   Include the new S3-D01 regressions and expanded native matrix, its isolated
   sanitizer scheduling measurement, and required broad Apple/Linux gates.

The VM decision boundary is [AGENTS.md](../../AGENTS.md)'s approval rule for
architectural shifts. Interpreting the proposed quiescence/nested-callback change
as such a shift is the agent's judgement, not a claim that ordinary provider
edits need renewed approval. The SAN-009 handoff is an explicit repository rule.

### Takeover and reproducibility

**Current user instruction, 2026-09-14:** diagnose/fix SAN-009 while Adrian
reviews S3-D01, but do not rerun the sanitizer yet. Follow-up source inspection
confirmed that the reproduced probe-lifetime repair is already present, with
all 20 recorded source hashes and build configurations unchanged and all 99
retained evidence checksums valid before the documentation update. No further
code change or build/test run was needed. Keep sanitizer builds/tests on hold
until Adrian directs otherwise; the commands below are reproducibility
instructions, not authorization to resume. Adrian subsequently approved S3-D01
implementation, then approved STEP-03 closure and assigned remaining sanitizer
proof to STEP-06. The hold remains in place until that gate; do not repeat
unchanged STEP-03 normal QA. STEP-04 is authorized with the narrower glue-only
performance scope in its live record.

Read this record, the entire parent plan, STEP-01, STEP-02, the S3-D01 proposal
and the canonical sanitizer worklist before continuing. Preserve the user's
GPU-first, persistent/batch, shared-weight, Windows/light-model and STEP-07/06
requirements. The current implementation is uncommitted; baseline HEAD remains
`c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8`. Do not reset it, weaken request
acceptance or reopen already approved upstream numeric debates.

The [evidence index](../qa/native-inference-step03/README.md) retains source
hashes, commands, package controls and failures alongside passes. Reproduce the functional package matrix with
`cmake --build cmake-build-debug --target rxllama_package_measure`; use
`tools/asan-run.sh --build-dir cmake-build-debugasan --phase build --build-target
rxllama_package_measure --build-leaks off` for the local Apple sanitizer shape.
Configure the main tree first when adding a new target to a Makefiles build.
Run the registered focused provider tests plus `crexx_project_build_contract`
with explicit serial scheduling; use the maintained runner and `--leaks off`
only for Apple's documented LSan capability limit. No model is downloaded by
these checks. The model directory and archive are explicitly provisioned from
STEP-01's exact pins.
