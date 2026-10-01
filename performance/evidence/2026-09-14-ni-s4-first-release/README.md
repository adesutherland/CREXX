# NI-S4 first Release integration verdict — 14 September 2026

Decision pending. Production is frozen and provisional. The approved
[STEP-04](../../../docs/planning/native-inference-step-04.md) asks for indicative
figures and cREXX/llama integration overhead, not a model/backend benchmark.
No model-quality study, upstream tuning, general portfolio or sanitizer work ran.

## Result and decision

CPU differences are within observed variation. Both Metal cases activate the
5% mean paired overhead tripwire (**NI-S4-P01**). The eight-row mean difference
is adverse in this panel; substantial variation prevents attributing it solely
to the glue. No samples were removed. This is not a GPU-performance pass.

| Fixed workload | Direct control median per request | cREXX median per request | Mean paired elapsed difference (95% interval) | Paired median difference |
| --- | ---: | ---: | ---: | ---: |
| CPU, one row | 1.577 ms | 1.602 ms | +0.26% (-2.28% to +2.81%) | +0.86% |
| CPU, eight rows | 12.242 ms | 12.125 ms | -2.96% (-8.52% to +2.61%) | -0.91% |
| Metal, one row | 3.332 ms | 3.509 ms | +18.90% (-13.60% to +51.41%) | +6.36% |
| Metal, eight rows | 3.957 ms | 4.231 ms | +21.53% (+4.41% to +38.65%) | +4.59% |

Times divide the median 30-request total by 30; they are not individual-request
latency distributions. Means/intervals use paired percentage differences, so
need not equal the ratio of the separately reported medians. Lower is better.
The candidate Metal one-row totals span 93.4–211.9 ms; eight-row totals span
116.6–195.6 ms. The corresponding direct spans are 95.2–163.9 and 111.8–128.9 ms.
This desktop had WindowServer, a VM, Drive and app activity throughout. AC power,
low-power mode off and no recorded thermal warning are retained in host logs.
Their contribution to the variation is plausible, not proved.

For a concrete preparation example, the first recorded cREXX runs spent
253–277 ms in explicit runtime/model/session preparation. Their first prepared
requests were 1.7/12.2 ms on CPU and 18.2/8.4 ms on Metal (one/eight rows).
These are examples, not preparation medians; OS file caches were not purged.
Preparation includes hash verification, model loading, context allocation and
warm-up; plugin/process startup before `main` is outside that timer. Process
elapsed time in the capture includes all phases and must not be compared with
warm-request time. Persistent sessions keep one model load across the requests.

Recommendation: accept the complete-text correction on its focused correctness
evidence, keep NI-S4-P01 open, and authorize one bounded unchanged Metal replay
under quieter host conditions before considering any glue edit. Do not modify
llama.cpp, tune model/backend parameters or broaden the workload. This replay
has not been run. Codex owns the next action under Adrian's direction. The
first-Release gate pauses facade/package/concurrency closeout and STEP-05 until
Adrian selects the disposition; no scope or GPU requirement is dropped.

## Fixed comparison and retained evidence

- Baseline HEAD `c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8`, `develop` with retained
  STEP-03 plus STEP-04/S4-D01 edits. `source-identity.json` records 40 changed
  source/build/test files; every hash was unchanged after capture. No pre-request
  implementation exists as a valid product baseline; the new direct control is
  the approved fixed comparator. The older STEP-02 broad timing matrix is outside
  the narrowed performance scope and was not rerun.
- Ordinary Ninja Release, Apple Clang, `CREXX_VM_PROFILING=OFF`, CPU/Metal on
  Apple M5, 10 logical CPUs, 24 GiB, Darwin 25.6 arm64. `rxvm` resolves to rxbvm;
  only the product VM is a cREXX timing cell. Source/TRACE metadata was retained,
  normal optimization used. `artifact-identity.json` hashes cache, binaries,
  runtime/backend libraries, compiled images and capture/reduction sources.
- Pinned llama.cpp `5266f24da75dc449bd56cbed7addb9c8e4a6a73e` and BGE small EN
  v1.5 F16 GGUF SHA256
  `f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999`.
  Fixed text `A cat sits by the window.`, document role, one or eight identical
  rows, two threads, context 4096 total/512 per sequence, max eight sequences,
  batch/ubatch 4096, CLS and L2. GPU uses full offload, CPU uses none.
- The C API reference uses the same packaged backend initialization outside the
  warm timer. Its measured path tokenizes complete text, builds one actual-size
  sequence batch, decodes/synchronizes once, normalizes and copies to an independent
  contiguous owner. The cREXX path adds VM/RXPA dispatch, checked resource/request
  ownership, admission, publication and close. Both retain weights/context. The
  comparison estimates the integration path as a whole; it is not a direct
  timing of the text-view callback alone.
- Correctness precedes capture: the ordinary CPU/Metal Release oracle passes
  complete-text, numeric/layout, boundary and lifecycle controls, following the
  [focused Debug evidence](../../../docs/qa/native-inference-s4d01/README.md).
  Capture processes require checked shape, finite output, correct completion and
  `load_count=1`; failures abort the run.
- Unchanged Level B `performance/tools/run_cross_runtime_matrix.crexx`, exact
  argv in `manifest.txt`: one process warmup plus 12 serial balanced/interleaved
  paired rounds per workload. Every process explicitly prepares, performs one
  first request outside the warm interval, then 30 measured requests.
- `timing/samples.csv` retains process elapsed separately from native `WARM_US`;
  `timing/outputs.csv` retains preparation, first request, warm totals and markers.
  `paired-summary.csv` is the duration verdict, computed by the retained Level B
  `summarize_paired.crexx`. Generic rate-oriented derived reports from the capture
  tool are omitted because their higher-is-better convention does not apply to
  this duration. No general aggregate is computed.
- Reproduce capture with `cmake-build-release/bin/rxvm
  cmake-build-release/tests/performance/ni-s4-matrix-linked -a --manifest
  ABS_MANIFEST --output-dir SCRATCH --measurement timing --warmups 1 --runs 12`.
  Sources are `tests/native-inference/embedding_overhead.crexx` and
  `embedding_reference_control.cpp`. Build logs and exact compiled-image hashes
  accompany the data; later evidence must not silently replace this first panel.

All full-plan OUT/NI/AC IDs remain in force. Typed facade/examples, actual worker
embedding requests, native/installed consumers and other remaining STEP-04
controls await disposition. STEP-06 owns sanitizer and other hardware/platform
qualification, including open SAN-009 and S3-D01 proof. No commit or publication.

## Later authorized follow-up

Adrian requested phase probes rather than immediately replaying on a quieter
host. The [separate diagnostic](../2026-09-14-ni-s4-glue-probes/README.md) retains
that work without replacing these raw samples. Normalize/convert and copy are
below 0.2% of request time; about 99% is inside decode/synchronization. His later
conditional acceptance of required conversion cost is recorded. The current
STEP-04 handoff recommends functional continuation with the GPU timing caveat;
no representation change, upstream repair or unconditional timing pass is claimed.
