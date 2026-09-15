# NI-S4-P01 glue phase attribution — 14 September 2026

Adrian requested probes after the noisy first Release panel, then asked whether
the float32-to-double conversion was necessary for crexx-rag's SQLite/sidecar
storage. He accepts the cost of conversion if required. This diagnostic answers
that attribution question; it does not replace or silently pass the retained
[first ordinary panel](../2026-09-14-ni-s4-first-release/README.md).

## Findings

The conversion is required by the existing `.packedfloat` interface, but **it does
not explain the earlier GPU difference**. This timer includes normalization,
finite checks and float32-to-double conversion together; conversion alone costs
no more than that combined measurement.

| cREXX workload | Request median | Decode/synchronization | Normalize + convert | Packed publication | Other request work, approximately |
| --- | ---: | ---: | ---: | ---: | ---: |
| CPU, one row | 1,553 µs | 1,539 µs | 0.53 µs | 0.13 µs | 14 µs |
| CPU, eight rows | 10,941 µs | 10,892 µs | 3.47 µs | 0.92 µs | 45 µs |
| Metal, one row | 3,369 µs | 3,342 µs | 1.69 µs | 0.24 µs | 25 µs |
| Metal, eight rows | 4,102 µs | 4,068 µs | 4.18 µs | 0.50 µs | 30 µs |

Each figure is the median of four observed 30-request averages, not a latency
percentile. Independently calculated phase medians do not sum exactly. Exact
per-sample partitions, including admission/tokenization/batch setup and remaining
unmeasured request overhead, are in `phase-samples.csv`; `phase-summary.csv`
retains all phase medians and ranges. No samples were removed.

- Normalize/convert plus publication is below 0.2% of request time in all four
  cases. It is not a plausible explanation for the prior roughly 20% mean GPU
  comparison. The user's conditional acceptance of required conversion is
  recorded, but not used to claim that conversion caused that difference.
- Approximately 99% of measured request time is within decode/synchronization.
  For eight-row Metal samples the cREXX averages varied from 3,706 to 4,913 µs;
  the direct control ranged from 3,603 to 4,036 µs. Most of that variation occurs
  inside the timed decode/synchronization boundary. This includes execution and
  waiting; it is not proof of an upstream defect or proof that all environmental
  or integration effects have been isolated.
- Every successful warm request makes exactly one decode and one independent
  packed publication. Counter deltas assert exactly 30 copies/decodes per run
  and 92,160 bytes for one-row runs or 737,280 bytes for eight-row runs. One
  `.packedfloat` holds a contiguous `.binary` of doubles. There are no 384/3,072
  per-coordinate RXPA publication calls and no text/JSON float serialization in
  this path. The packed-copy timer includes initialized-owner checking and native
  payload allocation/copy; rows/dimension publication is in the remainder.
- The direct control has comparable tiny conversion and copy costs. No observed
  bulk-copy/conversion bottleneck justifies a performance repair. No new model
  load, per-row decode, model tuning or upstream edit was introduced.

Recommendation: retain the current packedfloat output and proceed with STEP-04
functional closeout once Adrian accepts this disposition, retaining the GPU
comparison as indicative/unresolved timing evidence. Do not optimize conversion
or investigate model speed in response to this panel. The optional f32le storage
output is a distinct API choice, recorded in the
[read-only representation review](../../../docs/planning/native-inference-vector-representation-review.md).
SQLite already holds four-byte f32le values; the sidecar contains f32le centroids
in hex plus membership, with full vectors still in SQLite. No sibling product,
SQLite, sidecar, rxvector representation or public inference signature changed.

## Instrumentation and evidence boundary

The [numbered probe plan](../../../docs/planning/native-inference-glue-probes.md)
retains P-OUT-01–03, P-AC-01–04 and the original complete native-inference scope.

`CREXX_LLAMA_GLUE_PROBES=1` enables VM-local fixed-size aggregate counters only
when the provider was built with testing enabled. The default is off; absent
probes reject private diagnostic reads. No per-request logs, samples arrays or
new cross-worker state exist in the plugin. Borrowed counter pointers are used
only for the immediate native publication, never stored by the adapter.
Public procedure signatures, packed ownership, model/context and math are
unchanged. The new `probe_*` runtime inspection fields are private diagnostic
controls, not an endorsed public library API.

Phases are deliberately separate: input admission; submit/tokenization; output
buffer/batch setup and memory clear; llama_decode plus synchronization; finite
checks/L2/double output; packed payload allocation/copy. Request open, VM/RXPA
argument marshalling, handle collection, shape checking, close and timer overhead
remain in the end-to-end remainder. Snapshots are taken outside the timed warm
loop and deltas exclude initial preparation and the first request. Instrumented
figures include probe overhead; they are diagnostic observations, not a fresh
formal product-performance pass.

- `probe-before-implementation.log`: the permanent Level B probe consumer fails
  normally with unavailable diagnostic fields before implementation (exit 1).
- `debug-build.log`, `debug-checks.log`: normal Debug CPU/Metal numeric, complete
  text, boundary and lifecycle oracle passes with probes enabled. One/eight-row
  actual cREXX consumers check exact copy volume and count.
- `release-checks.log`: both CPU/Metal ordinary Release oracles pass with probes
  enabled. The deliberate no-environment control rejects private probe reads,
  followed by an explicit positive harness assertion of that expected behavior.
- `manifest.txt`: same model/text/device/threads/context/batch settings and
  30-request loop as the first panel; one warmup and **four** balanced/interleaved
  recorded pairs per case. This shorter diagnostic does not claim statistical
  resolution of the first panel's GPU overhead tripwire. Host activity and
  power/thermal observations are retained before and after.
- Existing Level B `run_cross_runtime_matrix.crexx` captures raw process timing
  and native `WARM_US` separately in `timing/samples.csv`; `timing/outputs.csv`
  keeps every counter. Level B `summarize_probes.crexx` checks counts/bytes and
  computes the phase report. Generic rate-oriented derived reports are omitted.
- `identity.json`: exact modified source, Release artifacts, compiled input and
  tool identities. Other runtime/backend binaries match the first panel.
  `representation-review-identity.json` identifies the read-only sibling files.

Work remains uncommitted. No sanitizer or broad suite was run. SAN-009 remains
open; STEP-06 native-inference release QA, owned by Codex under Adrian's direction,
retains all assigned sanitizer/platform work. Facade/examples, actual worker
requests, native/installed consumers and remaining STEP-04 criteria are still
open; none is removed by this performance disposition recommendation.
