# E3b-P1 branch-free validation and closeout

- Pre-verdict focused Debug and ordinary Release concurrency/ownership tests:
  10/10 passed in each build.
- Optimized process-reentrant and legacy imported call kernels returned the
  expected `RXPA_CALLS=1000` under `rxbvm` and `rxtvm`.
- Assembly contains no `rxvm_call_native_procedure` symbol and no per-call
  capability branch; inspected call sites use the procedure-bound invoker.
- Formal timing: 312/312 processes passed, comprising 24 warmups and 288
  recorded executions. Every sample has `correctness=pass`.
- Full closeout Debug CTest: 2,017/2,017 passed in 455.29 seconds with
  `--parallel 30`.
- Focused closeout ordinary Release concurrency/ownership panel: 11/11 passed,
  including `e2_active_context_isolation` after the internal ADDRESS bridge
  declared its audited process-reentrant property.
- The complete ordinary Release build passed. Rebuilt `rxbvm` and `rxtvm` are
  byte-identical to the timed candidate, so the accepted verdict remains
  authoritative.
- The full Debug suite includes the installed external C and C++ SDK consumer
  tests, preserving legacy and new macro coverage.

The approved shortest Mac closeout did not add sanitizer, cross-platform,
install/package or P2 session work. Those remain separate follow-up scopes.
