# Performance workspace

Use this workspace for current performance direction, durable decisions,
measurement rules, maintained tools and the selected comparison portfolio.
Completed investigations and superseded runs are available in Git history.

## Current authorities

| File | Purpose |
| --- | --- |
| [ROADMAP.md](ROADMAP.md) | Current closeout obligations and Beta 6 candidate selection |
| [RESULTS.md](RESULTS.md) | Frozen Apple scorecard, comparison limits and remaining Linux disposition |
| [DECISIONS.md](DECISIONS.md) | Accepted mechanisms, rejected alternatives and reopening triggers |
| [PERFORMANCE-GOVERNANCE.md](PERFORMANCE-GOVERNANCE.md) | Sampling, aggregation, noise and regression rules |
| [PERFORMANCE-CLOSEOUT-PLAN.md](PERFORMANCE-CLOSEOUT-PLAN.md) | Remaining scorecard closeout criteria |
| [portfolio/manifest-v3.md](portfolio/manifest-v3.md) | Current workload, equivalence and capability contract |
| [PERF3-FUSION-REGISTRY.md](PERF3-FUSION-REGISTRY.md) | Implemented fusion ownership and fallback contracts |
| [NR-09-MAPPING-REGISTER.md](NR-09-MAPPING-REGISTER.md) | Implemented instruction mapping contracts |
| [UNICODE-CERT-01-WORKLIST.md](UNICODE-CERT-01-WORKLIST.md) | Unresolved formal normalization-certificate verdict |
| [VALUE-CACHE-01-WORKLIST.md](VALUE-CACHE-01-WORKLIST.md) | Unselected numeric-cache design and residual-cost questions |
| [templates/performance-scorecard.md](templates/performance-scorecard.md) | Scorecard authoring template |

The project-wide [roadmap](../docs/ROADMAP.md) and
[Release 1 plan](../docs/release-1-plan.md) own product scope and dates.
[AGENTS.md](AGENTS.md) defines the performance implementation gates.

Keep portable workloads in `tests/benchmarks/`, focused comparisons in
`tests/performance/`, and user-facing profiling guidance in the
[programming guide](../docs/books/crexx_programming_guide/profiling.md).
The compact [Stage 5 baseline](evidence/2026-08-18-performance-closeout-stage5/)
remains because its exact-commit Linux QA-C disposition is open. Other dated
captures are recovered from Git when needed; do not copy them back into HEAD
merely to preserve investigation history.

## Maintained tools

| Tool | Purpose |
| --- | --- |
| `tools/run_cross_runtime.crexx` | Serial correctness-gated capture for one workload/runtime cell |
| `tools/run_cross_runtime_matrix.crexx` | Formal timing/RSS matrix, summaries and aggregate reporting |
| `tools/run_lifecycle.crexx` | Compile/translate and cold load-to-first-result capture |
| `tools/run_evidence_bundle.crexx` | Exact-image timing, profiling and RXSEQ orchestration |
| `tools/build_sequence_ledger.crexx` | Instruction-sequence analysis |
| `tools/inventory_fusions.crexx` | Public/private fusion inventory |
| `tools/inventory_performance_artifacts.crexx` | Bounded versioned artifact hash/size inventory |
| `tools/summarize_native_inference.crexx` | Matched native-inference comparison summaries |
| `tools/run_java_class.ps1` | Windows Java classpath adapter |
| `tools/windows_peak_rss.ps1` | Windows child-process peak working-set capture |

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
`--crexx-vm both`, share the compile/assemble rows, retain separate `rxtvm` and
`rxbvm` load rows, and write the same median/IQR/MAD/noise summary fields as the
matrix driver. The Stage 5 correction deliberately removed `rxvm` from this
concrete-engine selector because product `rxvm` may alias either engine.
`--append` preserves existing lifecycle rows, continues sequence numbering, and
refreshes the summary after a policy-required noise append.

`tools/run_cross_runtime_matrix.crexx` is the formal NR-10 matrix driver. Its
versioned manifest groups runtime cells by workload, and the driver rotates the
cell order per warmup and recorded round. It writes consolidated sample and
output tables, per-cell median/IQR/MAD/noise summaries, higher-is-better ratios,
and the four separately named common-portfolio geometric means. RSS capture
uses the same manifest with zero warmups and remains separate from timing.
Unix hosts use `/usr/bin/time`; native Windows uses `windows_peak_rss.ps1` to
sample the direct child's `PeakWorkingSet64`. For
an aggregate row, `work` must be a positive integer and the timing summary uses
process-inclusive `work / elapsed` normalized throughput; raw elapsed time and
the exact work count remain in `samples.csv`. `--summary-only` with one or more
`--samples PATH` arguments refreshes the summary/ratio/geomean files from an
initial capture plus policy-required append blocks without changing the raw
captures. Sample files are streamed into the merged collection so Windows
multi-file summaries do not retain a second whole-file array for each input.

`tools/run_java_class.ps1` is a Windows-only matrix adapter for Java controls.
The manifest format uses semicolons as argv separators, which conflicts with
the Windows Java classpath separator. The adapter accepts the Java executable,
class directory, runtime JAR and main class as separate manifest arguments,
constructs one native classpath argument and preserves the child exit code.

Formal campaigns and publications follow
[`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md). Qualification pilots
do not become formal baselines merely by appearing in the result index. Use the
scorecard template, keep `rxvm`/`rxbvm` separate, publish the exact aggregate
membership, and retain one compact checksum-closed bundle with consolidated
raw tables rather than one output file per successful sample.

RexxCPS is a mandatory first-class community lane in every multi-workload
representative benchmark, profile, PMU and candidate-verdict set. It remains
separate from the common-five aggregate because cREXX 2.2d and NetRexx 2.2n
are disclosed adaptations. A genuinely single-mechanism experiment may omit
it only when the experiment is explicitly labelled non-representative.

Canonical NetRexx common cells use `options nobinary decimal` with timed
numeric work held in NetRexx `Rexx` values. The generated Java and default
HotSpot JIT are the normal implementation substrate, not a reason to disable
JIT compilation. Record that substrate in the scorecard and keep any
`options binary`/primitive-Java result as an explicitly excluded control.

[`tools/report_nr09_macro_timings.zsh`](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/performance/tools/report_nr09_macro_timings.zsh) consumes paired `canonical-opt-rxvm.csv`
and `canonical-opt-rxbvm.csv` schema-4 profile directories plus the versioned
[`manifests/nr09-macro-review-v1.tsv`](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/performance/manifests/nr09-macro-review-v1.tsv). It emits exact component rows, per-form
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

With ordinary profiling-off Release and profiling-enabled builds prepared,
create a manifest for the selected current images using the schema above:

```bash
cmake-build-release/bin/crexx performance/tools/run_evidence_bundle.crexx \
  --nokeep --args \
  --manifest /path/to/current-image-manifest.txt \
  --release-build cmake-build-release \
  --profile-build cmake-build-profile \
  --output-dir performance/evidence/current-image-comparison \
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

Apply [AGENTS.md](AGENTS.md), including design selection and the first ordinary
Release verdict before broad closeout. Preserve current comparison boundaries
and recover superseded campaign details from Git only when needed.

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
