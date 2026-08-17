# Validation

## Correctness checks

- CD optimized and no-opt compilation: Debug and ordinary profiling-off
  Release pass.
- Focused Debug CTest, including the linked optimized runtime: 3/3 pass.
- Final focused Release CTest, including the linked optimized runtime: 3/3
  pass in 0.19 seconds with all artifacts current.
- Direct size-10 optimized/unoptimized by `rxvm`/`rxtvm`/`rxbvm` in Debug and
  Release: 12/12 pass.
- Complete no-opt reference matrix for 2, 10, 100, 200, 250, 500 and 1,000
  aircraft: 7/7 pass.
- Optimized reference checks at 2, 10 and 100 aircraft: 3/3 pass.
- Explicit maintained-runner checks and the four-cell bounded pilot: pass.
- Stripped-linked size-100 optimized/no-opt product executions: 2/2 pass and
  both report exactly 4,305 collisions.

The larger optimized references were not repeated after size 100 because the
same compiled program had already passed three input scales and the retained
33.96-second result made further Debug/Release timing disproportionate. This
is an explicit QA boundary, not an inferred result.

## Broad checks

```text
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
100% tests passed, 0 tests failed out of 2238
Total Test time (real) = 323.87 sec
```

Final `git diff --check` also passes.

## Boundary

This is macOS source, benchmark and ordinary-runtime qualification for CD and
the completion of POSTPERF-02. It does not claim sanitizer, installer/package,
Linux or Windows qualification. CD remains a non-aggregate
`indexed-red-black-tree-native-math` adaptation.
