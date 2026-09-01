# NR-03 automated performance evidence bundle worklist

Last updated: 2026-07-15

Status: complete

This is the resumable implementation and validation ledger for NR-03. The exit
criterion remains the one in the dated performance-programme report: one
command must retain unprofiled product timing, profiling CSV, allocation data,
and RXSEQ N=2/3/4 evidence for an exact image set, then generate a reviewable
ranked and paired-delta report.

## Handover and audit baseline

- [x] Confirm branch `develop`, clean starting tree, and HEAD `0486cc609`.
- [x] Confirm `develop` started two commits ahead of `origin/develop`.
- [x] Confirm `cmake-build-release` is Release, Ninja, Apple clang via
  `/usr/bin/cc`, `-O3 -DNDEBUG`, and `CREXX_VM_PROFILING=OFF`.
- [x] Confirm no profiling build was configured at handover.
- [x] Audit `run_cross_runtime.crexx`, `run_lifecycle.crexx`, and
  `run_benchmarks.crexx` before defining the NR-03 contract.
- [x] Audit existing profiler/RXSEQ documentation and tests: accounting,
  table/CSV, procedure rows, RXSEQ N=2/3/4, exact-module mismatch, and the
  end-to-end documentation flow already exist.
- [x] Confirm the profiler currently supplies instruction, transition,
  interrupt, procedure/method/factory/native, entry/exit, unwind, overflow,
  and degraded-tracking evidence.
- [x] Confirm there is no current general value/frame allocation counter or
  CSV surface.
- [x] Confirm the missing counters can be implemented inside the private,
  compile-time profiling backend without a language, serialized RXBIN, public
  ABI, ownership, or architecture change.

## Contract and implementation

- [x] Define and document a versioned exact-image input manifest accepted by
  the orchestration command.
- [x] Define the retained bundle layout and its machine-readable manifest.
- [x] Record branch, commit, dirty scope, host/CPU/OS, compiler, generator,
  CMake options, tool versions, exact argv, image/module hashes, optimization
  mode, source/TRACE retention, warmups/runs, serial policy, correctness
  contract, and interpretation boundaries.
- [x] Extend the private profiler schema with explicitly defined allocation,
  value-storage, frame-allocation/reuse, byte, and high-water counters.
- [x] Compile every new counter hook out when `CREXX_VM_PROFILING=OFF`.
- [x] Preserve and expose counter overflow plus allocation/procedure tracking
  status in CSV and table output.
- [x] Add focused unit and end-to-end tests for the counter definitions and
  CSV schema.
- [x] Implement one Level B cREXX orchestration entry point under
  `performance/tools/`.
- [x] Keep warmups and all recorded serial samples, stdout, stderr, exit code,
  correctness result, elapsed time, and argv.
- [x] Use only the ordinary Release `rxvm` for product timing.
- [x] Run profiling and RXSEQ separately with the profiling VM.
- [x] Retain raw `.rxseq` files and decoded candidate CSV for N=2, N=3, and
  N=4.
- [x] Implement Level B cREXX summary generation ranking instruction count,
  total time, average latency, static sites, and modules.
- [x] Implement optimized/unoptimized paired baseline/candidate deltas without
  treating profiled elapsed time as throughput.
- [x] Preserve inclusive body, self, entry/exit, native-child, unwind,
  overflow, and degraded-tracking distinctions in generated reports.

## Initial retained proof set

- [x] Add a versioned explicit manifest for Sieve optimized/unoptimized.
- [x] Add Bounce optimized/unoptimized.
- [x] Add Richards optimized/unoptimized.
- [x] Add Base64 optimized/unoptimized.
- [x] Add RexxCPS optimized/unoptimized with bounded correctness-valid
  arguments.
- [x] Generate a dated `performance/evidence/2026-07-15-nr-03-.../` bundle
  without recapturing or rewriting NR-02 evidence.

## Validation gates

- [x] Focused parser/reporting/allocation-counter tests pass.
- [x] Optimized Release profiling build with `CREXX_VM_PROFILING=ON` passes.
- [x] Ordinary Release build proves profiling options/hooks are absent and no
  new hot-path work survives preprocessing/compilation.
- [x] One command generates the retained proof bundle.
- [x] Every manifest field, sample count, correctness result, CSV status,
  RXSEQ file, decoded candidate CSV, and checksum is verified.
- [x] `ctest --test-dir cmake-build-release -L benchmark
  --output-on-failure --parallel 10` passes.
- [x] Relevant profiling CTests pass.
- [x] `git diff --check` passes and final scope review excludes unrelated work.

## Documentation and completion

- [x] Update `docs/books/crexx_programming_guide/profiling.md` with exact
  allocation definitions, schema, status, and interpretation limits.
- [x] Update `docs/ai-context/RXVM_INTERPRETER.md` for the implemented private
  profiling surface.
- [x] Update `performance/README.md` with the one-command workflow and bundle
  contract.
- [x] Update `performance/ROADMAP.md` with the retained bundle and final gate.
- [x] Mark NR-03 complete only after every exit item above is satisfied;
  otherwise record the precise remaining gate and leave it in progress.
- [x] Commit the scoped completed work locally on `develop`; do not push.

## Current negative findings and boundaries

- No pre-existing allocation/value/frame counter data can be recovered from
  the schema-v2 profile CSV; NR-03 needs new profiling runs after the private
  schema extension.
- Profiling elapsed time is instrumentation evidence only. The summary must
  source baseline/candidate timing exclusively from serial samples produced by
  the normal Release VM.
- Startup and lifecycle measurements remain separate from steady-state NR-03
  workload timing.
- NR-03 does not change benchmark algorithms, NR-02 equivalence decisions,
  capability-gap dispositions, canonical RXBIN, language syntax, public ABI,
  or deterministic value/frame ownership.

## Completion record

- Retained bundle:
  `performance/evidence/2026-07-15-nr-03-automated-proof/`.
- Exact proof set: 10 images (five optimized/unoptimized pairs), 40 serial
  samples, 10 schema-3 timing profiles, 90 allocation rows, 30 binary RXSEQ
  captures, 30 decoded candidate CSVs, and 311 independently reverified
  checksums.
- The ordinary Release proof found no profile/sequence CLI options or symbols,
  no profiling compiler definition, and no profiling state/calls/counters in
  preprocessed `rxvmintp.c`.
- Release benchmark CTest passed 29/29; profiling/RXSEQ CTest passed 11/11.
- Passing timing samples were retained without outlier rejection. In
  particular, the Richards candidate median is materially slower in this
  small startup-inclusive proof set; NR-03 records the observation but does
  not assign optimizer causality or a regression threshold.
