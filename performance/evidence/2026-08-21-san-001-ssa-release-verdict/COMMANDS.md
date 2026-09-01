# Commands

The control was frozen before rebuilding the edited source:

```sh
verdict_dir=$(mktemp -d /private/tmp/crexx-san001-release-verdict.XXXXXX)
cp cmake-build-release/bin/rxas "$verdict_dir/rxas-control"
cp cmake-build-release/tests/benchmarks/benchmark_awfy_towers_opt.rxas "$verdict_dir/towers.rxas"
cp cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxas "$verdict_dir/rexxcps.rxas"
cmake --build cmake-build-release --target rxas --parallel 10
cp cmake-build-release/bin/rxas "$verdict_dir/rxas-candidate"
```

Control/candidate equality was checked by assembling each copied input with
both binaries, hashing the outputs and comparing them with `cmp -s`.

Elapsed records used `zsh/datetime` around one process invocation:

```sh
zmodload zsh/datetime
start=$EPOCHREALTIME
"$binary" -o "$output" "$verdict_dir/rexxcps"
finish=$EPOCHREALTIME
awk -v s="$start" -v e="$finish" 'BEGIN { printf "%.9f", e-s }'
```

There was one unrecorded warmup per binary.  Odd recorded rounds ran
control-candidate and even rounds candidate-control.  The initial 12 pairs
were extended to 24 and then 36 because the paired mean 95% interval continued
to cross zero.  No samples were removed.  Every timed output was hashed.

Four separate, balanced peak-RSS pairs used:

```sh
/usr/bin/time -lp -o "$timing" \
  "$binary" -o "$output" "$verdict_dir/rexxcps"
```

The final normal Release Towers proof was:

```sh
cmake --build cmake-build-release \
  --target benchmark_awfy_towers_opt_artifact --parallel 10
ctest --test-dir cmake-build-release \
  -R '^benchmark_awfy_towers_opt$' -LE fixture_setup --output-on-failure
```

CTest selected the required linked-artifact fixture plus the named Towers
test, 2/2; no ordinary broad suite was run.
