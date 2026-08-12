# E3b-P1 inline-rework validation

- Focused Debug concurrency/ownership: 7/7 passed.
- Focused ordinary Release concurrency/ownership: 7/7 passed.
- Candidate and exact E3a-control optimized kernels returned the expected
  count before formal capture.
- Assembly contains no out-of-line `rxvm_call_native_procedure` frame and
  selects the direct/legacy adapters at the inlined call site.
- Formal timing: 312/312 processes passed, comprising 24 warmups and 288
  recorded executions. Every sample has `correctness=pass`.

Full Debug CTest, sanitizer, cross-platform, install/package and P2 work remain
closed because this reworked hot primitive also failed.
