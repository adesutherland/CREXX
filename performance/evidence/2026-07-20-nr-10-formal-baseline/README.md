# NR-10 formal same-host baseline

Status: complete formal absolute-baseline evidence; interpretation is an
`observation`, not a release claim

This compact bundle closes the NR-10 same-host baseline for commit
`1596d7c8cbbfd360b66d50423ef4ea80320fd885` and proves the NR-11 governance and
scorecard structure. It reuses the detailed NR-02 equivalence, optimizer-
resistance, ooRexx trace and NetRexx generated-form forensics by reference rather
than copying them.

## Result entry points

- `scorecard.md`: factual human publication and interpretation boundaries.
- `timing-equal-work/summary.csv`, `ratios.csv`, `geomeans.csv`: authoritative
  equal-work timing summaries after the policy-required append.
- `rss-equal-work/summary.csv`: authoritative equal-work peak-RSS summary after
  the policy-required append.
- `lifecycle-decimal/summary.csv`: authoritative decimal-NetRexx
  compile/translate/assemble/load summary after append.
- `controls/results/summary.csv`: two labelled native C upper-bound controls.
- `artifacts.csv`: 101 exact paths, sizes and SHA-256 hashes.
- `provenance/`: before/after machine, power, build and tool fingerprints.
- `checksums.sha256`: recursive bundle closure, verified after finalization.

## Sampling and raw data

The corrected formal timing matrix ran serially with rotated runtime order, two
warmups and ten recorded samples for all 20 common cells: 240 rows, zero
failures. Every runtime uses the same work argument within a workload. The
qualification pilot confirmed fastest medians of 1.062-1.182 seconds and
slowest medians of 1.614-16.061 seconds, so the ordinary equal-work rule applies
and the earlier unequal-work exception is unnecessary. The noise gate
(`MAD > 3%` or min/max span `> 10%`) selected three cells for an unchanged
ten-sample append, adding 30 rows. No sample was removed.

Peak RSS used zero warmups and three recorded executions for the same 20 cells:
60 rows, zero failures. Decimal NetRexx Base64 triggered a ten-sample append,
giving n=13. Lifecycle contains 160 passing decimal-NetRexx phase rows:
sequences 1-10 are the initial formal run and 11-20 the unchanged append. Both
native C controls used 2+10 sampling.

The original `timing*` and `rss*` directories without `equal-work` came from
the first version-1 matrix. Its NetRexx common cells used `options binary`, so
they are withdrawn from the Rexx aggregate and retained only as a binary/JVM
audit; the same consolidated files also preserve the separate non-common
diagnostics. The superseded binary lifecycle directory, derived append
summaries and empty RSS/control ratio tables are not retained. No duplicate
binary generated forms or reproducible build products are retained.

Cells that remain over the noise threshold after append are labelled noisy in
the scorecard and source summaries. An extreme without an independently proven
fault remains evidence.

## Reproduction

Build the ordinary profiling-off product and benchmark images:

```bash
cmake --build cmake-build-release --target language_benchmarks --parallel 10
ctest --test-dir cmake-build-release -L benchmark --parallel 10 --output-on-failure
```

Run the formal timing and RSS matrices:

```bash
cmake-build-release/bin/crexx performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args \
  --manifest performance/manifests/nr10-formal-baseline-v3.txt \
  --output-dir performance/evidence/2026-07-20-nr-10-formal-baseline/timing-equal-work \
  --measurement timing --warmups 2 --runs 10

cmake-build-release/bin/crexx performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args \
  --manifest performance/manifests/nr10-formal-baseline-v3.txt \
  --output-dir performance/evidence/2026-07-20-nr-10-formal-baseline/rss-equal-work \
  --measurement rss --warmups 0 --runs 3
```

The exact append schedules are
`performance/manifests/nr10-timing-noise-append-v3.txt` and
`performance/manifests/nr10-rss-noise-append-v3.txt`. Refresh the authoritative
summary with `--summary-only`, passing the initial and append `samples.csv`
paths. Lifecycle uses:

```bash
cmake-build-release/bin/crexx performance/tools/run_lifecycle.crexx \
  --nokeep --args --runtime all --crexx-vm both --runs 10 \
  --repo-root . --build-dir cmake-build-release \
  --output-dir performance/evidence/2026-07-20-nr-10-formal-baseline/lifecycle-decimal
```

Use the same command with `--append` for the policy append.

Compile the evidence-only C controls, then run the retained control manifest:

```bash
/usr/bin/clang -O3 -std=c11 -Wall -Wextra -pedantic \
  performance/evidence/2026-07-20-nr-10-formal-baseline/controls/rexxcps_mechanical.c \
  -o performance/evidence/2026-07-20-nr-10-formal-baseline/controls/build/rexxcps_mechanical
/usr/bin/clang -O3 -std=c11 -Wall -Wextra -pedantic \
  performance/evidence/2026-07-20-nr-10-formal-baseline/controls/rexxcps_dynamic_value.c \
  -o performance/evidence/2026-07-20-nr-10-formal-baseline/controls/build/rexxcps_dynamic_value
```

The generated control binaries are deliberately not retained after their hashes
and sizes are recorded in `artifacts.csv`; rebuild them before replaying
`performance/manifests/nr10-native-c-controls-v1.txt`.

## Retention decisions

Retained: consolidated canonical raw tables and outputs, the version-1
binary/non-common audit, summaries, exact manifests, scorecard, provenance,
decimal NetRexx generated Java/classes, artifact inventory, lifecycle final
forms, C control source, and checksums.

Not retained: disposable calibration runs, failed setup/smoke directories,
the superseded unequal-work decimal capture, build logs, repeated per-sample
files, duplicate NR-02 forensics, binary generated-form duplicates, ordinary
build products, and reproducible C control binaries. This bundle therefore
stays compact while preserving every fact needed to audit the publication and
its correction.
