# Validation

Date: 2026-08-17

## Correctness

- PASS: final-source profiling-off Release opt/no-opt by
  `rxvm`/`rxtvm`/`rxbvm`, 6/6;
- PASS: final-source published results for 1, 15, 150 and 1,500 dummy
  recognitions, including 6,102 loops and 5,213 nodes at 1,500;
- QUALIFIED MATHEMATICALLY, NOT EXECUTED PASS: 15,000 gives
  `1,602 + 3 * 15,000 = 46,602` loops and the fixed graph has 5,213 nodes;
  bounded executions were interrupted at 916.56 and 1,567.48 seconds;
- PASS: maintained Level B runner in opt and no-opt product cells with one
  discarded full-graph recognition, 2/2;
- PASS: focused Debug CTest, 3/3 including its linked-runtime build fixture;
- PASS: focused profiling-off Release CTest, 3/3 including its linked-runtime
  build fixture.

## Closeout

- PASS: full Debug CTest, 2,240/2,240 in 370.34 seconds with
  `--parallel 30`;
- PASS: final `git diff --check`;
- no sanitizer run is required for this Level B benchmark/documentation-only
  stage;
- no production compiler, RXAS or VM edit occurs in POSTPERF-03, so no new
  mandatory first-Release performance verdict is triggered.

## Claim boundary

Havlak is a separately named cREXX reserve lane with a stable-indexed CFG
adaptation. It is not Tier A, not in a common aggregate, and not a direct
cross-language timing claim. CTest and the six-cell matrix establish
correctness. The single runner samples establish integration only.
