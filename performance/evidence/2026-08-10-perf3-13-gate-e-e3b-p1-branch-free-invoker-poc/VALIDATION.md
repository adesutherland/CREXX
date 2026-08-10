# Validation

- Debug build: `e3b_rxpa_invoker_ceiling` built successfully.
- Debug CTest: 1/1 passed.
- Profiling-off Release build: target built successfully.
- Release CTest: 1/1 passed.
- The self-test proves that a reentrant-only executor does not transition,
  publication without legacy quiescence is rejected, a quiescent retry changes
  only legacy bindings, the transition is sticky, and a later legacy procedure
  binds locked.
- Formal timing: 65/65 processes passed: five warmups and 60 recorded cells.
- Raw files contain 12 recorded samples per cell and no failed correctness row.
- No cell requests a noise rerun; no sample was removed.
- `git diff --check` passed before and after preparation.
