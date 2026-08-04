# PERF3-12 validation

## Product and build identity

The live repository was on `codex/perf3-rxas-flow-infrastructure` at
`209206f0f0f07cd709dcdd04665489756e0b433e`.  A clean detached source at
`5fbe36049e26ee73ea0cf1720a7fc416f33d0fe2` supplied both the accepted
ordinary Release artifacts and the profiling build.  The diff between those
commits contains only PERF3-06 performance documentation/evidence.

The profiling build used CMake 4.3.2, Ninja 1.13.2, AppleClang 21.0.0,
`Release`, `-O3 -DNDEBUG`, and `CREXX_VM_PROFILING=ON`.  The accepted timing
product remained the profiling-off Release build.  Exact hashes are in
`artifacts.csv`.

During the first profile-build invocation, the command runner yielded while
Ninja remained active.  Two retries accidentally overlapped the first build
in the same temporary tree.  All three temporary-tree build processes were
stopped, after which one serial build completed.  Ninja retained a
`premature end of file; recovering` warning for `.ninja_log`; the incident log
is preserved in `logs/profile-build-incident.log`.  The final binaries have
stable hashes, report the expected product version, load both accepted images,
and produced checksum-valid complete profiles.  Because this activity uses
deterministic counts rather than profile elapsed time, no timing claim is
exposed to the build incident.

## Commands

The material command shapes were:

```sh
cmake -S /private/tmp/crexx-perf3-06.yHwy0b/src \
  -B /private/tmp/crexx-perf3-12.9cGqDj/build-profile \
  -G Ninja -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=ON

cmake --build /private/tmp/crexx-perf3-12.9cGqDj/build-profile \
  --target rxvm rxbvm rxseq --parallel 1

/private/tmp/crexx-perf3-06.yHwy0b/build-release/bin/crexx \
  performance/tools/run_evidence_bundle.crexx --nokeep --args \
  --manifest /private/tmp/crexx-perf3-12.9cGqDj/rexxcps-profile-fixed200-manifest-v1.txt \
  --release-build /private/tmp/crexx-perf3-06.yHwy0b/build-release \
  --profile-build /private/tmp/crexx-perf3-12.9cGqDj/build-profile \
  --output-dir /private/tmp/crexx-perf3-12.9cGqDj/rxvm-fixed200 \
  --vm rxvm --profile-mode counts --force

/private/tmp/crexx-perf3-06.yHwy0b/build-release/bin/crexx \
  performance/tools/run_evidence_bundle.crexx --nokeep --args \
  --manifest /private/tmp/crexx-perf3-12.9cGqDj/rexxcps-profile-fixed200-manifest-v1.txt \
  --release-build /private/tmp/crexx-perf3-06.yHwy0b/build-release \
  --profile-build /private/tmp/crexx-perf3-12.9cGqDj/build-profile \
  --output-dir /private/tmp/crexx-perf3-12.9cGqDj/rxbvm-fixed200 \
  --vm rxbvm --profile-mode counts --force
```

The committed bundle manifests preserve the exact driver arguments and all
derived commands.  The runner source was unchanged and had SHA-256
`2495bc2c630e5862368e16dfebc89f459b4e3b5268357dd4aef8ed2ee9af5713`.

## Calibration exclusions

Two preliminary attempts were correct but unsuitable for optimized/no-opt
subtraction:

1. `--smoke-count 1` calibrated no-opt/opt to 36/103 under `rxvm` and 34/99
   under `rxbvm`.
2. `--smoke-count 100` left no-opt at 100 but calibrated optimized to 200 in
   both VMs.

They established the mismatch and were excluded before attribution.  In line
with the compact-evidence rule, reproducible calibration scratch bundles are
not committed.  The authoritative manifest fixes all four cells at 200; every
cell reports `initial_count=200`, `effective_count=200`, `averaging=100`, and
`calibrated=0`.

## Profile validity

All four final profiles satisfy:

- schema version 5;
- requested VM mode and `counts` profile mode;
- benchmark result 0 and `PASS: RexxCPS 2.2d cREXX port`;
- `invalid_events=0` and `counter_overflow=0`;
- procedure, allocation, census and branch tracking available;
- instruction, procedure, allocation, call-census, frame-entry,
  value-operation and branch-site domains complete; and
- successful RXSEQ N=2, N=3 and N=4 capture/analysis.

Summing every instruction row independently reproduces the summary totals:

| VM | no-opt sum | optimized sum |
| --- | ---: | ---: |
| `rxvm` | 148,701,541 | 54,221,210 |
| `rxbvm` | 148,701,541 | 54,221,182 |

The no-opt opcode vectors are exactly equal.  Optimized vectors differ only in
19 low-frequency opcodes, with an absolute difference sum of 28.  The material
candidate opcodes, PARSE counts, DCOPY/DTOS counts, compound-tail concat count,
stem counts, call count, allocations and value-operation counts are equal.

## Independent ceiling recomputation

- PARSE executes five operations per `lvar` in `main` and two per call in
  `cps_subroutine`: `(5 + 2) x 280,000 = 1,960,000`.
- PARSE copies execute 18 times per `lvar` in `main` and eight per call in the
  subroutine: `(18 + 8) x 280,000 = 7,280,000`.
- Current final grouping executes five parse-temporary null instructions in
  `main` and two in the subroutine: `(5 + 2) x 280,000 = 1,960,000`.
- PARSE direct-destination ceiling:
  `7,280,000 + 1,960,000 = 9,240,000`, or 17.041302% of 54,221,210.
- Five `"Key Bee." || lvar` sites are weighted 1, 1, 2, 2 and 2 per `lvar`:
  `8 x 280,000 = 2,240,000` materializations.  One-per-loop reuse removes
  `7 x 280,000 = 1,960,000`; segmented stem operations remove at most all
  2,240,000 concat dispatches.
- `DCOPY=DTOS=2,220,000`; the value-operation domain independently records
  2,220,000 decimal copies and 97,680,000 bytes.
- `cps_subroutine` has 280,000 calls; direct call plus return removal is at
  most 560,000 VM instructions before considering separate frame operations.

Instruction, frame, allocation and value-operation ceilings are not summed
across domains.

## Host state and checksums

The Mac was on AC with low-power mode off throughout.  Pre/post state is in
`provenance/`.  Profiles ran serially and no competing CREXX workload was
present.

To verify the complete bundle from this directory:

```sh
shasum -a 256 -c checksums.sha256
```

The nested `profiles/*/checksums.sha256` files independently validate each
runner bundle.
