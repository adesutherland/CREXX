# Commands and sampling contract

All paths are relative to `/Users/adrian/CLionProjects/CREXX` unless absolute.
The two Release configurations were confirmed from their CMake caches:

```sh
cmake -S . -B cmake-build-nr14-baseline -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF
cmake -S . -B cmake-build-nr14-candidate -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF
```

The baseline was built while the six production surfaces still matched the
starting revision. The candidate was built after the selected D implementation
was frozen. Target-only generation covered the products, compiler exits,
focused tests and the three named benchmark artifacts.

The retained focused checks were selected with this exact test set in Debug and
candidate Release:

```sh
ctest --test-dir BUILD --parallel 8 --output-on-failure \
  -R '^(linked_opt_runtime_artifacts_build|test_parse_bin_build|test_parse_direct_bin_build|nr21_fixed_call_contract|nr14_frozen_parse_contract|benchmark_parse_frozen_noopt|benchmark_parse_frozen_opt|test_parse_direct_opt|test_parse_direct_noopt|test_parse_noopt|test_parse_opt)$'
```

`BUILD` was first `cmake-build-debug`, then
`cmake-build-nr14-candidate`. The retained logs report 11/11 for each.
Those logs are the final rerun after the `+0` eligibility fallback correction.
Re-linking all three candidate workload images afterward produced byte-identical
files to the measured images.

Each formal workload was linked from the corresponding product and its exact
library input with source/TRACE metadata stripped:

```sh
cmake-build-nr14-IMAGE/bin/rxlink -s \
  -o /tmp/nr14-release-images.pj6UWD/IMAGE-WORKLOAD.rxbin \
  cmake-build-nr14-IMAGE/tests/benchmarks/benchmark_WORKLOAD_opt.rxbin \
  cmake-build-nr14-IMAGE/bin/library.rxbin
```

The serial runtime commands were:

```sh
cmake-build-nr14-IMAGE/bin/VM \
  /tmp/nr14-release-images.pj6UWD/IMAGE-rexxcps_levelb.rxbin

cmake-build-nr14-IMAGE/bin/VM \
  /tmp/nr14-release-images.pj6UWD/IMAGE-parse_frozen.rxbin -a 100000

cmake-build-nr14-IMAGE/bin/VM \
  /tmp/nr14-release-images.pj6UWD/IMAGE-awfy_richards.rxbin -a 1
```

`IMAGE` is `baseline` or `candidate`; `VM` is `rxvm` or `rxbvm`.
Process elapsed used zsh's `EPOCHREALTIME` around exactly one command. Odd
rounds ran baseline then candidate; even rounds ran candidate then baseline.
The VMs were recorded separately. Native CPS, checksum, queue/hold counts,
return code and expected PASS text were captured before accepting a sample.

RexxCPS used one complete four-cell warmup and 12 new recorded pairs. The first
warmup loop omitted `zmodload zsh/datetime`, so its process elapsed fields are
invalid zeros; its four complete benchmark outputs and native CPS values remain
valid warmups. A fifth partial warmup cell was excluded and the whole formal
block was restarted. Its disposition is retained in
`raw/rexxcps-warmup-invalid-timer.csv`.

Focused PARSE, Richards and lifecycle use round 1 as the four-cell warmup and
rounds 2-13 as the initial 12 recorded pairs. Lifecycle ran the same PARSE image
with `-a 1` under:

```sh
/usr/bin/time -l cmake-build-nr14-IMAGE/bin/VM \
  /tmp/nr14-release-images.pj6UWD/IMAGE-parse_frozen.rxbin -a 1
```

The focused `rxvm` candidate PARSE absolute cell and three lifecycle absolute
cells received ten unchanged-condition serial samples after crossing the
relative-MAD/span rule. Richards `rxvm` and both lifecycle paired series
received balanced rounds 14-25 and 26-37 after their mean t interval continued
to cross zero, reaching the 36-pair cap. No sample was removed.

Summary reduction uses sorted R-7 linear-interpolated quartiles, the median of
absolute deviations for MAD, per-pair
`(candidate / baseline - 1) * 100`, and a two-sided Student-t interval around
the arithmetic mean of paired percentages. Critical values were 2.200985 for
11 degrees of freedom and 2.030108 for 35 degrees of freedom. The reduction was
an ad hoc, non-repository Ruby/CSV calculation; no performance tool was added or
changed.
