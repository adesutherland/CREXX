# STEP-02 completion evidence

Status: S2-01 through S2-05 complete, 2026-09-14. STEP-03 has not started.
Machine-readable audit: [validation-summary.json](validation-summary.json).

This extends the approved S2-D01 v2 control without changing its numerical
limits, 100 single requests, 20 batches, four contexts, or model pins. It is
native-library acceptance evidence before implementing `rxllama`. Source and
binary identities are in `frozen-identities.json`; archive and model checks are
in `source-and-model-verification.json`. All 3,527 regular upstream source files
matched the verified commit archive. The working checkout is `develop` at
`037e7939bc29eb91b29ed41e9b1b8debdef6353d`, with uncommitted planning, guidance and
QA additions. No production provider code changes are included.

The BGE captures use the whole-prefill-era binary identified in
`whole-prefill-identities.json`; the subsequent change affects generation only.
Generation's final controls use `frozen-identities.json` and the bounded-prefill
shape described below. `control-whole-to-bounded-prefill.patch` reconstructs the
previous source by reverse application. Unchanged BGE, template and tripwire
work were not repeated solely because generation code or documentation changed.

The commands and source live in
[tests/native-inference](../../../../tests/native-inference/README.md).
`control-v2-to-completion.patch` and `cmake-v2-to-completion.patch`, reversed
against the identified completion sources, reconstruct the v2 sources. The
existing v2 reverse patch then reconstructs the original v1 workload. Historical
v1 numerical failures and noisy initial captures retain their dispositions.
`provider-v1-to-completion.patch` similarly reconstructs the original provider
acceptance source, checked against its saved SHA-256. The Level B reducer was
corrected to read unpadded `recorded-1`..`recorded-10` filenames, then verified
against both known numeric input and actual capture output.

## Controls and interpretation

- Direct BGE/Smol CPU and Metal: real tokenized input, persistent 100/20 work,
  independent contexts, numerical/token repeatability, boundaries, malformed
  GGUF/missing model/empty native batch rejection and recovery after active
  cancellation. CPU uses the public abort callback; Metal cancellation is
  observed after the admitted GPU unit completes and synchronizes. This is
  cooperative draining, not GPU kernel preemption.
- Two-model co-residency: load each model once, then allocate one/two/four
  private contexts per model, run both capabilities concurrently, retain model
  pointer identity and compare results with serial controls. RSS is sampled at
  each stage, with the 4 GiB process budget enforced; upstream logs retain
  model/context buffer allocation details. Close BGE while generation remains
  usable, then close both and run four BGE load/use/close cycles. This proves
  direct shared-model feasibility, not the future provider registry or cREXX
  worker ownership implementation. Public llama.h has no comprehensive
  per-context allocation breakdown API; process RSS and allocator logs are
  evidence, not a production admission estimator or double-counted unified VRAM.
  Smol CPU logs also show a shared 366.56 MiB CPU_REPACK model buffer alongside
  mapped weights, and 160 MiB of CPU KV buffer per generation context. File or
  payload bytes alone undercount these costs. Before the generation-only change,
  RSS with four contexts per model was 2,779,086,848 bytes CPU and 2,418,343,936
  Metal, both below 4 GiB; the final generation rerun has separate JSON/logs.
- Template oracle: upstream Jinja rendering equals the approved explicit
  rendering and token IDs for four default/explicit-system/empty/Unicode cases.
- Scratch: both pinned Stories260K and a generated random dense llama GGUF pass
  the 100/20/four-context CPU workload. The QA launcher explicitly loads the CPU
  backend before invoking the unchanged upstream generator. Dense/MoE files
  generated with seed 1234 are 4,763,872 and 7,119,040 bytes; only dense inference
  was tested. These are plumbing fixtures, not embedding or generation quality
  models. Hashes/toolchain matter because upstream seed derivation includes
  implementation-dependent `std::hash`.
- Ordinary provider tests remain red: 59 missing-procedure diagnostics in the
  persistent/negative/incremental acceptance and 12 in the foreign-worker-handle
  acceptance. These are compile failures, not green expected-failure tests.
  Packed-vector positive coverage is retained in the parent evidence; new binary
  worker transport passes the compiler, assembler, linker and both embedded and
  plain VM execution. Plain workers need a resolvable standard-library root;
  running the plain VM from `cmake-build-release/bin` supplies it. Installed
  worker library provisioning remains a STEP-03/06 package check.

## Debug, sanitizer and scheduling

The normal and ASan builds compile the pinned upstream source independently.
`build-options.txt` records each lane's flags. `CONTROL_DEBUG_GGML_OPT=ON`
uses `-O1` for ggml tensor libraries only; assertions and ASan remain enabled,
with llama.cpp and the adapter retaining ordinary Debug compiler settings.
Initial all-`-O0` tensor runs were explicitly stopped after minutes; they are
not passing evidence. The fixed workload was not reduced. No nested aggregate
was added to root CTest. The standalone tests run serially with 600-second
limits for real models/co-residency and 120 seconds for scratch/template.
The original eleven tests pass in 108.86 seconds Debug and 188.86 seconds ASan;
additional four-thread Smol passes independently in 25.95/36.21 seconds. After
bounded prefill was added, all seven affected generation/co-residency cases
were rerun; `*/bounded-prefill/` retains those results. Release and Debug followup
QA overlapped in separate trees, so their elapsed times are correctness and
conservative scheduling evidence, not performance measurements.

**S2-QA01, RSS instrumentation correction:** the initial ASan run failed the
32 MiB RSS-growth assertions without an ASan invalid-access diagnostic. In the
retained focused Smol CPU diagnostic, RSS increased by 113,377,280 bytes while
ASan live allocations increased by only 5,168 bytes; reserved heap grew by
99,876,864 bytes. This is evidence that ASan reservation/quarantine dominated
that RSS growth. The final instrumented lane retains RSS and allocator samples,
but applies the same 32 MiB allowance to live allocations, including closed
reload cycles. Ordinary Debug/Release retain the RSS allowance. No quarantine
setting, workload/numerical threshold or sanitizer suppression was changed.
This is a QA measurement correction; the failed run stays failed. It is not
an upstream leak repair or a first-party sanitizer report requiring a SAN ID.

Apple LeakSanitizer is unavailable; the maintained runner records
`detect_leaks=0` for that platform capability. CPU-side ASan does not instrument
Metal kernels. Supported-platform LSan and the broad cross-platform product
sanitizer gates remain STEP-06 work, and no release-wide sanitizer-clean claim
follows from these focused controls.

## S2-C01/C02: CPU configuration and bounded causal prefill

The two-thread Smol completion capture stopped at its first warmup because a
whole compute call exceeded 250 ms (`capture/smol-cpu`). One unchanged replay
passed at 220.40 ms maximum prefill and under 1 ms CPU abort response. A
four-thread screen passed at 148.69 ms, followed by Debug/ASan passes; however,
its formal whole-prefill capture also failed at recorded sample 3
(`capture/smol-cpu-threads4`). These failed captures retain every collected row.
The old guard threw before JSON serialization, so exact failed-unit durations
are unavailable. The screen overlapped briefly with the reducer build and is
not formal performance evidence. Thread tuning alone was not sufficient.

The approved `prepare`/`process` design requires bounded token units. The direct
control had called llama_decode on the entire logical four-prompt prefill,
which could include several physical microbatches. S2-C02 now yields at the
existing **128-token physical microbatch** boundary between API calls for
causal generation. It retains every prompt/token, four logical rows, 512 tokens
per sequence, 32 output-token limit, 100 repeated requests and 20 batches.
First-token decisions are retained before a later call replaces the logits.
The 20 four-prompt batches therefore add 20 prefill calls (140 instead of 120
warm calls), not more requests or a smaller logical batch. Noncausal BGE is
unchanged and remains a whole sequence/batch. No upstream source is modified,
and the 250 ms/1 s limits and S2-D01 numerical policy remain fixed.

The two/four-thread CPU and Metal outputs exactly match retained whole-prefill
input IDs, generated token IDs and finish reasons; see
`bounded-prefill-token-equivalence.json`. This is a control of the planned
bounded processing pattern, not a provider implementation. Actual cREXX worker
yielding/admission and cancellation during concurrency remain STEP-03/05/06.
The conservative whole-call timing guard remains even though the CPU backend
can observe aborts more frequently between graph nodes. New generation captures
use explicitly named `smol-*-bounded128` cells; old failures never become passes.

## Timing protocol

The frozen ordinary Release adapter links the previously pinned ordinary
Release engine. The existing Level B capture runner executes two warmup
processes plus ten recorded processes per BGE/Smol CPU/Metal cell, serially.
BGE uses two CPU threads per context; final Smol CPU uses four and Metal uses
two. Warm serial timing has one active context; the four-context concurrent
correctness phase is separate. These settings are explicit controls, not a
claim of an optimal automatic configuration. Each process independently loads one model and retains it across its 100
single requests, 20 batches and private-context controls. No build or other
QA invocation overlaps capture. Other desktop applications and the user's
running VM are not stopped; pre/post power, low-power, thermal and process/load
snapshots accompany each cell, so an idle dedicated host is not implied.

`capture/*/manifest.json` records commands and correctness; every raw stdout,
stderr and complete-process sample remains available. API-boundary metrics
include backend setup, model load, context creation, first request, reference
construction plus two in-process warmups, and warm single/batch totals. GPU
measurements include synchronization. The first BGE request has one row;
Smol's first request has four rows and up to 32 output tokens per row. Neither
fresh process nor first request implies flushed OS page/shader caches.
Generation includes token selection/finite-logit checks, and request timers
include native control-side input/output handling: this is the frozen QA
workload, not an upstream kernel-only score or a production wrapper verdict.

The Level B reducer emits `absolute-summary.csv` from all ten recorded rows,
with mean, range and approximate Student-t 95% mean intervals. Its retained
1..10 fixture gives mean 5.5 and interval 3.3341494103..7.6658505897; no outlier
is removed. Absolute uncertainty does not supply a twelve-pair wrapper verdict,
prove optimal CPU thread/BLAS settings, or qualify automatic device selection.

## Final capture and peak-memory results

All four final cells (`bge-cpu`, `bge-metal`, `smol-cpu-bounded128`,
`smol-metal-bounded128`) passed two warmups plus ten recorded processes: 48
passing processes in total. The [STEP-02 report](../../../../docs/planning/native-inference-step-02.md#final-local-absolute-measurements)
contains the compact measurements table; `absolute-summary.csv` retains all
means/ranges/intervals. BGE warm relative 95% half-widths are 1.9–2.1%; Smol
Metal is 0.5–1.2%. Smol CPU is 7.1% single and 5.6% batch, too uncertain for a
5% wrapper-overhead verdict from this absolute capture. The later paired
comparison is still required; no slow sample is removed or interpreted as a
product regression. Model-load timing is distinct from backend/context setup,
first complete request and warm work. All final runs pass the fixed work-unit,
cancellation, memory-growth and correctness guards.

Independent `peak-memory-*` probes use the same Level B runner around macOS
`time -l`, one process each with zero warmups. They are memory diagnostics, not
formal timing baselines. Peak RSS was **2,775,924,736 bytes CPU** and
**2,414,870,528 bytes Metal**, both below 4 GiB for the two-model/four-contexts-
per-model workload. Raw OS counters and native results are retained alongside
[peak-memory-check.json](peak-memory-check.json). macOS's separate footprint
counter is not added to RSS or treated as additional VRAM.

## Remaining product/platform qualification

All OUT/NI/AC requirements in the approved plan remain intact and all product
ACs remain open. Actual Intel Linux/Windows CPU devices, CUDA/Vulkan GPUs and
drivers, hosted exact-head gates, installed static/dynamic consumers, provider
ownership/admission/fallback and wrapper performance belong to STEP-03..06.
GitHub OS labels are not evidence of a real GPU. BGE/Smol conversion provenance
and scratch redistribution evidence remain explicit STEP-01/06 gaps. Corpus
retrieval quality is owned by the consuming crexx-rag project under NI-06.
STEP-07 may overlap STEP-06 as Adrian approved. STEP-03 has not started.
