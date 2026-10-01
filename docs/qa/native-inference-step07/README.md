# STEP-07 documentation, examples and QA handoff

Authority: [the parent vision, AC-01–14 and S7-AC-01–06](../../planning/native-inference-backlog.md).
Recorded 15 September 2026. Adrian approved documentation/examples and review
before full STEP-06 QA. This ledger supports that plan; it does not replace it.
Documentation preparation and focused local checks are complete. Review of the
completed deliverables is complete under S7-AC-06: Adrian approved the pack on
15 September 2026. STEP-07
is closed and STEP-06 is now authorized. No full QA, sanitizer run or performance
panel occurred during the documentation phase.

Subsequent STEP-06 QA applied Adrian's wide-backstop/serialization direction to
the examples after a two-minute shared-generation scope expired under ASan.
That correction and its matching normal/sanitizer proof are tracked as S6-QA02
in the [current QA ledger](../native-inference-step06/README.md). The evidence
below retains the documentation-phase inputs and results rather than relabelling
them as later runs.

## Reviewable deliverables

| Reader need | Current guide / executable source |
| --- | --- |
| Entry point and typed lifecycle | [llama.rexx guide](../../../lib/plugins/llama/README.md) |
| Source/package install, Windows and GPU prerequisites, offline source inputs | [Installation](../../../lib/plugins/llama/installation.md) |
| Exact downloads, hashes, storage, licences, provenance and scratch limits | [Models](../../../lib/plugins/llama/models.md) |
| Options/defaults, device selection, ownership, limits, output, errors | [Operating reference](../../../lib/plugins/llama/reference.md) |
| Complete native/VM commands and walkthrough of four persistent/shared examples | [Examples](../../../lib/plugins/llama/examples/README.md) |
| Verified/pending/unsupported matrix, accepted glue/memory observations | [Qualification status](../../../lib/plugins/llama/qualification.md) |
| Agent implementation guidance | [CREXX_LIBS](../../ai-context/CREXX_LIBS.md#rxllama-native-inference-lifecycle) |
| Source API contracts | [C RXPA typed declarations](../../../lib/plugins/llama/typed.h), [generic RXPA guide](../../books/crexx_programming_guide/rxpa.md) |

The optional install includes all six Markdown pages and four `.crexx` examples
through the `llama-docs` component. A normal full install includes that component.
The four executable examples and all C RexxDoc blocks are retained unchanged;
their existing qualified workloads are useful examples, not discarded in favor
of simpler one-shot demos. Historical STEP-01 factory proposals and STEP-02 red
controls are labelled as history where current guides link to them.

## Parent acceptance coverage map

Every row is **open for full-product acceptance**. Existing evidence is a local
contribution. Platform columns refer to the initial matrix in the parent plan:
macOS arm64 CPU/Metal, Linux x86-64 CPU/CUDA/Vulkan and Windows x64
CPU/CUDA/Vulkan, with actual available devices inventoried before execution.
Unavailability keeps a required cell open for Adrian's explicit disposition.
Codex under Adrian owns the STEP-06 remaining actions in every row.

| AC | Documented behavior / user workflow | Runnable example or control | Retained contribution | STEP-06 verification still required |
| --- | --- | --- | --- | --- |
| AC-01 | [Install/distribute](../../../lib/plugins/llama/installation.md#what-is-installed-and-what-to-distribute), [download then run locally](../../../lib/plugins/llama/models.md) | Four [examples](../../../lib/plugins/llama/examples/README.md); `typed_package_consumer.py`, `generation_package_consumer.py` | [Typed embedding delivery](../native-inference-typed/README.md), [generation delivery](../native-inference-step05/README.md) | Every target OS: full installed/dynamic/native execution, network-disabled inference with provisioned files and no server/CLI; inventory dependencies. |
| AC-02 | [Platform matrix](../../../lib/plugins/llama/qualification.md#platform-and-delivery-matrix), [GPU selection](../../../lib/plugins/llama/reference.md#hardware-selection-and-inspection) | Persistent/shared examples with `cpu` and `required-gpu`; native embedding/generation bridge controls | [STEP-03](../native-inference-step03/README.md), [STEP-04](../native-inference-step04/README.md), [STEP-05](../native-inference-step05/README.md): actual M5 CPU/Metal | Each Linux/Windows CPU and named GPU cell: hardware/driver/toolchain identity, actual offload and compute for both models. |
| AC-03 | [All hardware options and device keys](../../../lib/plugins/llama/reference.md#configuration) | `typed_configuration.crexx`, `lifecycle_acceptance.crexx`, `bridge_lifecycle.cpp`, `package_acceptance.py` | STEP-03 selection/package and typed configuration controls | Cross-backend device identity/deduplication, no GPU, missing backend/driver, required-GPU and explicit overrides, insufficient device memory, incompatible options; retain selection/fallback reasons. |
| AC-04 | [Startup and prepared lifetime](../../../lib/plugins/llama/reference.md#status-state-and-lifetime), persistent examples | `embedding_bridge.cpp`, `generation_bridge.cpp`; persistent examples' one-load/twenty-batch assertions | STEP-04/05 native 100 singles/20 batches and prepared repeated public requests | Same sustained work on target cells; inspect load count and separate cold/preparation/warm evidence. Reuse accepted matched performance controls where inputs are unchanged. |
| AC-05 | [BGE profile/roles](../../../lib/plugins/llama/models.md#model-behavior-and-identity), [packed storage](../../../lib/plugins/llama/reference.md#output-representation) | `embedding_bridge.cpp`, `embedding_text_boundary.crexx`, `typed_embeddings.crexx`, both embedding examples, `rxvector_*` CTests | STEP-02 numeric tripwires, STEP-04 packed/typed/vector controls | Per backend: order/dimensions/finite norm, CPU/batch tolerance controls and corruption positive controls; empty, malformed, embedded-NUL and oversized inputs, pooling/profile rejection; maintained sanitizer. |
| AC-06 | [Generation lifecycle/chunks](../../../lib/plugins/llama/README.md#generation), [output](../../../lib/plugins/llama/reference.md#output-representation) | `generation_bridge.cpp`, `typed_generation.crexx`, `provider_acceptance.crexx`, persistent/shared generation examples | STEP-05 independent same-backend token/text/finish parity, full UTF-8, boundaries and recovery | Remaining devices: repeated independent prompts, row order, EOS/empty output/output limits, split UTF-8, finish/error preservation and request isolation. Cross-device text equality is not required. |
| AC-07 | [Shared ownership](../../../lib/plugins/llama/reference.md#shared-weights-admission-and-scheduling) | Shared examples; native embedding/generation 1/2/4-owner controls and separate memory modes | STEP-03/04/05 M5 identities, private outputs and direct/bridge/VM memory observations | Every backend: shared/private actual RAM/VRAM, incompatible-placement separation, private mutable state, teardown/failure stress; explicit disposition for unsupported sharing. |
| AC-08 | [Budget scope, admission and scheduling](../../../lib/plugins/llama/reference.md#shared-weights-admission-and-scheduling) | `bridge_lifecycle.cpp`, `generation_bridge.cpp` with both models, `generation_memory.crexx`, native embedding controls | STEP-05 BGE/Smol co-residency below 4 GiB, bounds and no retained normal RSS growth | Target RAM/VRAM peaks, low-memory/load-failure controls, saturation and application queue bounds; simultaneous models and multiple runtimes. Reservations are not a process-wide OS cap. |
| AC-09 | [Cancellation, errors and cleanup](../../../lib/plugins/llama/reference.md#status-state-and-lifetime) | `rxllama_backend_probe_cycle`, bridge lifecycle, typed consumers, native generation cancellation; `rxpa_host_text_services` | [SAN-009 local repair](../../SANITIZER-WORKLIST.md#san-009--cpu-backend-probe-unloadreload-re-registers-apple-asan-globals), normal STEP-03–05 cleanup/output controls | Maintained sanitizer focused + full platform gates, supported Linux LSan, preparation/load/prefill/decode/batch cancel and GPU drain latency, failed-state cleanup. Close SAN-009 only by its worklist conditions. |
| AC-10 | [Exact pins and provenance gap](../../../lib/plugins/llama/models.md#licences-and-conversion-provenance), inspectable identity keys | Download verification recipes; altered-file/profile controls in lifecycle/embedding/generation tests | [Pin manifest](../../planning/native-inference-step-01-lock.json), STEP-05 identities | Resolve exact conversion ancestry or separately approve/pin a reproducible conversion; retain commands/source/tool/tokenizer/output hashes and licences. Recheck provisioning/altered artifacts per target. |
| AC-11 | [Package locations and deployment](../../../lib/plugins/llama/installation.md#what-is-installed-and-what-to-distribute) | `package_acceptance.py`, `step03_install_qualification.py`, typed/generation package consumers and SDK tests | STEP-03 package checks, STEP-04/05 clean outside-checkout installed and relocated native runs | All target OSes/CPU-only hosts, missing/wrong provider/backend/runtime files, Unicode/spaces/relocation and Windows DLL dependencies; verify no sibling checkout reliance. |
| AC-12 | [Indicative glue scope](../../../lib/plugins/llama/qualification.md#retained-indicative-evidence) | Accepted direct/bridge/public matched controls; load counters, batching and sharing checks | [Accepted S5 Release verdict](../../../performance/evidence/2026-09-15-ni-s5-first-release/README.md); accepted NI-S4-P01 remains unexplained | Preserve GPU/persistence/batching with no new unexplained integration loss; investigate a material new glue regression if reproduced. Do not replay accepted timing panels or benchmark/tune models and llama.cpp. |
| AC-13 | [Installed four-tool route](../../../lib/plugins/llama/examples/README.md#the-explicit-four-tool-installed-vm-route), [qualification boundary](../../../lib/plugins/llama/qualification.md#open-acceptance-items) | Full ordinary CTest; `rxpa_host_text_services`, `rxpa_objects_*`, `rxvector_*`; HTTP/task/native SDK and typed public controls | [Fresh 2,347-test normal Debug pass](../native-inference-step05/README.md), [QA01/QA02 regression/factory repairs](../native-inference-qa01/README.md) | Maintained sanitizers including S3-D01/S4-D01/S5-D01/native objects/typed-call boundaries, remaining platforms and exact-head hosted publication gates. Verify unchanged inputs before evidence reuse. |
| AC-14 | All six installed pages and four examples; source and agent contracts | Documentation link/pin/source checks, installed recipe smokes and prior shared example runs | This handoff plus unchanged STEP-04/05 example and source evidence | Adrian's S7 review first; then final platform/evidence/criterion reconciliation after QA, keeping every unmet criterion visible. |

## Evidence reuse and documentation checks

The baseline is STEP-05's uncommitted implementation on `develop` at
`f9f87a8e8671a5e7894fbc187dbc93ee7bcf8466`. Documentation work must preserve its
runtime, compiler, adapter and example behavior. The only intended non-Markdown
change is the CMake install rule adding the documentation component/files.
That changes a source input used by the bridge identity hash on a future
rebuild, but does not edit execution behavior. Record this distinction rather
than rewriting historical evidence to pretend it was captured on a new revision.

Focused check results and input identities are retained below.
Reuse the unchanged four examples' existing opt/noopt, both-VM, CPU/Metal and
installed/native runs as evidence for those captured revisions; unchanged example
source alone does not prove a later runtime binary. STEP-06 must compare each
record's relevant inputs and select affected rechecks where they changed. The
new native/VM recipe checks cover the current STEP-05 runtime. Check download verification
recipe on the current host. Windows/PowerShell, Linux and discrete-GPU commands
remain pending native execution. Existing result-count claims are linked above;
do not repeat the 2,347-test suite for Markdown/install-documentation changes.

### Completed focused checks

| Check | Result / retained evidence |
| --- | --- |
| Existing cache recipe | Both exact GGUF hashes verified; no replacement/download of existing files. `model-cached-verification.log`. |
| Fresh download branch | Exact pinned BGE URL downloaded 67,308,128 bytes to a disposable directory; hash passed before final rename. Smol reused its verified cache. `model-fresh-bge-download.log`. |
| Wrong existing artifact | Rejected with nonzero status and SHA-256 mismatch; original wrong bytes left unchanged. `model-bad-hash.log`. |
| Installed guides | CMake reconfiguration and actual `--component llama-docs` install passed into a disposable clone of the retained STEP-05 scratch install. All six Markdown pages and four examples installed. `configure-docs.log`, `install-docs.log`. |
| Native embedding recipe | Installed `crexx --native` build passed; `auto` selected Metal, twenty batches completed, one model load and owned f32 storage checked. `embeddings-native-build.log`, `embeddings-native-auto.log`. |
| Native generation recipe | Installed native build passed; `auto` selected Metal, twenty four-row batches completed with incremental text/finishes and one model load. `generation-native-build.log`, `generation-native-auto.log`. |
| Explicit four-tool recipe | `rxc`, `rxas`, `rxlink`, product `rxvm` route passed using installed imports; Metal and twenty embedding batches. `vm-compile.log`, `vm-assemble.log`, `vm-link.log`, `vm-auto.log`. |
| Source/doc audit | All 14 parent rows, every source-defined configuration option, pinned model URL/hash pairs and local links checked. 8,611 existing non-Markdown inputs compared; only the documentation install declarations changed. `documentation-checks.json`. |
| Evidence reuse | All 23 non-document source hashes compared with STEP-05 (excluding the separately proven documentation-only CMake delta) and all 48 tool/runtime artifact hashes match. `documentation-checks.json`. No broad regression/sanitizer/performance replay. |
| Source documentation | C typed API's 41 RexxDoc blocks / 71 selected tags and four examples' 22 blocks / 46 tags retained byte-for-byte. `documentation-checks.json`. |

`recipe-results.json` retains exact resolved commands and results;
`recipe-identities.json` retains installed guide/example and native manifest
identities at execution. `source-check.json` records the pinned Smol publisher
card's licence verification. `run_doc_checks.py` and `verify-models.sh` retain
the smoke driver/recipe; only prefix/model/work paths replace the guide's user
paths. These are explicit documentation checks, not registered CTest aggregates.
The download-mismatch negative control is recorded separately. No user-prefix
installation occurred.

The ordinary source toolchain was already built and qualified; this phase
reconfigured CMake and installed only the documentation component, then built
the documented native consumers. It did not repeat a clean toolchain build or
replace engine binaries. Native execution verified the prior packaged runtime
with the new documentation. A future exact-head build may change the bridge
identity because it hashes CMake source; final identity/package qualification
remains STEP-06. Windows/PowerShell, Linux, discrete GPUs, fresh Smol download
execution and enforced network-disabled inference remain explicit pending
qualification, not inferred from these checks.

Editorial review reconciled the public methods/options and error categories
against `typed.h`, `bridge.cpp` and `generation.h`; verified source tags and
example retention; fixed stale pre-implementation/typed-facade status in the
live control guide; and synchronized the project, documentation and agent entry
points. Public text distinguishes per-runtime reservations from process-wide
memory limits, one-device selection from multi-GPU placement, and token work
budgets from elapsed-time guarantees.

## STEP-06 command handoff — after S7 review

From the repository root, set `BUILD` to the chosen ordinary Debug build,
`RELEASE_BUILD` to the profiling-off Release build, `SOURCE` to this checkout,
`MODELS` to the verified two-model directory and `MODES` to explicit supported
modes (`cpu,required-gpu` on the M5). Use distinct fresh work directories and a
scratch `PREFIX`. Provision the exact engine archive from the install guide.
Inventory hardware before expanding MODES or assigning a GPU result.

```sh
cmake --build "$BUILD" --target qa-prep --parallel 6
cmake --build "$BUILD" --target rxllama_embedding_bridge rxllama_generation_bridge \
  llama_provider_runtime_package crexx-provider-package --parallel 4
python3 tests/native-inference/typed_toolchain.py "$BUILD" "$SOURCE" "$MODELS" "$WORK_EMBED" "$MODES"
python3 tests/native-inference/generation_toolchain.py "$BUILD" "$SOURCE" "$MODELS" "$WORK_GENERATE" "$MODES" --closeout
cmake --build "$RELEASE_BUILD" --target stage-c1-toolchain stage-product stage-optional \
  llama_provider_runtime_package crexx-provider-package --parallel 4
cmake --install "$RELEASE_BUILD" --prefix "$PREFIX"
python3 tests/native-inference/typed_package_consumer.py "$PREFIX" "$SOURCE" "$MODELS" "$WORK_PACKAGE_EMBED" "$MODES"
python3 tests/native-inference/generation_package_consumer.py "$PREFIX" "$SOURCE" "$MODELS" "$WORK_PACKAGE_GENERATE" "$MODES"
```

These are a command inventory, not instructions to rerun every unchanged case.
At the STEP-07 capture, the embedding package dispatcher used Unix executable
paths. STEP-06 has added the same `.exe` handling already present in the typed
toolchain and generation package dispatchers, retaining identical assertions.
This source adaptation is not Windows qualification. `--closeout` excludes the already-retained minimum
generation matrix. Native direct controls' full argument shapes and model hashes
are in the [STEP-05 reproduction record](../native-inference-step05/README.md#reproduction)
and [native control guide](../../../tests/native-inference/README.md).

For ordinary regressions, use retained evidence if inputs are unchanged;
otherwise prepare and run the applicable full selection:

```sh
ctest --test-dir "$BUILD" --parallel 30 --output-on-failure -LE performance-measurement
```

Choose lower parallelism on slower hosts as required by `AGENTS.md`. Maintain
isolated scheduling for nested aggregates. STEP-06 must apply existing S2-QA01
live-allocator accounting to instrumented memory controls and measure new
aggregates in Debug and the maintained sanitizer tree before broad registration.
Normal RSS allowances/workloads remain unchanged; sanitizer timing is diagnostic.

Use the [maintained runner](../../ai-context/CREXX_ASAN_TESTING.md) for sanitizer
execution. SAN-009's focused CTest is `rxllama_backend_probe_cycle`; generic
output coverage is `rxpa_host_text_services`. The same focused command must
pass normal Debug first. Representative runner form after preparation:

```sh
tools/asan-run.sh --build-dir cmake-build-debugasan --phase ctest \
  --regex '^(rxllama_backend_probe_cycle|rxpa_host_text_services)$' --test-jobs 1 --no-live-tail
tools/asan-run.sh --build-dir cmake-build-debugasan --phase full --test-jobs 8 --no-live-tail
```

These commands do not override platform leak capability or worklist requirements.
Use supported Linux ASan/LSan for leak closure and the recorded macOS capability
settings on Apple; no first-party suppression/waiver is introduced. Retain all
new findings in the canonical worklist. Exact-head hosted publication gates
remain separate; documentation review authorizes neither publication nor a
release-ready claim.

## Review disposition

| Readiness ID | Evidence / disposition |
| --- | --- |
| S7-AC-01 | Installation/models guides, installed documentation component, fresh BGE and cached two-model hash recipes; licence/provenance and pending-platform boundaries explicit. |
| S7-AC-02 | Four unchanged installed examples, retained STEP-04/05 shared-worker evidence, new native `auto` and explicit four-tool walkthrough checks. |
| S7-AC-03 | Complete configuration/lifecycle/output/error reference, unchanged C RexxDoc and examples, synchronized human/agent entry points and editorial source audit. |
| S7-AC-04 | Parent coverage table accounts for all AC-01–14, negative cases, platform cells, existing controls and remaining commands/evidence. |
| S7-AC-05 | Installed capability/qualification matrix retains accepted indicative figures, SAN-009, unresolved conversion provenance and every pending platform/gate. |
| S7-AC-06 | Approved by Adrian, 15 September 2026. Documentation/examples accepted; STEP-06 may commence. |

- S7-AC-01–05: complete for documentation readiness, with the guides, coverage
  map and focused/reused evidence above. Platform/product acceptance stays open.
- S7-AC-06: approved; all STEP-07 criteria/tasks are complete.
- STEP-06: now authorized after the documentation phase. SAN-009, conversion
  provenance, all unverified platform cells and parent AC-01–14 stay open.
