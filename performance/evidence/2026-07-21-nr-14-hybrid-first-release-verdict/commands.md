# Commands and reduction contract

All commands ran from `/Users/adrian/CLionProjects/CREXX`.

## Focused validation

```sh
ctest --test-dir BUILD --parallel 8 --output-on-failure \
  -R '^(linked_opt_runtime_artifacts_build|test_parse_bin_build|test_parse_direct_bin_build|nr21_fixed_call_contract|nr14_frozen_parse_contract|benchmark_parse_frozen_noopt|benchmark_parse_frozen_opt|benchmark_parse_frozen_generic_noopt|benchmark_parse_frozen_generic_opt|test_parse_direct_opt|test_parse_direct_noopt|test_parse_noopt|test_parse_opt)$'
```

`BUILD` was `cmake-build-debug` and then
`cmake-build-nr14-candidate`; both passed 13/13.

## Image creation

The new generic source was compiled independently by each product:

```sh
cmake-build-nr14-IMAGE/bin/rxc \
  -i cmake-build-nr14-IMAGE/bin -o TMP/IMAGE-generic \
  tests/benchmarks/parse_frozen_generic.crexx
cmake-build-nr14-IMAGE/bin/rxas \
  -o TMP/IMAGE-generic TMP/IMAGE-generic.rxas
cmake-build-nr14-IMAGE/bin/rxlink -s \
  -o TMP/IMAGE-generic-linked.rxbin \
  TMP/IMAGE-generic.rxbin cmake-build-nr14-IMAGE/bin/library.rxbin
```

Existing optimized PARSE, RexxCPS and Richards artifacts were linked with `-s`
against the matching baseline/candidate library. Reassembling and relinking the
generic RXAS at the same canonical paths after final validation reproduced both
measured hashes exactly.

## Runtime cells

```sh
cmake-build-nr14-IMAGE/bin/VM TMP/IMAGE-generic-linked.rxbin -a 100000 all
cmake-build-nr14-IMAGE/bin/VM TMP/IMAGE-parse_frozen-linked.rxbin -a 100000
cmake-build-nr14-IMAGE/bin/VM TMP/IMAGE-rexxcps_levelb-linked.rxbin
cmake-build-nr14-IMAGE/bin/VM TMP/IMAGE-awfy_richards-linked.rxbin -a 1
```

`IMAGE` is baseline/candidate and `VM` is `rxvm`/`rxbvm`. Zsh
`EPOCHREALTIME` surrounded exactly one command. Odd rounds ran baseline then
candidate; even rounds reversed the order. Round 1 is warmup. Rounds 2-13 are
the 12 formal pairs. Richards appended balanced rounds 14-37 after its interval
crossed zero. Every process passed its exact checksum/native result and PASS
marker; RexxCPS also passed canonical provenance.

Lifecycle used `/usr/bin/time -l` around the one-repetition generic command and
retained maximum RSS and peak memory footprint.

The output-strategy PoC used 300,000 repetitions, six balanced orders, and all
three modes (`generic`, `prepared`, `chained`) on each VM.

Summary reduction uses R-7 interpolated medians/quartiles, median absolute
deviation, per-pair `(candidate / baseline - 1) * 100`, and two-sided 95%
Student-t intervals around the mean paired percentage. Critical values are
2.200985 for 12 pairs and 2.030108 for 36 pairs. No passing sample was removed.
