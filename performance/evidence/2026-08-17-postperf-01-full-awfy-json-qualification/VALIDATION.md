# Validation

## Focused checks

After the first Release verdict was accepted, the final benchmark, compiler
regression and maintained-runner scope passed:

- Debug: 8/8;
- ordinary profiling-off Release: 8/8;
- direct optimized/unoptimized by `rxvm`/`rxtvm`/`rxbvm` matrix: 6/6;
- optimized and unoptimized maintained runner with explicit `awfy-json`: 2/2.

Both benchmark modes report 25,820 fixture bytes, 156 operations and 3,392
indexed nodes. The runner reports `process-smoke` and supplies the exact
fixture path; `awfy-json` is not added to its historical default set.

## Broad checks

```text
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
100% tests passed, 0 tests failed out of 2231
Total Test time (real) = 359.53 sec
```

`git diff --check` also passes. Fixture source/staged hashes match and the JSON
contract is independently confirmed by `jq` in `fixture-validation.txt`.

## Boundary

This is complete macOS source, compiler, benchmark and ordinary-runtime QA for
POSTPERF-01. It does not claim sanitizer, installer/package, Linux or Windows
qualification. Full AWFY Json remains a non-aggregate indexed-standard-library
adaptation.
