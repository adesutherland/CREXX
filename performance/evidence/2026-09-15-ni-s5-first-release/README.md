# NI-S5 first Release verdict — 15 September 2026

**Original submitted verdict; subsequently approved below.** The fixed generation comparison does not reproduce the
material positive Metal overhead observed in STEP-04 embeddings. All four mean
paired differences are below the existing +5% diagnostic tripwire. CPU and the
four-row Metal interval include zero; this is indicative integration evidence,
not proof of equivalence or a general cREXX speed advantage. Recommend accepting
this first verdict and continuing STEP-05's remaining functional/delivery work.
No production tuning or speculative upstream repair was made.

| Fixed workload | Direct median per request | cREXX median per request | Mean paired elapsed difference (95% interval) | Paired median difference |
| --- | ---: | ---: | ---: | ---: |
| CPU, one row | 92.846 ms | 90.181 ms | -0.00% (-3.72% to +3.71%) | -0.24% |
| CPU, four rows | 224.518 ms | 233.340 ms | +2.66% (-1.55% to +6.87%) | +3.88% |
| Metal, one row | 66.913 ms | 55.621 ms | -16.82% (-22.37% to -11.26%) | -15.72% |
| Metal, four rows | 185.344 ms | 178.661 ms | -2.52% (-8.08% to +3.04%) | -3.30% |

Positive differences mean cREXX took longer. Times divide each median ten-request
warm total by ten; they are not individual-request latency distributions. Mean
paired percentages are not ratios of separate medians. No sample was removed.
The favorable one-row Metal observation does not establish its cause or a general
performance improvement. The conditional instruction to find the cause of a
recurring material **slowdown** is not triggered by this capture. The historical
embedding observation remains unchanged and unexplained.

## Fixed work and correctness

- Ordinary Release, `CREXX_VM_PROFILING=OFF`, optional glue probes explicitly
  disabled in every process. Apple M5, 10 logical CPUs, 24 GiB, Darwin 25.6 ARM64;
  compiler-selected `rxvm` is `rxbvm`. No sanitizer was run.
- Pinned llama.cpp `5266f24da75dc449bd56cbed7addb9c8e4a6a73e`; SmolLM2 360M
  Instruct Q8_0 SHA256
  `48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201`.
- Same explicit chat template, prompt `Name one animal.`, greedy sampling,
  32-token output cap, 512 tokens per sequence / 4096 total, eight sequence
  slots, batch 512 / physical batch 128, CPU four threads / Metal two threads,
  identical context offload and output-slot settings. Metal requires real GPU
  offload; CPU requests no offload. One or four ordered prompt rows.
- Each process loads/prepares once, records one first request, then times ten
  independent requests reusing its private context and model weights. Both paths
  clear request KV state and synchronize completed compute. The cREXX path uses
  the public low-level API, bounded process calls and incremental UTF-8 reads;
  the independent direct-library control gathers the same complete output.
- Every row produced the same six sampled tokens' text, `One animal is a cat.`,
  and EOS. `verification.json` checks exact text bytes, aggregate token counts
  and every row's finish reason across **all 104 processes**. Counts are six for
  one row and 24 for four. Native correctness separately compares exact token
  IDs against an independent direct control on each backend, including different
  prompts, output limits, work budgets, cancellation and recovery.
- Minimum normal checks retained under `minimum-correctness/`: the ordinary
  preimplementation missing-generation and embedded-NUL failures, passing host
  string-service controls, native CPU/Metal controls, and all 16 typed/low-level
  opt/noopt consumer executions through rxc, rxas, rxlink and both VMs. These
  implementation checks preceded the Release freeze; the final prefix helper's
  split/invalid-scalar controls are in the final native Metal run. This is not
  the broad STEP-05 regression or STEP-06 sanitizer gate.

## Capture and limits

Capture ran 09:12:46–09:17:27 UTC. The unchanged Level B matrix runner performed
one warmup and twelve serial balanced paired rounds per case: eight warmups and
96 recorded samples. `identity.json` records baseline HEAD
`f9f87a8e8671` (full SHA in that file), 24 source inputs and 58 artifacts/model
hashes, verified unchanged after capture. Work is uncommitted on `develop`.

`PREPARE_US` and `FIRST_US` are retained separately in `timing/outputs.csv`.
Preparation is not an equivalent bare-engine comparison: the provider validates
its package/model identity and asynchronously loads the model; the direct control
loads directly. OS caches were not purged, and process/plugin startup before main
is outside this timer. No matched cold-start performance claim is made.

The host was not quiescent. Pre/post snapshots report about 34%/70% aggregate CPU
idle and no recorded thermal/performance warning; low-power mode is off. AC power
was confirmed afterward. The retained recent power-history excerpt records AC
before capture with no intervening source-change record; the immediate pre-run
current-source query was omitted. These observations do not certify continuous
GPU/CPU idleness. Do not rerun automatically or generalize from favorable samples.

Reproduce with the absolute argv in `manifest.txt`:

```text
/usr/bin/env -u CREXX_LLAMA_GLUE_PROBES cmake-build-release/bin/rxvm \
  cmake-build-release/tests/performance/ni-s4-matrix-linked -a \
  --manifest ABS_MANIFEST --output-dir FRESH_OUTPUT \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/rxvm \
  cmake-build-release/tests/performance/ni-s4-summary-linked -a \
  ABS_PAIRED_SUMMARY ABS_SAMPLES
```

The retained S4 runner/reducer binaries and their source identities are reused;
the reducer is generic over workload names despite its older diagnostic label.
It uses twelve paired percentage differences and a t interval with eleven degrees
of freedom. Raw samples, all outputs, capture metadata and `paired-summary.csv`
are retained. The generic capture's rate-oriented summary/ratio/geomean reports
were removed because their higher-is-better convention is wrong for duration.
There is no portfolio aggregate or model/backend quality/tuning programme here.

## Required next gate

[performance/AGENTS.md](../../AGENTS.md) requires reporting this first Release
verdict and stopping for Adrian's direction before broad completion work.
[STEP-05](../../../docs/planning/native-inference-step-05.md) keeps the remaining
100-single/20-batch, shared-worker/co-resident memory and cancellation,
installed/native examples, documentation and ordinary regression criteria open.
SAN-009 remains open and release blocking, owned by Codex under Adrian, with
sanitizer/platform qualification held for STEP-06. STEP-05 is not closed.

## Subsequent disposition

Adrian accepted this first Release verdict on 15 September 2026 and authorized
S5-05/06. The review-pending wording above describes the original submitted
verdict. No timing panel was rerun and no inference production logic was changed
during [ordinary closeout](../../../docs/qa/native-inference-step05/README.md).
