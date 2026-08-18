# Replay commands

## Product and images

The candidate used the ordinary profiling-off Release tree:

```sh
cmake --build cmake-build-release \
  --target performance_postperf04_scalar_access rxtvm rxbvm rxlink \
  --parallel 10
```

The CMake cache was checked for `Release`, `CREXX_VM_PROFILING=OFF` and the
default profile-20 handler panel. The retained G0 RXAS was:

```text
/tmp/crexx-postperf04-inline.6lKUwW/postperf04.rxas
```

The retained pre-guard assembler was:

```text
/tmp/postperf04-guard-baseline.m5kK5d/bin/rxas
```

G0 and the current compiler candidate were assembled through that preserved
assembler to separate compiler effects from guard deletion. The candidate was
also assembled with the selected current RXAS:

```sh
PRE_GUARD_RXAS -o VERDICT/base.rxbin G0.rxas
PRE_GUARD_RXAS -o VERDICT/preguard.rxbin CANDIDATE.rxas
cmake-build-release/bin/rxas -o VERDICT/current.rxbin CANDIDATE.rxas
```

Exact hashes and sizes are retained in `artifact-summary.csv` and
`hashes-and-sizes.txt`.

## Correctness and generated shape

The focused gate selected these tests:

```sh
ctest --test-dir cmake-build-debug --output-on-failure -R \
  '^(rxas_optimizer_metadata|rxas_flow_graph_contract|rxas_optimizer_barrier_(asserttype|assertinitialized|ichkrng|bcheckrange)|rxas_optimizer_successful_guard_flow_(optimized|noopt))$'
```

The scalar fixture additionally ran optimized/no-opt through `rxtvm` and
`rxbvm`, the source/binary-import regression and
`postperf04_scalar_access_codegen`. The code-generation check disassembles the
optimized RXBIN and requires exactly one initialization guard in `main()`: the
computed-receiver guard following the non-inlined factory call.

## Formal timing

Every timing process used the same command shape:

```sh
cmake-build-release/bin/VM VERDICT/VARIANT.rxbin \
  cmake-build-release/bin/library -a 100000 10
```

`VM` was concrete `rxtvm` or `rxbvm`; `VARIANT` was `base`, `preguard` or
`current`. One warm-up preceded each cell. The initial 12 recorded rounds used
this balanced serial rotation:

```text
odd:  rxtvm/base, rxbvm/current, rxtvm/preguard,
      rxbvm/preguard, rxtvm/current, rxbvm/base
even: rxbvm/base, rxtvm/current, rxbvm/preguard,
      rxtvm/preguard, rxbvm/current, rxtvm/base
```

Initial absolute series exceeding 3% relative MAD or 10% min/max span received
the single required ten-sample serial append. Paired intervals crossing zero
or the 3% guard received two unchanged 12-round balanced append blocks to the
36-pair ceiling. No observation was removed.

For each round the elapsed percentage is:

```text
(current / control - 1) * 100
```

`paired-summary.csv` uses R-7 quartiles and a two-sided 95% Student-t interval
around the mean paired percentage. For 36 pairs the critical value is
2.030108. The paired median is the headline point estimate.

## Closeout qualification

The complete product qualification used:

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
cmake --build cmake-build-release --parallel 10
```

The accepted selected RXBIN is path-identity-sensitive because RXAS records
the input module identity. Its byte-exact closeout reproduction therefore used
the original absolute input spelling:

```sh
cmake-build-release/bin/rxas \
  -o /tmp/postperf04-final-current-absolute.rxbin \
  /Users/adrian/CLionProjects/CREXX/cmake-build-release/tests/performance/postperf04_scalar_access_compare_opt.rxas
```

That output compares byte-for-byte with the retained accepted `current.rxbin`.

The post-acceptance assembler lifecycle panel used the exact real
`httpcodec.rxas` input, one warm-up and 12 balanced rounds across the retained
pre-guard assembler, the selected current assembler and a bounded condition-
reorder diagnostic. The reorder was neutral and reverted. Raw observations are
retained in `assembler-lifecycle-samples.csv`.
