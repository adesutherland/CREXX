# PERF3-11 K06 commands

Commands were run from `/Users/adrian/CLionProjects/CREXX`. Verbose build and
assembler output was redirected to temporary logs.

## Focused Debug and Release validation

    cmake --build BUILD --target rxas rxdas test_rxop_metadata --parallel 10
    ctest --test-dir BUILD --output-on-failure \
      -R '^(rxas_optimizer_metadata|rxas_optimizer_copy_acopy_(opt|noopt))$'

`BUILD` was `cmake-build-debug` and `cmake-build-release`.

## Exact retained-input replay

    git archive 78bd7f6f5 tests/rxas_optimizer/copy_acopy.rxas | \
      tar -x -C REPLAY_DIR
    cd REPLAY_DIR
    RXAS -d -o copy_acopy_opt.rxbin \
      tests/rxas_optimizer/copy_acopy.rxas
    RXAS -n -d -o copy_acopy_noopt.rxbin \
      tests/rxas_optimizer/copy_acopy.rxas
    cmp BASELINE_OPT.rxbin copy_acopy_opt.rxbin
    cmp BASELINE_NOOPT.rxbin copy_acopy_noopt.rxbin

The relative source path is preserved because source metadata participates in
the RXBIN hash.

## Canonical zero census

    RXAS -d -o OUTPUT.rxbin \
      cmake-build-release/tests/benchmarks/benchmark_awfy_richards_opt.rxas
    RXAS -d -o OUTPUT.rxbin \
      cmake-build-release/tests/benchmarks/benchmark_awfy_towers_opt.rxas
    RXAS -d -o OUTPUT.rxbin \
      cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxas
    rg -c 'signature=d132e98f' STDERR

## Closeout

    shasum -a 256 cmake-build-debug/bin/rxas \
      cmake-build-release/bin/rxas OUTPUTS
    git diff --check
