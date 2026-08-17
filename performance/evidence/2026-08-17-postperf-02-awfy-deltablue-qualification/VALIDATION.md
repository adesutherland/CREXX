# Validation

## Focused checks

After the compiler-repair verdict was accepted, the final benchmark,
regression and maintained-runner scope passed:

- Debug: 8/8;
- ordinary profiling-off Release: 8/8;
- direct optimized/unoptimized by `rxvm`/`rxtvm`/`rxbvm` benchmark matrix:
  6/6;
- explicit `deltablue` maintained-runner checks in Debug and Release,
  optimized and unoptimized: 4/4.

Both benchmark modes report the complete chain and projection contracts as
passing. The runner reports `process-smoke`, uses bounded size 500 and does not
add DeltaBlue to its historical default set.

The first focused Release invocation found only an absent no-opt artifact in
the partially built tree. After building
`benchmark_awfy_deltablue_noopt_artifact`, the cell passed; the final complete
focused selection then passed 8/8.

## Broad checks

```text
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
100% tests passed, 0 tests failed out of 2236
Total Test time (real) = 311.69 sec
```

`git diff --check` also passes.

## Boundary

This is complete macOS source, compiler, benchmark and ordinary-runtime QA for
the DeltaBlue half of POSTPERF-02. It does not claim sanitizer,
installer/package, Linux or Windows qualification. DeltaBlue remains a
non-aggregate `stable-indexed-constraint-graph` adaptation. CD remains the
active second half of POSTPERF-02.
