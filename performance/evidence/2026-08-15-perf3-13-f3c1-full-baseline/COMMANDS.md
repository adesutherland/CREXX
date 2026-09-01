# Replay commands

Commands ran from the repository root. The retained logs and capture manifests
are the exact command-output/time authority.

```sh
cmake --build cmake-build-release \
  --target linked_opt_runtime_artifacts rxtvm rxbvm --parallel 10

caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/manifest.txt \
  --output-dir performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/timing \
  --measurement timing --warmups 1 --runs 12
```

The unchanged governance appends used the same command with `--warmups 0
--runs 12` and output directories `timing/append-1` and `timing/append-2`.

The final absolute summary merges all three sample files:

```sh
cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/manifest.txt \
  --output-dir performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/timing/combined \
  --measurement timing --summary-only \
  --samples performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/timing/samples.csv \
  --samples performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/timing/append-1/samples.csv \
  --samples performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/timing/append-2/samples.csv
```

Final paired and aggregate reduction:

```sh
cmake-build-release/bin/crexx \
  performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/summarize_paired_36.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/paired-summary-36.csv \
  performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/timing/samples.csv \
  performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/timing/append-1/samples.csv \
  performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/timing/append-2/samples.csv

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/summarize_common_five_36.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/common-five-summary-36.csv \
  performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/timing/samples.csv \
  performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/timing/append-1/samples.csv \
  performance/evidence/2026-08-15-perf3-13-f3c1-full-baseline/timing/append-2/samples.csv
```
