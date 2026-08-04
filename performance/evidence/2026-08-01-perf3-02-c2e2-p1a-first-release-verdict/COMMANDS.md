# P1A reproduction commands

Run from `/Users/adrian/CLionProjects/CREXX`.

## Build and focused proof

```sh
cmake --build cmake-build-debug --target rxas --parallel 10
cmake --build cmake-build-release --target rxas --parallel 10
ctest --test-dir cmake-build-debug \
  -R '^(rxas_optimizer_(duplicate_link.*|nr09_class1.*|whole_procedure.*|nr18_flow_harvest.*|storage_identity_flow)|storage_identity_runtime_.*)$' \
  --parallel 10 --output-on-failure
```

For `richards`, `towers`, and `tests/rxas_optimizer/storage_identity_flow.rxas`,
assemble with locked P1 and candidate using `-d`, compare the RXBIN with
`cmp`, filter `NR27 identity|flow|reject|bound` diagnostics, and compare those
with `cmp`.

## Three-way Release timing

The binaries were:

```text
pre_p1=/tmp/crexx-c2e2-p1-baseline.VosQ4p/rxas-baseline
locked_p1=/tmp/crexx-p1a.ceSYyq/rxas-p1-locked
candidate=cmake-build-release/bin/rxas
```

For each Richards and Towers input, run two unrecorded warmups per binary, then
30 rounds. In every round time the complete command with zsh
`EPOCHREALTIME`:

```sh
$binary -o /tmp/crexx-p1a.ceSYyq/formal-output.rxbin $input
```

Rotate the variant order through these six permutations, repeated five times:

```text
pre_p1 locked_p1 candidate
pre_p1 candidate locked_p1
locked_p1 pre_p1 candidate
locked_p1 candidate pre_p1
candidate pre_p1 locked_p1
candidate locked_p1 pre_p1
```

`paired-effects.csv` computes the per-round percentage delta, median and mean.
The 95% interval is `mean +/- t(0.975,29) * sample_sd / sqrt(30)`, using
`t=2.04523`. No sample is removed.
