# PERF2-02 focused correctness and lifecycle

Status: complete for the recommended zero-state direct surface; Q7 gaps retained

## Recommended Q3b surface

- Short canonical Bounce: PASS with result `1331` in `rxvm` and `rxbvm`.
- Isolated `perf2_qref_guard`: PASS in both VMs. It covers owned physical-child
  fallback/lifetime, `LINKTOATTR` external mandatory fallback/lifetime, and a
  direct current-frame local hit. The owned-child case is not an A-ATTR fast hit
  because its intervening `LOAD` breaks the required adjacent pair.
- Focused CTest: 8/8 PASS: `dynamic_load`, dual-VM reference/catch, dual-VM
  signal-call-unwind and `reentrancy_check`. The re-entry test calls
  `rxvm_prepare()` before repeated/nested execution, covering prepare-only.
- Additional `source_provenance`, TRACE result/source-jump and re-entry block:
  7/7 PASS on the Q3b build.
- Q3b profile diagnostics report canonical `MKREF_REG_REG` count `3` in both
  modes. Q0 and Q3b two-instruction RXSEQ files are byte-identical in both
  modes, SHA-256
  `882e9101cab5ea2a31f2aed9e2e3b499ca79f3e0fb39aa9ce1b3e019ea643a38`.
- Q3b adds no allocation, execution-image state or teardown object. Existing
  OOM ordering remains: identity allocation succeeds before destination clear
  or lifetime marking; every guard miss calls the canonical owner/tree search.
- Separate build-private Q3b and Q4 counter runs agree in both VMs: short
  Bounce has 5,100 executions and 5,100 hits, split as 100/100 at word 791 and
  5,000/5,000 at word 844. The guard fixture has one hit and two fallbacks.
  Invalidations are exactly zero and semantically N/A for these zero-state
  forms. Raw lines and instrumentation provenance are retained in
  `correctness/q3b-q4-counter-diagnostics.md` and
  `correctness/q3b-q4-counter-runs.txt`.

## Q4 and execution observability

- The same isolated guard fixture passes in Q4 under both VMs.
- Focused reference/dynamic-load/signal/re-entry CTest: 8/8 PASS.
- Additional `source_provenance`, TRACE result/source-jump and re-entry block:
  7/7 PASS.
- Profile diagnostics report three canonical MKREF executions. Fresh Q4
  RXSEQ output is byte-identical to Q0 in both modes with the hash above.
- Normal CLI target, guard, startup and RSS processes all complete ordinary
  teardown without crash. Q4 adds no teardown allocation or state.

## Richards Q1

After building the compiler test driver, the direct/computed receiver,
copyback rewrite, mutating scalar-return, reference, vararg, writable-object,
nested-call and class-method focused selection passes 33/33. The Q1 optimized
benchmark passes with identical expected output in both accepted Q0 VMs.
This qualifies the bounded compiler-owner control; it does not authorize the
general compiler change.

## Guard diagnostics

One-work profiling runs of Sieve, Permute, Storage, Towers and Base64 pass their
correctness gates in both Q4 profiling VMs. All ten CSVs contain no
`MKREF_REG_REG` row, which is the schema's zero-execution result; benchmark plus
executed library paths therefore show no unexplained guard hit.

## Q7 boundary and retained gaps

Q7 passes short Bounce/Richards outputs, the custom reference guard and normal
process teardown, but it is not production-qualified. Its PoC leaves these
explicit gaps: overlay replay after late load, repeated `rxbvml` run/re-entry,
nested native-callback RXVML, allocation-failure injection, epoch reset/wrap,
complete dual-mode TRACE transitions and a documented concurrency/publication
contract. Ordinary successful CLI teardown does not prove leak-free embedded
lifecycle. These gaps are acceptable evidence for rejecting Q7, not for
selecting it.

A build-private transition fixture nevertheless closes the bounded event-count
and first-hit observation requirement: both VMs record one cold specialization,
one specialized fallback/dequicken and one later disabled canonical execution.
On one-repetition Richards they identically record 8 cold specializations, 62
cold disables, 32,539 fast hits and 648,355 subsequent disabled/canonical
executions. Twelve fresh processes per VM retain gross first-specialize means
of 116.319 ns/event (`rxvm`) and 18.663 ns/event (`rxbvm`); steady readings are
too close to the timer-pair floor for precise interpretation. These perturbed
diagnostics do not replace formal product timing. Q7 general invalidation and
reference dequickening are unimplemented/N/A, which remains a rejection reason.
See `correctness/q7-diagnostics.md` and the retained raw/command artifacts.

No broad full-suite closeout was run: PERF2-02 is an isolated design PoC and the
repository gate requires architecture selection before production work.
