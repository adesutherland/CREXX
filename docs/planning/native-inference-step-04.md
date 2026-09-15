# STEP-04 — Persistent embedding batches

Status: local STEP-04 implementation and acceptance work complete, 14 September
2026; ready for Adrian's public-contract and phase-closure review. Authorized
after STEP-03 closure; the accepted performance verdict remains unchanged.
Parent scope: [complete plan](native-inference-backlog.md), OUT-01–05,
CREXX-NI-01–07 and AC-01–14. Implementation started from
`c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8`. The 15 September checkpoint below
commits STEP-03/04 with the newly discovered broad regression gap retained.

## Vision and intended outcomes

An ordinary cREXX program prepares a BGE embedding session once, submits repeated
single inputs and real bounded batches, obtains ordered packed vectors, and
closes requests without reloading weights or leaking private state. CPU and GPU
use the same API and ownership contract. This step implements the approved
native request interface; its typed Level-G convenience facade remains part of
the public library delivery, with source documentation and runnable examples.
Generation remains STEP-05. Windows/Linux/CUDA/Vulkan and maintained sanitizer
qualification remain STEP-06, including the approved SAN-009/S3-D01 handoff.

Adrian explicitly narrowed performance work: provide indicative timings and
identify overhead or restrictions introduced by the cREXX/llama integration.
Do not undertake model-quality studies, llama.cpp performance investigations,
backend/thread/model tuning sweeps, or application capacity planning. A small
matched direct-library comparison may isolate glue overhead, using fixed pinned
weights, texts, batch layout, device, threads and context. Report cold preparation
separately from warm requests; check for unintended reloads, copies, serialization
and lost batching. Numeric integration tripwires and bounded resource/cancellation
semantics remain correctness obligations, not model-quality benchmarking.

## Numbered checkable acceptance criteria

1. [x] **S4-AC-01:** Approved `requestopen`, `addembedding`, `submit`, `process`,
   `embeddings`, `cancel` and `close` operations implement checked VM-local
   requests and inspectable building/running/complete/cancelled/failed states.
   Prove wrong kind/owner/state, parent close, one active request per session,
   cancellation and recovery. Ordinary failures must precede production edits.
2. [x] **S4-AC-02:** Query prefix, tokenizer special tokens, CLS pooling and L2
   normalization match the pinned BGE contract. Reject invalid role/encoding,
   oversized rows and aggregate row/token/byte limits without silent truncation.
   Check empty/Unicode inputs and exact 512/513-token boundaries including prefix.
3. [x] **S4-AC-03:** One/four/eight-row batches use upstream sequence batching and
   preserve row order. Never split noncausal attention for a work budget. Publish
   only a complete finite 384-dimensional packed result compatible with rxvector;
   output ownership survives request closure and failure publishes no partial batch.
4. [x] **S4-AC-04:** Repeats and CPU/Metal results satisfy the accepted STEP-02
   same-layout, different-layout and device tripwires. Independent shared-model
   sessions isolate results; 100 singles and 20 batches retain one model load,
   bounded request state and correct teardown. Keep platform/sanitizer proof open
   under STEP-06; never claim it from normal local tests.
5. [x] **S4-AC-05:** The typed public surface and persistent example compile, assemble,
   link and run. Cover supported optimized/unoptimized and applicable VM paths;
   installed/native embedding consumers complete after the first Release gate.
   Retain existing provider and generation-preparation behavior.
6. [x] **S4-AC-06:** Retain a first ordinary Release integration verdict after minimum
   focused correctness, before broad closeout or tuning. Use indicative cold/warm
   figures and a bounded matched glue-overhead comparison, with exact identities
   and raw samples. Existing 5% wrapper throughput/latency and memory allowances
   are diagnostic tripwires; investigate first-party causes, report noise honestly,
   and stop for Adrian's verdict. No general inference performance programme.

## Numbered implementation steps

1. [x] **S4-01 (AC-01–06):** Record STEP-03 closure, the named STEP-06 sanitizer owner
   and this performance clarification. Retain the pre-edit source identity and
   ordinary failing embedding controls alongside working lifecycle controls.
2. [x] **S4-02 (AC-01–04):** Implement request ownership/admission, preprocessing,
   batched compute, ordered packed publication, cancellation and cleanup through
   the existing session-affine bridge. Preserve its ABI and worker boundaries.
3. [x] **S4-03 (AC-02–06):** Add the minimum cREXX four-tool persistent consumer and
   direct reference controls, verify CPU/Metal correctness, then freeze the
   production slice for the first ordinary Release glue-overhead verdict.
4. [x] **S4-04 (AC-01–06):** Report that bounded verdict and pause for direction as
   required by the repository first-Release rule. Reuse valid retained evidence.
5. [x] **S4-05 (AC-01–05):** After acceptance, finish the typed facade, examples,
   native/package and remaining focused integration checks and reconcile all
   criteria. New nested aggregates stay explicit until STEP-06 sanitizer timing
   permits registration. Do not repeat unchanged full STEP-03 QA or run sanitizer
   builds/tests early. Full-product ACs remain open until individually verified.

## Selected implementation

Use one request resource under each prepared session, with bounded owned inputs,
token rows and packed result storage. Validate and tokenize a complete request
before changing it to running; one decode handles the admitted noncausal batch.
The caller's positive work budget is a scheduling hint for this indivisible unit,
not permission to change attention semantics or silently shrink the batch.
Retain the result until request closure and copy it once into the existing packed
numeric owner. Reuse model/context allocations across requests. A model/context
per call would violate persistence; one decode per row would discard true batching.
New eager cross-request caches or backend scheduling mechanisms are unnecessary.

## Current handoff — local STEP-04 complete; closure review

S4-01 is complete. S4-02/03 implement the accepted native request slice and
Adrian-approved S4-D01 length-aware text correction. The original NUL failure now
passes; negotiated host services and old-host/old-plugin controls pass. CPU and
Metal four-tool acceptance passes on both VMs, including owned complete input,
100 singles/20 batches and packed output lifetime. CPU/Metal direct numeric,
token/layout/boundary and cleanup controls also pass in ordinary Release.
See [S4-D01 evidence](../qa/native-inference-s4d01/README.md).

S4-04 presented the first decision gate. The
[first Release panel](../../performance/evidence/2026-09-14-ni-s4-first-release/README.md)
finds CPU overhead within variation and **NI-S4-P01**, the Metal glue-overhead
tripwire: mean paired +18.90% for one row and +21.53% for eight, with substantial
variation. The eight-row interval is +4.41% to +38.65%; do not describe GPU
performance as qualified or attribute the entire difference to glue without
further evidence. No outlier was removed. Production was frozen; its 40 recorded
changed source/build/test hashes remained unchanged through capture.

Adrian next authorized [phase probes](native-inference-glue-probes.md) and a
read-only [representation review](native-inference-vector-representation-review.md),
and conditionally accepted any required conversion cost. The
[probe results](../../performance/evidence/2026-09-14-ni-s4-glue-probes/README.md)
show normalization/conversion plus packed copying below 0.2% of request time;
about 99% is decode/synchronization. One packed copy and exact row byte volumes
are checked. Conversion therefore does not explain the original GPU difference.
The optional counters are VM-local, test-enabled only and off by default; no
public procedure/output format or upstream math changed.

Adrian then requested a replay with the machine quieter. The
[12-pair probes-disabled Release replay](../../performance/evidence/2026-09-14-ni-s4-quiet-release/README.md)
completed at 17:55 UTC on 2026-09-14 without code/build input changes. CPU mean
paired overhead is -0.52%/-0.87%; Metal remains +19.69%/+21.27% for one/eight rows,
with paired medians +8.37%/+6.96% and substantial spikes. The Metal differences
between separate request medians are about 0.241/0.235 ms. Both Metal means still
trigger NI-S4-P01; background load is not established as the cause. The replay
uses the existing probe-capable binaries with counters explicitly disabled and
the original uninstrumented workload; earlier evidence remains unchanged.

**Accepted by Adrian, 2026-09-14:** "I agree accept / approved" after reviewing
this replay. S4-04 and S4-AC-06's bounded embedding verdict are accepted. Retain
packedfloat and the measured Metal overhead/variation, with its cause unresolved;
NI-S4-P01 is dispositioned as an accepted integration observation, not a repaired
bottleneck or proof of compliance with the 5% diagnostic tripwire. Do not repeat
these timings or investigate upstream performance. S4-05 functional closeout is
now authorized. All platform and complete-product requirements remain open.

S4-05a and the native-interface portion of S4-05b now pass the
[normal local closeout checks](../qa/native-inference-step04-closeout/README.md):
request boundaries, CPU/GPU query/document parity at layouts 1/4/8, four real
workers running 20 batches each against isolated baselines, rxvector consumption,
both VMs/optimization modes and scratch-installed/relocated native consumers.
The [S4-D03](native-inference-typed-interface-proposal.md) typed API is now
implemented in C through the completed generic [RXPA native-object
surface](rxpa-native-objects.md). It preserves the presented names and contracts,
with durable diagnostics, failed-construction values, complete text, ordered
`add_all`, owned packed results and explicit closure. Installed public examples
cover twenty persistent batches and four workers sharing weights with private
sessions. Its own preimplementation failures and delivery evidence are retained
in the [typed completion bundle](../qa/native-inference-typed/README.md).
The final public-contract and phase-closure review remain Adrian's decisions;
no new API alternative, model default or language syntax has been selected.

SAN-009 remains open under STEP-06 native-inference release QA, owned by Codex
under Adrian's direction. Its repaired probe lifetime and prior provider ASan
evidence remain retained. S3-D01, S4-D01 and expanded request/native matrices
need the assigned sanitizer/platform verification there. No new sanitizer or
full Debug run, commit, push or STEP-05 work occurred.

### S4-05 closeout sequence

Adrian's continuation explicitly requires finishing this sequence and ticking
the documented acceptance criteria. The completed C RXPA dependency is not
STEP-04 closure. S4-05a–d are now complete against the unchanged S4-D03
contract as a reviewable candidate. Final public-contract/phase acceptance is
not inferred; all documented local implementation and evidence work is complete.

1. [x] **S4-05a (S4-AC-01–04):** Complete focused request row/token/byte boundaries,
   cancellation/state/recovery, CPU/Metal comparisons and actual shared-model
   worker embedding results. Retain ordinary controls before any repair.
2. [x] **S4-05b (S4-AC-05):** Exercise optimized/unoptimized four-tool consumers and
   relocated installed/native embedding and worker consumers. Reuse valid
   STEP-03 packaging controls; do not repeat its complete suite.
3. [x] **S4-05c (S4-AC-05):** Finalize the thin typed `llama` surface and persistent
   examples with RexxDoc coverage. STEP-01 explicitly reserves final public class
   spelling for review; present a concrete interface before implementing it.
4. [x] **S4-05d (all):** Reconcile each criterion and report phase status. Maintain
   the STEP-06 sanitizer/platform handoff and remaining whole-product outcomes.

### Criterion reconciliation after accepted Release verdict

The evidence column qualifies each local tick. Named later qualification is
still open in the parent plan and is not included in these STEP-04 ticks.

| Criterion | Local disposition and retained evidence |
| --- | --- |
| [x] S4-AC-01 | Native ownership/state/cancellation/recovery controls pass; typed failed constructors, diagnostics, parent/child and copied-owner close, one active request and partial-add cancellation pass in Debug and installed/native Release. [Native](../qa/native-inference-step04-closeout/README.md), [typed](../qa/native-inference-typed/README.md). |
| [x] S4-AC-02 | Native exact row/token/byte limits, query prefix, 512/513 boundaries and complete UTF-8 inputs pass. Typed full NUL-bearing embedding text and selector rejection preserve the same contract. Bridge inputs are unchanged. |
| [x] S4-AC-03 | Layouts 1/4/8, row order, one-decode noncausal batches, complete-only publication, owned packed results surviving request close and rxvector consumption pass. Typed values are independent owned copies. |
| [x] S4-AC-04 | Repeated requests (native 100 singles/20 batches), CPU/Metal tripwires and four real workers with private outputs/one shared model load pass. Typed examples perform 20 batches per session and correct teardown. STEP-06 platform/sanitizer proof remains open. |
| [x] S4-AC-05 | Typed public API and installed persistent/shared-worker examples pass 28 Debug and 42 installed/relocated-native Release consumer runs, both optimizations and applicable VMs. Generic RXPA compatibility and native delivery pass. Existing bridge/package code is unchanged; four current installed Release low-level Smol lifecycle/preparation runs pass on CPU/Metal and both VMs. |
| [x] S4-AC-06 | Bounded first/probe/replay verdict accepted by Adrian. NI-S4-P01 remains an accepted Metal observation with unresolved cause; no performance rerun or upstream investigation. |

### Task and outcome reconciliation

- [x] **S4-01:** Baseline, scope, ordinary preimplementation failures and named
  STEP-06 sanitizer handoff retained.
- [x] **S4-02:** Native admission, batched compute, ordered packed publication,
  cancellation and cleanup implemented and checked.
- [x] **S4-03:** Minimum persistent four-tool and CPU/Metal reference controls
  completed before the first Release verdict.
- [x] **S4-04:** First/probe/replay verdict presented and accepted by Adrian.
- [x] **S4-05:** Typed C surface, source contracts, installed examples, workers,
  package/native checks and all local criterion reconciliation completed.

OUT-01's typed embedding portion, OUT-02's local CPU/Metal embedding path,
OUT-03's persistent embedding lifecycle, OUT-04's local shared-weight/private
session behavior and OUT-05's local delivery proof are evidenced. The parent
OUT-01–05, CREXX-NI-01–07 and AC-01–14 retain generation, other hardware,
provenance and release qualification requirements; see their
[current disposition](native-inference-backlog.md#step-04-parent-acceptance-disposition).

The candidate also repairs two ordinary generic defects exposed by its consumers:
interface-only native factory provider discovery and non-integer external return
status in workers/C callbacks. Their minimal failures and permanent regressions
are retained in the typed evidence bundle. All 32 native-object Debug CTests,
55 focused compatibility tests, eight static-compiler executions and installed
C/C++ SDK/native controls pass. This is focused current evidence, not a new full
Debug or sanitizer qualification.

**Next action at typed capture:** Adrian's review of the completed public
contract and STEP-04 closure. The 15 September follow-up below supersedes this
next action with the newly discovered full-regression qualification failure. No implementation/test/documentation item is left midway in this local
step. STEP-05 generation has not begun; STEP-06 retains SAN-009 and the complete
platform/sanitizer gate. STEP-07 documentation/examples may overlap STEP-06 as
already approved. No scope item is removed to obtain local closure.


## 15 September baseline and broad regression follow-up

Adrian requested a committed baseline and confirmation of complete regression
coverage after the RXPA changes. The earlier 2,314-test full Debug pass predates
the latest C object/compiler/executor changes. The 87 focused Debug tests and
70 typed consumer runs establish the bounded STEP-04 evidence, not a current
full-product pass. This follow-up captures the completed implementation and
checks the ordinary full suite against that same source before baseline commit.
It does not advance generation or the held STEP-06 sanitizer/platform gate.

1. [x] **S4-B-01:** Verify the current implementation hashes match the final typed
   acceptance capture and inventory the complete baseline, excluding generated
   Python caches and retaining original outcomes/criteria/evidence.
2. [ ] **S4-B-02:** Prepare all ordinary QA artifacts with `qa-prep`, run the full
   Debug CTest selection excluding `performance-measurement`, and retain the
   exact selection, result and source identity. Report any failure explicitly;
   earlier focused passes cannot stand in for this result.
3. [x] **S4-B-03:** Commit the reviewed baseline and evidence locally, with the
   native-worker repair independently reviewable. Report commit identities and
   distinguish local regression evidence from later platform/sanitizer/release
   qualification. Do not push or repeat unchanged timing/sanitizer workloads.

Live evidence: [baseline regression record](../qa/native-inference-baseline/README.md).


**Current broad qualification result:** S4-B-02 remains open. `qa-prep` failed
with NI-S4-QA01: existing HTTP server clients report RETURNS_VOID/RETVAL_MISSING
for request/get results. The standalone current-compiler failure is retained in
the baseline record. Its cause is not yet attributed. The requested commit is a
checkpoint including this gap, not a full-regression-qualified baseline. S4's
bounded typed/native results remain recorded; overall AC-13 and release closure
remain open. Do not describe the earlier 2,314 or 87 focused passes as covering
this failed current broad preparation. No production repair has been made in
this baseline follow-up.
