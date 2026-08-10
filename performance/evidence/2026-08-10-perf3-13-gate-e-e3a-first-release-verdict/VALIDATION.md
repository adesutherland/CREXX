# E3a validation record

## Pre-fix reproducer

The new ownership test was first built in reproducer mode against the old
shared-instance path. It completed with:

```text
Pre-E3a shared decimal-provider collision reproduced
```

The disposable reproducer mode was removed from the test source after Adrian
accepted the verdict. The production test retains only the final isolation and
lifecycle assertions.

## Minimum pre-verdict correctness

- Final Debug focused CTest: 15/15 passed. This covered
  `rxvmplugin_context_ownership`, `rxvmactive_isolation`, the three RXAS decimal
  provider cases, reentrancy, DB archive linking, decNumber, MC static/manual/
  dynamic/full cases and DB full tests.
- Both concrete Debug engines ran `tests_decimal` with the default provider:
  73 tests, 0 errors for `rxtvm`; 73 tests, 0 errors for `rxbvm`.
- Both concrete Debug engines repeated `tests_decimal` with
  `-p rxvm_mc_decimal`: 73 tests, 0 errors for each engine.
- After moving provider preparation behind the worker-affinity entry, the
  focused ownership/active/reentrancy/default-MC-DB decimal subset passed 6/6.
- The Release `rxvmplugin_context_ownership` test passed 1/1 before timing.

The final ownership test additionally proves duplicate manual and dynamic load
idempotence, failing/nonexistent load rollback, later-provider priority,
two-thread state isolation, catalogue clearing with two live contexts, reverse
teardown and zero final live instances.

## Formal capture integrity

- Capture manifest result: `pass`.
- Process rows: 208 total, comprising 16 warmups and 192 recorded executions.
- Exit status: 208/208 zero.
- Correctness oracle: 208/208 `pass`.
- Recorded failures: zero.
- Sampling: serial, pairwise-balanced by workload; 12 recorded pairs per
  workload/VM comparison.
- Reducer: maintained cREXX Level B `summarize_paired.crexx`; output retained in
  `paired-summary.csv`.
- Verdict: all eight 95% intervals cross zero; all eight 3% guards are clear.

## Accepted closeout

- Adrian accepted E3a on 2026-08-10 and directed the programme to E3b.
- Post-cleanup focused Debug: 15/15 passed.
- Post-cleanup Release ownership: 1/1 passed.
- The first full Debug build found a missing plugin-subdirectory include for
  auxiliary `rxvmintp.h` consumers. The interpreter-root-relative include
  correction rebuilt successfully across all targets.
- Complete Debug CTest: 2,007/2,007 passed in 291.77 seconds with
  `--parallel 30`.
- Rebuilt profiling-off Release `rxtvm` and `rxbvm` are byte-identical to the
  accepted verdict artifacts, so no performance rerun was warranted.

## Hygiene and deferred checks

`git diff --check` is rerun after sealing this bundle. AddressSanitizer, native
Windows/Linux execution proof and install/package verification were not added
to the approved shortest closeout. E3b implementation remains behind its own
architecture approval gate.
