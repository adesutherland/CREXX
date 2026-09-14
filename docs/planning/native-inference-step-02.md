# llama.rexx: STEP-02 acceptance controls

Status: complete, 2026-09-14; output ready for review. Authorized by Adrian after approval of
[STEP-01](native-inference-step-01.md). The
[authoritative plan](native-inference-backlog.md) retains all OUT, NI, AC and
STEP IDs. No production provider implementation is part of this step.

The outcome of this step is a reproducible direct llama.cpp control and
ordinary failing provider acceptance coverage, with limits fixed before the
provider candidate exists. Hardware absence remains explicit; local results
cannot close the full CPU/Metal/CUDA/Vulkan matrix.

## Active workload and thresholds: version 2

The workload and version 1 limits were recorded before direct inference or a
provider candidate. Adrian approved S2-D01 version 2 on 2026-09-14 after the
upstream investigation: preserve batching/GPU performance, retain an accuracy
tripwire fitted to the observed variation, and do not pursue upstream
determinism changes. Only the batch-layout numerical policy changes; other
budgets remain fixed. Version 1 failures remain retained failures, not
retroactive passes. These limits apply to the pinned BGE F16 control and must
not automatically widen after a failure or be advertised as retrieval accuracy.

| Control | Requirement |
| --- | --- |
| Model identity | Exact STEP-01 hashes; explicit CPU versus Metal device selection. No model substitution or automatic GPU fallback in a required-device control. |
| Embeddings | BGE F16, CLS, L2, pinned query prefix; 384 finite values per row. L2 norm differs from 1 by at most 0.00001. |
| Same backend and batch layout | Maximum absolute coordinate difference at most 0.00001 and cosine similarity at least 0.999999 against that layout's reference; includes repeats and independent contexts. |
| Same backend, different batch layout | BGE F16 maximum absolute coordinate difference at most 0.001 and cosine similarity at least 0.99998; compare single inputs and four/eight-row batches. Both limits must pass. |
| CPU versus GPU embedding | Maximum absolute coordinate difference at most 0.002 and cosine similarity at least 0.9999. This is numeric parity, not corpus retrieval qualification. |
| Input boundaries | Empty/ASCII/Unicode, query versus document, exact 512-token input and 513-token rejection; prefix/special tokens count. No truncation. |
| Generation | SmolLM2 Q8, explicit context 512 per sequence, max 32 output tokens, greedy control; fixed template and tokenizer IDs. Repeat/independent-context control on the same backend must produce identical token IDs. CPU/GPU exact generated text is not required. |
| Work sizes | Embedding rows 1/4/8 and representative token lengths near 32/128/384; generation independent prompts in rows 1/4 with fixed limits. Keep actual token counts in evidence. |
| Persistence | Per capability: 100 repeated single requests and 20 batches, with weights loaded once per model owner. Two explicit warm-up requests precede recorded work. |
| Concurrency | One/two/four contexts sharing the same model pointer, independent input/output and sampler state; compare each result to its serial reference. Later provider tests must also prove shared allocation identity and VM ownership. |
| Memory | Explicit 4 GiB total admitted inference budget for the initial two-model example; no unbounded queue. After warm-up, 100 requests/20 batches must not add more than 32 MiB retained process memory above the warmed control, or show a persistent upward trend. Normal Debug/Release use RSS; the instrumented lane uses live allocator bytes with RSS retained as diagnostic evidence (S2-QA01 below). Unified RAM/VRAM is not counted twice. |
| Wrapper memory overhead | Candidate peak memory at most direct matched control +64 MiB process overhead +16 MiB per active session. Shared-weight duplication is a separate failure even if this allowance is met. |
| Cancellation | Observe cancellation at the next supported boundary; target at most 250 ms for warm processing on the M5 at the fixed work sizes, at most 1 s for load cancellation/drain. Retain observed longest indivisible CPU/GPU unit; failure does not authorize silent work-size/context reduction. Other hardware needs declared limits before candidate assessment. |
| Wrapper performance | At least 95% of matched direct-control warm throughput; no more than 5% warm latency overhead. Startup overhead at most 10% plus 25 ms, measured separately from weight loading and warm-up. |
| Auto selection | Within 5% of the fastest qualified explicit configuration that meets identical budgets; use an inconclusive result when noise prevents selection. Device count alone is not a performance rule. |

These are provider-specific controls, not a representative language portfolio
or RexxCPS score. Existing portfolio regression guards still apply to any later
core change. Formal absolute timing needs two warmups and ten recorded serial
samples; a later before/after verdict needs twelve balanced/interleaved paired
rounds. Capture AC power, low-power setting, host load and thermal state before
and after; keep raw values and distinguish upstream compute timing from whole
process elapsed time. Do not use ASan timing as the Release baseline.

## Checkable STEP-02 exit conditions and sequence

1. **S2-01 (AC-02/03/10/11):** reproduce the pinned source build, record source
   archive/build/toolchain identity, discover actual devices and load both real
   models on CPU and local Metal. Record missing hardware cells explicitly.
2. **S2-02 (AC-04/05/06/07/08/09):** retain a direct-library correctness control
   with persistent repeated/batch input, tokenizer/preparation checks, shared
   private contexts, failure/cancellation and teardown controls. Validate a tiny
   generation fixture. Each checked item has a command, output and source hash.
3. **S2-03 (AC-12):** use correctness-gated ordinary Release upstream/direct
   controls and Level B orchestration to retain raw timing and memory evidence.
   Freeze this workload before provider implementation; do not silently replace
   end-to-end requests with random-token compute measurements.
4. **S2-04 (AC-01/03/04/05/06/09/11/13):** establish ordinary provider acceptance
   tests with positive controls; demonstrate their expected pre-implementation
   failure. Keep them outside broad aggregate registration until Debug and
   maintained sanitizer scheduling/timeouts have been measured in isolation.
5. **S2-05 (all served ACs):** retain the evidence/limitations, update this record
   and the authoritative plan, and identify Step 3 readiness without claiming
   initial-delivery qualification. No missing GPU cell is waived by this step.

## Evidence and availability

- Local Apple M5 CPU/Metal: the pinned Release build and both real models have
  executed. SmolLM2 passes 100 repeated requests, 20 four-row batches and four
  contexts sharing the same model. BGE executes the equivalent embedding
  workload, 32/128/384-token rows, document/query 512/513 boundaries, memory and
  load-cancellation controls. Its original version 1 single-versus-batch
  failures are retained; fresh CPU and Metal runs pass the approved version 2
  tripwire, including single/four/eight-row layouts. No BGE product or
  automatic-selection qualification is claimed.
- Stories260K and generated scratch: both pass repeated/batch/shared-context
  CPU generation. A QA launcher explicitly loads the CPU backend before invoking
  the unchanged upstream generator, resolving its original dynamic-backend save
  setup failure. Seed 1234 generated dense/MoE fixtures of 4,763,872/7,119,040
  bytes; only dense inference was tested. The generated weights test plumbing,
  not useful language or embedding semantics. Hashes and toolchain are retained.
- cREXX: ordinary persistent embedding/generation acceptance is in
  [provider_acceptance.crexx](../../tests/native-inference/provider_acceptance.crexx).
  It includes malformed admission, failed model identity/load, incremental
  generation and cancellation. A separate ordinary worker test requires copied
  native handles to be rejected in foreign VMs. Both fail solely on missing
  provider procedures (59 and 12 diagnostics). Existing packed-vector and new
  binary worker-transport positive controls pass compilation, assembly, linking
  and VM execution. No skip or `WILL_FAIL` converts absence into a pass.
- Direct control source, build/reproduction instructions and exact workload
  limits are in [tests/native-inference](../../tests/native-inference/README.md).
  Retained outputs, logs and identities are in the
  [evidence bundle](../../performance/evidence/2026-09-14-native-inference-step02/README.md).
- The historical SmolLM2 timing uses the existing Level B serial capture runner, two warmups
  and ten recorded samples per backend. CPU process time ranged from 23.16 to
  39.88 seconds, with host load rising from 2.88 to 9.53; Metal ranged from
  18.79 to 27.20 seconds. All 24 samples passed correctness, but these are not
  qualified comparative performance
  baselines. Retain all samples; do not remove slow rows as outliers. A later
  process snapshot also showed an active VM. Host contention is a plausible
  influence, not an established explanation of every timing difference. These
  initial samples are not superseded into passes by the completion capture.
- Intel Linux/Windows: host/checkouts requested; CPU/GPU/driver inventory pending.
- GitHub runners: existing OS labels are recorded in STEP-01; actual GPU
  availability is unproven. No extra hosted GPU resource has been started.
- All product acceptance criteria remain open. STEP-07 may overlap STEP-06 as
  approved; that scheduling permission does not weaken these controls.

## Completion controls and S2-QA01

The [completion evidence](../../performance/evidence/2026-09-14-native-inference-step02/completion/README.md)
extends the direct control with active CPU abort/Metal drain cancellation during
embedding, prefill and decode, recovery, missing/bad-magic/truncated GGUF and
empty native batch rejection. Two real models remain resident together with
one/two/four private contexts each, checked serially and concurrently, followed
by independent close and four reload cycles. Model-pointer identity and
allocation/RSS evidence establish local direct-library sharing feasibility;
they do not implement provider registry, VM ownership or memory admission.
An independent upstream Jinja oracle validates the pinned Smol template and
token IDs. No public comprehensive context-allocation API was found in the
pinned llama.h; retain upstream buffer logs and process measurements rather
than inventing precise per-owner GPU accounting.

Normal Debug and maintained-runner Apple ASan each pass eleven isolated tests,
plus the separately added four-thread Smol CPU control (25.95/36.21 seconds).
The full fixed workload takes 108.86 seconds in Debug and 188.86 seconds in
ASan. Tests remain outside the root pool and run serially, with measured
600-second real-model and 120-second scratch/template limits. Tensor libraries
use `-O1` in these Debug lanes while assertions and instrumentation remain on;
llama.cpp and the QA adapter retain Debug settings. Initial all-`-O0` tensor
runs were stopped, with their dispositions retained. No workload was reduced.

**S2-QA01:** ASan initially failed the normal RSS-growth metric. A focused
diagnostic recorded 113,377,280 bytes of RSS growth but only 5,168 bytes of live
allocation growth, alongside 99,876,864 bytes of additional reserved heap. The
instrumented lane now applies the unchanged 32 MiB allowance to live allocations
and keeps RSS, heap and free-pool samples. Normal Debug/Release still enforce
the RSS allowance. The earlier assertion failures remain recorded; no numerical
limit, work size, quarantine setting or sanitizer suppression changed. This is
a measurement correction supported by allocator evidence, not an upstream leak
fix. No ASan invalid-access diagnostic was observed. Apple LSan is unavailable,
Metal kernels are not ASan-instrumented, and supported-platform leak/full-product
qualification remains STEP-06. These focused passes do not certify a release.

The worker positive control also establishes a packaging check for STEP-03/06:
plain worker VMs need their own resolvable standard-library root. Embedded
execution passes directly; plain execution passes from the Release `bin`
directory. A raw-tree parent `-l` argument alone did not supply the worker root.
No core runtime change or unproven defect disposition is part of STEP-02.

**S2-C01, explicit CPU configuration:** the first completion Smol CPU capture
with two threads failed the conservative 250 ms whole-call guard. One unchanged
replay passed with a 220.40 ms longest prefill and less than 1 ms abort response.
The pinned CPU backend checks cancellation after graph nodes, so a whole CPU
decode call is interruptible; nevertheless the existing conservative guard is
retained. A separate four-thread control passed with a 148.69 ms longest
prefill, then passed focused Debug/ASan. Use four threads for the new Smol CPU
absolute capture, retaining the two-thread failure/replay. That whole-prefill
four-thread capture also failed at recorded sample 3; thread tuning alone was
not sufficient. No model, numerical
limit, 100/20 request count, batch/context size or deadline changes. This is
explicit configuration evidence, not a measured optimum or an automatic-choice
verdict. The warm timing phase has one active context; bounded processing under
concurrent provider workers remains part of STEP-03/05/06.

**S2-C02, bounded causal prefill:** exercise the already approved bounded
`prepare`/`process` pattern at the existing 128-token physical microbatch
boundary. Previously, one native call admitted the entire logical four-prompt
prefill, including multiple physical units. The direct control now returns
between those units and retains first-token decisions before later calls replace
the logits. Every logical prompt/token, batch row, context/output limit,
100/20 request count and deadline is preserved; no upstream source is changed.
The final workload has 140 warm prefill calls instead of 120 because each of
the twenty four-prompt batches uses two calls. Noncausal BGE remains unsplit.
CPU two/four-thread and Metal input/output token IDs and finish reasons exactly
match retained whole-prefill references. All seven affected generation and
co-residency controls pass again in Release, Debug and maintained-runner ASan
(78.99, 120.26 and 137.13 seconds). The unchanged five BGE/template/CLI/tripwire
controls retain their earlier passes. Final generation captures are explicitly
labelled `bounded128`; old whole-prefill deadline failures remain failures.
This establishes a direct control of the planned processing pattern; actual
cREXX worker yielding and concurrent admission are still provider work.

## Final local absolute measurements

Each cell passed two warmup and ten recorded complete-control processes. Values
below are means in milliseconds from the Level B reducer; all raw samples and
95% mean intervals remain in the [completion evidence](../../performance/evidence/2026-09-14-native-inference-step02/completion/README.md).
BGE batches have eight rows; generation batches have four. These are direct
native controls, not plugin overhead or optimal-hardware verdicts.

| Model / device (CPU threads per context) | Model load | First complete request | 100 warm single requests, total | 20 warm batches, total |
| --- | ---: | ---: | ---: | ---: |
| BGE CPU (2) | 16.70 | 2.42 | 1,448.21 | 2,543.59 |
| BGE Metal (2) | 20.41 | 6.56 | 478.75 | 349.79 |
| Smol CPU, bounded128 (4) | 270.05 | 238.87 | 13,007.08 | 5,927.97 |
| Smol Metal, bounded128 (2) | 42.30 | 276.89 | 9,978.45 | 5,750.77 |

The first BGE request is one row; Smol's is a complete four-prompt request, not
one token or one processing unit. Model-load timing excludes backend/context
setup and may benefit from OS caches; those setup and preparation metrics are
retained separately. Every final sample passes the conservative 250 ms warm
native-call and cancellation guards and the 1 s load-cancellation guard.
Generation token/finish patterns can differ across CPU/Metal, so these are
bounded real-request observations, not equal-token kernel-rate comparisons.

BGE warm-total 95% relative half-widths are about 1.9–2.1%; Smol Metal is about
0.5–1.2%. Smol CPU is noisier at 7.1% single and 5.6% batch, so that absolute
capture cannot resolve the proposed 5% wrapper allowance. Retain this uncertainty
and use the planned twelve paired rounds with matched controls at the later
implementation verdict; no slowdown sample is removed. All captures ran on AC
with low-power mode off, with pre/post host snapshots. The desktop and user's
VM remained active, and no dedicated idle-host claim is made.

Independent one-process peak-memory probes, orchestrated by the same Level B
runner around macOS `time -l`, recorded **2,775,924,736 bytes CPU** and
**2,414,870,528 bytes Metal** for both models with up to four private contexts
per model. Both are below 4 GiB. These probes supplement the per-stage RSS and
upstream buffer records; they are not formal throughput samples. CPU repacking
and private KV/compute buffers must be included in provider admission estimates.

## S2-D01: approved numerical tripwire

The first version intentionally predeclared a strict `0.00001` maximum absolute
coordinate difference and `0.999999` minimum cosine for same-backend comparisons.
Those version 1 controls remain failed in the retained historical evidence.

| Direct BGE F16 comparison | Largest observed coordinate difference | Lowest observed cosine | Version 1 result |
| --- | ---: | ---: | --- |
| CPU single versus batch | 0.000765192 | 0.999989958 | Fail |
| Metal single versus batch | 0.000399479 | 0.999998319 | Fail |

The unmodified upstream `llama-embedding` executable independently reproduces
the discrepancy (first-row absolute difference about `0.0003593` in its
CPU/BLAS-enabled configuration). Locally widening the same weights to F32
reduces the direct CPU maximum to about `0.0001345`; F32 with flash attention
disabled reduces it to about `0.0000287`, still above the limit. These are
diagnostic artifacts, not replacement reference models. The evidence is
consistent with differences between numerical compute paths; it does not prove
one exact kernel-level cause or establish corpus retrieval quality.

Upstream review on 2026-09-14 supports treating batch-layout invariance as a
separate requirement from repeatability. In
[llama.cpp PR #16016](https://github.com/ggml-org/llama.cpp/pull/16016), a
maintainer explicitly declines to maintain bit-identical results across batch
sizes. That CUDA deterministic-mode proposal remains a draft and is absent
from the pinned source; it is not an available cross-platform solution.
The pinned public API exposes batch sizes and flash-attention selection, but
no general batch-invariance switch. Its
[attention graph](https://github.com/ggml-org/llama.cpp/blob/5266f24da75dc449bd56cbed7addb9c8e4a6a73e/src/llama-graph.cpp#L2568)
also casts F32 K/V intermediates to F16 on the flash-attention path for
non-cached attention, so F32 weights alone do not force every intermediate to
F32. The precision diagnostics above reduce the discrepancy without meeting
version 1. Their memory/performance trade-offs have not been qualified.
Always processing one input at a time could preserve the tested single-input
layout, but would sacrifice computational batching and would not guarantee
identical results across devices. No such restriction or upstream fork is
approved. The original numerical limits were project control thresholds,
not a documented llama.cpp guarantee.

**Version 2 approved by Adrian, 2026-09-14:** retain the original strict limit for
identical inputs and identical batch layout, including repeated calls and
private-context comparisons. Give F16 single-versus-batch layout comparisons a
separate maximum absolute difference of `0.001` and minimum cosine of `0.99998`.
Keep CPU/GPU comparison limits at `0.002` and `0.9999`. Establish
separate single and batch references and rerun under the named policy, retaining
version 1 failures. The control now records limits, comparison counts, maximum
coordinate drift and minimum cosine for each comparison class. It rejects
nonfinite, zero-norm and wrong-dimension vectors independently; deliberate
coordinate and distributed-direction corruption test the two numerical guards.
This preserves meaningful repeatability
checks while explicitly accounting for the observed layout-dependent numeric
variation. It is not permission to make unrelated failures green.

**Version 2 validation:** CPU and Metal each pass 100 repeated single requests,
20 eight-row batches and four private contexts, plus four-row layout controls.
Same-layout comparisons have zero observed coordinate drift. Maximum
single/four/eight-layout coordinate drift is `0.000765192` on CPU and
`0.000399479` on Metal. Across CPU/Metal at matching layouts the maximum is
`0.000727215`, with minimum cosine `0.999989540` for four-row batches;
the eight-row minimum cosine is `0.999989430`. All pass their respective fixed
version 2 limits. The tripwire corruption controls pass separately.
See [version 2 evidence](../../performance/evidence/2026-09-14-native-inference-step02/v2/README.md).

### Index compatibility and retrieval boundary

Changing CPU/GPU execution or batch size does not by itself change the intended
embedding space or invalidate stored vectors. With identical model weights,
tokenization, query/document preparation, pooling, dimensions and normalization,
the measured variations are small numerical perturbations. For normalized
vectors, cosine ranking uses their dot product; see the
[Faiss metric reference](https://github.com/facebookresearch/faiss/wiki/MetricType-and-distances).
Well-separated scores are expected to retain their order. Nearly tied records
can swap order or cross a top-k/score cutoff; large collections can contain many
such ties, and approximate search can add further candidate variation. An index
is not corrupt merely because a boundary result changes. There is no blanket
guarantee of identical retrieved IDs, and vector cosine near one is not a
percentage retrieval-accuracy measurement.

Preserve model/preparation identity and backend/build provenance, permit only
qualified numerical compatibility, and use fixed-corpus query/top-k checks in
the consuming RAG application. A hardware or batch switch alone does not call
for rebuilding the index; a different model or preparation contract requires
explicit compatibility/migration work even when dimensions match. BGE query
instructions and unprefixed passages must remain as pinned in STEP-01; see its
[model card](https://huggingface.co/BAAI/bge-small-en-v1.5).
Corpus retrieval qualification remains with crexx-rag under NI-06; a small
provider semantic/ranking control must not be presented as that qualification.

The retained illustrative control compares all 36 combinations of CPU/Metal
single/four/eight-row query and document vectors, using the existing one
prefixed query and seven document rows. Exact normalized-dot-product search
returns the same complete ranking in all 36 cases; maximum score change is
`0.000733417` against CPU batch-eight documents plus a CPU single query.
This includes a fixed index queried with different execution configurations.
The collection is deliberately tiny, with well-separated relevant records; it
does not measure corpus recall, near-tie stability or an approximate index.

## Current exit status and takeover

Completion sequence authorized by Adrian on 2026-09-14:

1. Complete direct failure/cancellation/recovery and one/two/four-context,
   simultaneous-model memory controls; retain actual GPU drain boundaries.
2. Establish template/token golden controls and broaden ordinary red provider
   coverage using the approved API, without implementing the provider.
3. Run focused normal Debug and maintained-runner Apple ASan controls; keep
   real-model tests outside the root CTest pool. Record measured scheduling.
4. Freeze the direct workload, capture separate initialization/first/warm
   measurements through the existing Level B runner, and retain host state and
   all samples. No product performance verdict is implied.
5. Audit S2-01 through S2-05 and record Step 3 readiness. Missing external
   hardware and product qualification remain assigned to their approved later
   steps; do not claim those ACs complete.

- **S2-01 complete:** exact source/archive/toolchain/model identities, local
  CPU/Metal device execution and the explicit unavailable-hardware inventory
  are retained. Conversion provenance and other platform qualification remain
  open under the approved later gates.
- **S2-02 complete:** direct persistence, batches, token/template oracles,
  boundaries, shared private contexts, simultaneous two-model residency,
  cancellation/recovery, malformed loads, teardown/reload and both scratch
  forms pass. BGE uses S2-D01 v2; generation now exercises bounded causal
  prefill and retains exact token equivalence. No provider is implemented.
- **S2-03 complete:** four final CPU/Metal cells retain two warmups and ten
  passing recorded samples each, separate startup/first/warm measurements,
  host snapshots, Level B summaries and peak-memory probes. Whole-prefill
  failures and Smol CPU timing uncertainty remain explicit. This freezes direct
  controls and thresholds; it does not pass product AC-12 or a wrapper verdict.
- **S2-04 complete:** ordinary red provider/foreign-worker acceptance, packed
  vector and worker transport positives, and all twelve distinct isolated QA
  cases are covered in normal Debug and Apple ASan. Seven generation-affected
  cases were rerun after bounded prefill; unchanged tests retain prior evidence.
  No intentionally red test or nested aggregate entered the broad product pool.
- **S2-05 complete:** source reconstruction, binary/model identities, evidence
  checksums, limitations and this report are retained; the authoritative plan
  and roadmap mark STEP-02 complete. No OUT/NI/AC/STEP scope was removed.
  **STEP-03 is ready to begin and has not started.** All product ACs remain open.

For takeover, read the full approved backlog and STEP-01 contract before product
edits. Retain GPU support from the first implementation slice, the existing
worker/VM ownership boundary, trusted package discovery, bounded causal prefill,
whole noncausal embedding batches, and unchanged numerical/budget guards. Use
native allocation evidence rather than GGUF file size for admission. Qualify
missing platforms, conversion/redistribution provenance and installed consumers
under the named later gates; STEP-07 overlap does not close those gates.
