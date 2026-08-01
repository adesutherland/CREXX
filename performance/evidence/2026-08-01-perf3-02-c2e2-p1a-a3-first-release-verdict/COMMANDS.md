# P1A A3 reproduction commands

Run from `/Users/adrian/CLionProjects/CREXX`.

## Build and focused proof

```sh
cmake --build cmake-build-debug --target rxas --parallel 10
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(rxas_optimizer_(duplicate_link.*|nr09_class1.*|whole_procedure.*|nr18_flow_harvest.*|storage_identity_flow)|storage_identity_runtime_.*)$'
cmake --build cmake-build-release --target rxas --parallel 10
```

For absolute Richards, Towers and
`tests/rxas_optimizer/storage_identity_flow.rxas` input paths, assemble with
locked P1 and A3 using `-d`. Compare the RXBIN with `cmp`, filter
`NR27 (identity|flow|reject|bound).*` diagnostics with `rg -o`, and compare the
filtered files with `cmp`. Assemble without `-d` using all four oracles and
compare ordinary outputs with `cmp`.

## Four-way Release timing

The binaries were:

```text
pre_p1=/tmp/crexx-p1a-a3.yaiVu3/rxas-pre-p1
locked_p1=/tmp/crexx-p1a-a3.yaiVu3/rxas-p1-locked
a1=/tmp/crexx-p1a-a3.yaiVu3/rxas-a1-locked
a3=/tmp/crexx-p1a-a3.yaiVu3/rxas-a3-candidate
```

For each Richards and Towers input, run two unrecorded warmups per binary.
Then time complete assembly with zsh `EPOCHREALTIME`:

```sh
$binary -o /tmp/crexx-p1a-a3.yaiVu3/formal-output.rxbin $absolute_input
```

The first 24 rounds enumerate every permutation of `pre_p1`, `locked_p1`,
`a1` and `a3` once. Because the intervals crossed zero and the guard, append
these 12 even permutations under unchanged conditions:

```text
pre_p1 locked_p1 a1 a3
pre_p1 a1 a3 locked_p1
pre_p1 a3 locked_p1 a1
locked_p1 pre_p1 a3 a1
locked_p1 a1 pre_p1 a3
locked_p1 a3 a1 pre_p1
a1 pre_p1 locked_p1 a3
a1 locked_p1 a3 pre_p1
a1 a3 pre_p1 locked_p1
a3 pre_p1 a1 locked_p1
a3 locked_p1 pre_p1 a1
a3 a1 locked_p1 pre_p1
```

`paired-effects.csv` computes per-round percentage deltas. The 95% interval is
`mean +/- t(0.975,35) * sample_sd / sqrt(36)`, with `t=2.03011`. Quartiles use
the inclusive convention. No sample is removed.
