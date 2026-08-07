# PERF3-13 Gate C C2 formal geometry commands

Run from `/private/tmp/crexx-rxvm-inline.yvLywZ/source` on the reserved host.
The runner binary is the accepted Gate B Release `crexx`:

```text
/private/tmp/crexx-perf3-13-gateb.qFFjMa/control-build-release/bin/crexx
```

## Runner proof

```sh
$CREXX performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args --self-test
```

## Initial formal timing block

```sh
$CREXX performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-survivors-manifest.txt \
  --output-dir /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-survivors-timing \
  --measurement timing --warmups 1 --runs 12
```

## Governed timing appends

```sh
$CREXX performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-noise-append-manifest.txt \
  --output-dir /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-noise-append \
  --measurement timing --warmups 0 --runs 10

$CREXX performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-survivors-manifest.txt \
  --output-dir /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-balanced-append \
  --measurement timing --warmups 0 --runs 12

$CREXX performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-terminal-append-manifest.txt \
  --output-dir /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-terminal-append \
  --measurement timing --warmups 0 --runs 12
```

## Final Level B paired reduction

```sh
$CREXX \
  performance/evidence/2026-08-06-perf3-13-gate-c-c2-geometry-formal-verdict/tools/summarize_geometry_paired.crexx \
  --nocolour --nokeep --args \
  /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-survivors/final-paired-summary.csv \
  /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-survivors/final-stable-six.csv \
  /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-survivors-timing/samples.csv \
  /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-noise-append/samples.csv \
  /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-balanced-append/samples.csv \
  /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-terminal-append/samples.csv
```

## Separate RSS panel

```sh
$CREXX performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-survivors-manifest.txt \
  --output-dir /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-formal-survivors-rss \
  --measurement rss --warmups 0 --runs 4
```

`$CREXX` above denotes the exact absolute runner binary printed at the start of
this file; it was not an exported environment variable in the retained run.
