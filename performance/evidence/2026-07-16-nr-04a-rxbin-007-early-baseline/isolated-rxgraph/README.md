# Isolated RXBIN 007 graph review

Status: exploratory evidence; current implementation measured and rejected;
replacement layout not selected

This evidence was produced by the disposable `rxgraph_bench` target. The
executable links the production `rxbin` library directly and does not build or
execute the compiler or VM. It loads real 007 images, discovers valid query
fixtures from them, reports serialized/retained size, and measures structural
queries.

## Build and run

```sh
cmake --build cmake-build-release --target rxas rxgraph_bench --parallel 10

./cmake-build-release/bin/rxgraph_bench \
  --iterations 1000000 --samples 7 \
  performance/evidence/2026-07-16-nr-04a-rxbin-007-early-baseline/images/runtime-interface-retained.rxbin

./cmake-build-release/bin/rxgraph_bench \
  --iterations 1000000 --samples 7 \
  performance/evidence/2026-07-16-nr-04a-rxbin-007-early-baseline/images/rexxcps-retained.rxbin
```

To exercise a freshly assembled standalone image without rebuilding CREXX:

```sh
cd cmake-build-release/tests/performance
../../bin/rxas -o rxgraph_iteration runtime_interface_lookup_compare_opt.rxas
../../bin/rxgraph_bench --iterations 1000000 --samples 7 rxgraph_iteration.rxbin
```

The first header-triggered `rxas` plus harness rebuild completed in 0.84
seconds and assembly took 0.31 seconds on the evidence host. The later focused
`rxgraph_bench` plus `test_rxgraph` rebuild completed in 0.8 seconds.

## Measurement discipline

- Host/build provenance is inherited from the parent early-gate README:
  ordinary profiling-off Release on Darwin arm64.
- The retained measurements were run serially, one image after another.
- An earlier exploratory invocation accidentally ran three cells concurrently.
  Its timing values were discarded because CPU contention inflated them; its
  size/count observations were identical.
- Each retained lookup metric uses 1,000,000 iterations and seven samples.
- `dispatch_bound_target` is explicitly `poc-not-production`: it compares a
  synthetic already-bound target array and does not claim that the VM currently
  binds `ProcRef`s.
- `image_load` is a small cold diagnostic, not a formal lifecycle result.

## Main result

Exact type support is approximately equality cost, but transitive support is
32-73 ns because the production API allocates and walks the graph on every
query. The scratch precomputed bit test is 0.85-0.98 ns and a direct bound
target load is 0.94 ns, both effectively at control cost. Numeric
dispatch/factory primitives are about 3-5 ns in isolation, but the VM then
performs unmeasured module/procedure scans to bind the portable callable. These
results reject the current implementation and prove the replacement target:
inline pointer equality/bit membership and an already bound direct target load.

The complete medians/minima are in `serial-results.csv`. The associated design
review and options are in
`performance/NR-04A-RXBIN-007-IMPLEMENTATION-REVIEW.md`.
