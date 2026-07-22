# cREXX performance workspace

This is the operational home for the cREXX performance programme. It keeps the
live roadmap, idea backlog, portfolio contract and retained evidence together
without turning test directories into a planning system.

The programme charter and 2026-07-15 evidence review remain in
[`docs/planning/release-1/performance-programme-report-2026-07-15.md`](../docs/planning/release-1/performance-programme-report-2026-07-15.md).
That dated report defines the `NR-*` activities and their exit criteria.
[`ROADMAP.md`](ROADMAP.md) is the live status overlay and the one place to
capture ideas that otherwise risk being lost.

## Directory map

| Location | Purpose |
| --- | --- |
| `performance/ROADMAP.md` | P0/P1 status, work notes, decisions and idea ledger |
| `performance/PERFORMANCE-GOVERNANCE.md` | Normative portfolio, sampling, aggregation, regression and claim policy |
| `performance/templates/performance-scorecard.md` | Standard publication structure |
| `performance/manifests/` | Versioned exact-image manifests, including the NR-03 proof set and NR-05 22-image call census |
| `performance/portfolio/manifest.md` | Versioned seed workload and measurement contract |
| `performance/portfolio/cross-runtime-plan.md` | Coverage targets and ooRexx/Regina/NetRexx/Java execution matrix |
| `performance/evidence/benchmark-median-summary.md` | Master per-date/run median comparison with explicit exclusions and comparability markers |
| `performance/evidence/` | Dated provenance, commands, raw samples and summaries |
| `performance/tools/run_cross_runtime.crexx` | Level B serial capture tool for one workload/runtime cell |
| `performance/tools/run_lifecycle.crexx` | Level B compile/translate and cold load-to-first-result capture across the three portfolio runtimes |
| `performance/tools/run_evidence_bundle.crexx` | Level B exact-image timing/profile/RXSEQ bundle orchestration and reporting |
| `performance/tools/run_cross_runtime_matrix.crexx` | Level B compact formal timing/RSS matrix capture, summary and aggregate reporting |
| `performance/tools/inventory_performance_artifacts.crexx` | Level B hash/size inventory for versioned performance artifact manifests |
| `performance/tools/report_nr09_macro_timings.zsh` | NR-09 all-form component/macro timing and review-ledger report from paired schema-4 profiles |
| `performance/capability-gaps.md` | Audited missing surfaces and candidates uncovered by portfolio ports |
| `tests/benchmarks/` | Portable, correctness-gated language workloads and runner |
| `tests/performance/` | Focused internal microbenchmarks and implementation comparisons |
| `docs/books/crexx_programming_guide/profiling.md` | Supported VM profiling and RXSEQ workflow |

Add future automation under `performance/tools/` only when it coordinates more
than one existing benchmark/profiler tool. Test sources remain under `tests/`.

`tools/run_cross_runtime.crexx` is the Level B cREXX orchestration layer for a
single workload/runtime cell. It executes serial process warmups and recorded
runs, requires an observable correctness string, retains stdout/stderr for
every sample, keeps process elapsed time separate from an optional
benchmark-native metric, and writes the exact argv and cREXX version to
`manifest.json`.

`tools/run_lifecycle.crexx` is also Level B cREXX. It keeps lifecycle phases
outside the steady-state aggregate and emits one CSV row per runtime, phase and
sequence. The final phase is named `load_first_result` because the public CLIs
do not expose a consistent loaded-but-not-executed boundary. Formal captures use
`--crexx-vm both`, share the compile/assemble rows, retain separate `rxvm` and
`rxbvm` load rows, and write the same median/IQR/MAD/noise summary fields as the
matrix driver.
`--append` preserves existing lifecycle rows, continues sequence numbering, and
refreshes the summary after a policy-required noise append.

`tools/run_cross_runtime_matrix.crexx` is the formal NR-10 matrix driver. Its
versioned manifest groups runtime cells by workload, and the driver rotates the
cell order per warmup and recorded round. It writes consolidated sample and
output tables, per-cell median/IQR/MAD/noise summaries, higher-is-better ratios,
and the four separately named common-portfolio geometric means. RSS capture
uses the same manifest with zero warmups and remains separate from timing. For
an aggregate row, `work` must be a positive integer and the timing summary uses
process-inclusive `work / elapsed` normalized throughput; raw elapsed time and
the exact work count remain in `samples.csv`. `--summary-only` with one or more
`--samples PATH` arguments refreshes the summary/ratio/geomean files from an
initial capture plus policy-required append blocks without changing the raw
captures.

Formal campaigns and publications follow
[`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md). Qualification pilots
do not become formal baselines merely by appearing in the result index. Use the
scorecard template, keep `rxvm`/`rxbvm` separate, publish the exact aggregate
membership, and retain one compact checksum-closed bundle with consolidated
raw tables rather than one output file per successful sample.

Canonical NetRexx common cells use `options nobinary decimal` with timed
numeric work held in NetRexx `Rexx` values. The generated Java and default
HotSpot JIT are the normal implementation substrate, not a reason to disable
JIT compilation. Record that substrate in the scorecard and keep any
`options binary`/primitive-Java result as an explicitly excluded control.

`tools/report_nr09_macro_timings.zsh` consumes paired `canonical-opt-rxvm.csv`
and `canonical-opt-rxbvm.csv` schema-4 profile directories plus the versioned
`manifests/nr09-macro-review-v1.tsv`. It emits exact component rows, per-form
handler and transition-aware estimates, and a 60-form review ledger covering
coherence, temporary-register policy and implementation/decision status.
Profile timing remains diagnostic; ordinary profiling-off Release isolation
is the decision source.

## Exact-image evidence bundles

`tools/run_evidence_bundle.crexx` accepts a versioned explicit manifest; it
does not discover or silently rebuild benchmark images. Each non-comment row
in a version-1 manifest has these pipe-delimited fields:

```text
id|pair|workload|variant|mode|image|modules|args|expectation|source_trace|warmups|runs|image_sha256|module_sha256s
```

Semicolons separate repeated module, argument, and module-hash values. Every
image and module hash must match before a run starts. `variant` is `baseline`
or `candidate` for paired-delta reporting; `mode` records the actual image
mode such as `noopt` or `opt`. Rebuilds that intentionally change an image or
runtime library require a new manifest/hash revision rather than an automatic
checksum update.

With an ordinary profiling-off Release build and an optimized
`CREXX_VM_PROFILING=ON` build already prepared, the retained proof bundle is
generated by one command:

```bash
cmake-build-release/bin/crexx performance/tools/run_evidence_bundle.crexx \
  --nokeep --args \
  --manifest performance/manifests/nr03-proof-v1.txt \
  --release-build cmake-build-release \
  --profile-build cmake-build-profile \
  --output-dir performance/evidence/2026-07-15-nr-03-automated-proof \
  --force
```

NR-05 reuses the same driver and exact-image contract, but requires schema-4
profiles and writes the dynamic census rows to `summary/call-census.csv` plus
a concise `summary/call-census.md` dashboard. Its versioned 22-image manifest
covers all eleven current language workloads in noopt/opt form:

```bash
cmake-build-release/bin/crexx performance/tools/run_evidence_bundle.crexx \
  --nokeep --args \
  --manifest performance/manifests/nr05-call-census-v1.txt \
  --release-build cmake-build-release \
  --profile-build cmake-build-profile \
  --output-dir performance/evidence/2026-07-16-nr-05-call-census \
  --force
```

For every entry the driver keeps serial warmup and recorded stdout/stderr,
exit/correctness/timing rows, a schema-4 profile CSV, and separate binary plus
decoded RXSEQ captures for N=2, N=3, and N=4. Product timing comes only from
the ordinary Release `rxvm`; profile elapsed time is diagnostic and never
enters the paired timing delta. NR-05 is a census, not a performance-win
claim. Startup/lifecycle timing remains separate.

The bundle root contains machine-readable repository, host, compiler, CMake,
tool-version/hash, argv, sampling, correctness and interpretation provenance;
per-entry exact-image manifests; summary CSV/Markdown; a verification record;
and a recursively verified `checksums.sha256`. Summary tables retain ranked
instruction timing/count data, callable metrics including inclusive body,
self, native child, entry/exit and unwind state, allocation/value/frame data,
dynamic call path/arity/kind/frame/return/mechanics/unwind census data, RXSEQ
static sites/modules, and unprofiled baseline/candidate deltas. The raw profile
CSV remains authoritative for transition, interrupt, overflow and
degraded-tracking details. Exact-image portfolio zeros mean “not observed in
these bounded cells”; the profiler's focused fixtures cover native, dynamic,
restoration, and signal-unwind cold paths separately.

Run it through the Release driver, placing the workload command after `--`:

```bash
cmake-build-release/bin/crexx performance/tools/run_cross_runtime.crexx \
  --nokeep --args \
  --workload sieve --runtime oorexx --warmups 1 --runs 3 \
  --expect "PASS: AWFY Sieve Classic port" \
  --output-dir performance/evidence/sieve/oorexx/pilot -- \
  /path/to/rexx tests/benchmarks/cross-runtime/classic/awfy_sieve.rex 50
```

## Working loop

1. Select or capture an item in `ROADMAP.md`; state the question and exit
   criterion.
2. Establish correctness and a dated baseline using the portfolio manifest.
3. Collect the least instrumentation needed to explain the cost.
4. Make one bounded change or prototype.
5. Repeat the same correctness and unprofiled measurements, then update the
   roadmap with the result, including a neutral or negative result.

`NR-01` began with the five-workload seed bundle under
`performance/evidence/2026-07-15-seed-portfolio/` and is now complete. The
approved portfolio, serial correctness-gated raw capture, machine/build
provenance and separate steady-state/lifecycle reports are proved by the NR-10
formal bundle; the NR-11 governance and scorecard define future publication.

Cross-runtime work is deliberately staged rather than deferred to one final
comparison: NR-01 fills the workload coverage matrix; NR-02 ports, qualifies
and runs the selected portfolio on CREXX, ooRexx and NetRexx (starting with
RexxCPS), with Regina limited to RexxCPS; and NR-10 records formal same-host
results as each workload becomes comparison-ready. See
`portfolio/cross-runtime-plan.md`.

## Technical pointers

- Compiler/emitter and register allocation:
  `docs/ai-context/CREXX_ARCHITECTURE.md` and
  `compiler/docs/emitter_architecture.md`
- Language argument semantics:
  `docs/books/crexx_language_reference/procedures_and_arguments.md`
- Assembler optimisation: `docs/ai-context/RXAS_ASSEMBLER.md`
- VM calls, signals and execution: `docs/ai-context/RXVM_INTERPRETER.md`
- Level B authoring: `docs/ai-context/CREXX_LEVELB_AUTHORING.md`
- Existing benchmark use and provenance: `tests/benchmarks/README.md`
