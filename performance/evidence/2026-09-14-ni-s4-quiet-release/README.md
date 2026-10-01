# NI-S4 user-requested quieter-host replay — 14 September 2026

Adrian requested this replay after the phase probes. The fixed ordinary Release
comparison completed at 17:55:00–17:55:38 UTC. CPU overhead remains within
variation; both Metal cases again exceed the 5% mean paired diagnostic tripwire.
NI-S4-P01 remains open for disposition. No implementation changed for this run.

| Fixed workload | Direct control median per request | cREXX median per request | Mean paired elapsed difference (95% interval) | Paired median difference |
| --- | ---: | ---: | ---: | ---: |
| CPU, one row | 1.526 ms | 1.527 ms | -0.52% (-1.78% to +0.74%) | -0.03% |
| CPU, eight rows | 10.998 ms | 10.924 ms | -0.87% (-2.89% to +1.15%) | +0.15% |
| Metal, one row | 3.278 ms | 3.519 ms | +19.69% (+2.62% to +36.76%) | +8.37% |
| Metal, eight rows | 3.778 ms | 4.012 ms | +21.27% (-4.26% to +46.80%) | +6.96% |

Times divide each median 30-request total by 30; these are not individual-request
latency distributions. One eight-row request processes eight embeddings. Means
and intervals use paired percentage differences, not ratios of separate medians.
Positive differences mean slower. No samples were removed. Metal paired changes
still span -4.95% to +85.36% for one row and -4.03% to +135.73% for eight rows.
The difference between medians is about 0.241/0.235 ms per Metal request.

The original panel's Metal means were +18.90%/+21.53%; this replay does not remove
that observation or establish background load as its cause. Host snapshots show
AC power, low-power mode off, no recorded thermal/performance warning and about
86% aggregate CPU idle immediately before/after capture. WindowServer and a QEMU
VM remained active. These snapshots neither certify continuous CPU/GPU idleness
nor identify the cause of individual timing spikes.

The earlier [phase probes](../2026-09-14-ni-s4-glue-probes/README.md) put combined
normalization/conversion/packed publication below 0.2% of request time, with
about 99% inside decode/synchronization. Required float-to-double conversion
therefore remains too small to explain the observed Metal difference. This does
not establish an upstream defect or exclude an interaction with the integration
path. Recommendation: retain packedfloat and make no conversion/copy or upstream
performance edit; accept the bounded indicative Metal overhead and variation
explicitly before continuing STEP-04 functional closeout. Acceptance is pending.

## Reproduction and evidence boundary

- Baseline HEAD `c2cf28a4f5c66430b4c2cc4d49b00ae720675ab8`, `develop`, with the
  existing uncommitted STEP-03/STEP-04/S4-D01 and authorized probe changes.
  `identity.json` records 41 source/build/test inputs and 34 artifacts/model
  hashes, verified before and after capture. No rebuild or repeated correctness
  suite was needed; retained focused checks apply to these inputs.
- Ordinary Ninja Release, `CREXX_VM_PROFILING=OFF`, `BUILD_TESTING=ON`, Apple M5,
  10 logical CPUs, 24 GiB, Darwin 25.6 arm64; product `rxvm` resolves to `rxbvm`.
  Current bridge/control binaries contain optional probes, unlike the first
  panel's pre-probe binaries. Every process explicitly unsets
  `CREXX_LLAMA_GLUE_PROBES`; the original uninstrumented
  `ni-s4-overhead-linked.rxbin` workload is reused. This is a probes-disabled
  replay of the same workload, not a claim that the first panel's binaries are
  byte-identical. Engine/backend, VM, workload and reducer identities match.
- Same pinned llama.cpp `5266f24da75dc449bd56cbed7addb9c8e4a6a73e`, BGE small EN
  v1.5 F16 model SHA256
  `f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999`, document text
  `A cat sits by the window.`, one/eight rows, two threads, context 4096 total /
  512 per sequence, max eight sequences, batch/ubatch 4096, CLS/L2, CPU no offload
  or Metal full offload. Each process prepares once and performs its first
  request outside the warm interval, then measures 30 requests.
- The unchanged Level B matrix runner uses one warmup and 12 serial balanced
  paired rounds per fixed case: 104 successful processes, including eight
  warmups. Capture checks completion, finite output, shape and one model load.
  `timing/outputs.csv` retains preparation and first-request timings separately
  from `WARM_US`. OS caches were not purged; process/plugin startup before main
  is outside the preparation timer. No new cold-start conclusion is drawn.
- `manifest.txt` preserves exact argv. Capture command:
  `/usr/bin/env -u CREXX_LLAMA_GLUE_PROBES cmake-build-release/bin/rxvm
  cmake-build-release/tests/performance/ni-s4-matrix-linked -a --manifest
  ABS_MANIFEST --output-dir FRESH_OUTPUT --measurement timing --warmups 1 --runs 12`.
- `paired-summary.csv` uses the unchanged first-panel Level B
  `summarize_paired.crexx` and its retained compiled image:
  `cmake-build-release/bin/rxvm
  cmake-build-release/tests/performance/ni-s4-summary-linked -a OUTPUT_SUMMARY
  INPUT_SAMPLES`. It requires 12 pairs and uses a t interval with 11 degrees of
  freedom. The generic capture tool's three rate-oriented derived reports are
  omitted because their higher-is-better convention is wrong for `WARM_US`.
  Raw samples/output and the duration summary are retained; no portfolio
  aggregate is computed. `verification.json` records the identity/data audit.

The [first panel](../2026-09-14-ni-s4-first-release/README.md) and probe evidence
remain unchanged. The [STEP-04 handoff](../../../docs/planning/native-inference-step-04.md)
retains every original outcome, acceptance criterion and remaining functional
obligation. No sanitizer/full-suite, model/backend tuning, representation change,
commit or publication occurred. SAN-009 and the assigned platform/sanitizer
qualification remain STEP-06, owned by Codex under Adrian's direction.
