# PERF3-13 Gate E E3b-P2 first Release verdict

Date: 2026-08-10

Branch: `develop`

Control: the exact accepted E3b-P1 ordinary-Release VMs copied before the P2
build. Their hashes match the accepted P1 closeout artifacts.

Candidate: the frozen P2 optional ABI query, per-VM plugin sessions,
procedure-bound session invoker, mixed `rxmath` policy and ODBC per-VM session
implementation.

Status: **accepted and Mac closeout complete**. Adrian accepted the guard-clean
verdict on 2026-08-10 and authorized the approved proportional closeout.

## Minimum correctness before measurement

The profiling-off Release build completed. The focused P2 panel passed 10/10,
covering static and dynamic session lifecycle, factory rollback, malformed
manifest fallback, mixed `rxmath` policy, prepared ODBC sessions, old-host
default-session compatibility and the ODBC runtime source under both concrete
VMs.

## Paired ordinary-Release result

One warmup and 12 pairwise-balanced recorded rounds ran serially for each of
12 cells. All 156/156 processes passed: 12 warmups and 144 recorded processes.
No sample was removed.

Positive elapsed percentages are adverse.

| Comparison | VM | Paired mean | Mean 95% interval | Median absolute | Result | Guard |
| --- | --- | ---: | ---: | ---: | --- | --- |
| P1 to P2 reentrant direct-path drift | `rxbvm` | -3.775905% | -7.529830% to -0.021980% | -24.5365 ms | clear favorable | clear |
| P1 to P2 reentrant direct-path drift | `rxtvm` | -2.560034% | -3.979992% to -1.140075% | -22.0670 ms | clear favorable | clear |
| P2 session-affine versus P2 direct, 20M calls | `rxbvm` | +5.310167% | +4.239307% to +6.381027% | +58.3100 ms | clear adverse diagnostic | n/a |
| P2 session-affine versus P2 direct, 20M calls | `rxtvm` | +6.398250% | +5.236356% to +7.560144% | +81.5590 ms | clear adverse diagnostic | n/a |
| P2 session versus direct one-call lifecycle | `rxbvm` | -0.166079% | -1.007978% to +0.675821% | -0.0125 ms | inconclusive | clear |
| P2 session versus direct one-call lifecycle | `rxtvm` | -0.695232% | -1.594117% to +0.203653% | -0.0930 ms | inconclusive | clear |

The direct fast path has no P2 regression; its favorable movement is treated
as code-layout observation rather than a claimed optimization. The deliberately
empty session-aware call pays about 2.92 ns per call on `rxbvm` and 4.08 ns on
`rxtvm`, including nested-call-safe session enter/leave. No acceptance threshold
was invented for that new diagnostic. For an ODBC operation the measured
3–4 ns host-side session selection cost is immaterial beside driver and database
latency. Session creation and destruction add no measurable process-lifecycle
penalty.

## Artifact guard

Each candidate VM file grows by 432 bytes (+0.0382%). Its `__text` grows by
4,092 bytes (+0.45%), while the 983,040-byte `__TEXT` segment is unchanged.
The artifact guard is clear.

## Recommendation

Adrian accepted the P2 verdict. E4, public workers/channels and Gate F remain
closed, and no push is authorized.

## Mac closeout

- The initial full normal-Debug build passed; CTest passed 2,032/2,032 in
  220.94 seconds. After enabling the installed real ODBC driver and adding its
  two runtime tests, the final full Debug rebuild passed and CTest passed
  2,034/2,034 in 225.19 seconds.
- The combined P1/P2/ODBC focused panel passed 25/25 in normal Debug, supported
  Apple AddressSanitizer and ordinary Release.
- AddressSanitizer used the project runner with `detect_leaks=0`, as required on
  Apple. The focused session/DSO teardown assertions and mock-driver unload
  check still require zero live sessions and handles.
- A Debug failure in `rxpa_dynamic_context_ownership` exposed a test-oracle
  assumption that internal length-counted VM strings had a trailing NUL. The
  runtime produced the correct 20 bytes; replacing three `strcmp` assertions
  with length-aware comparisons repaired the test. No product edit was needed.
- Post-acceptance ODBC hardening validates ordinals before narrowing and stages
  parameter replacements until `SQLBindParameter` succeeds. The regression
  injects a failed rebind, then executes through the retained prior binding
  under AddressSanitizer.
- After Adrian approved dependency installation, Homebrew unixODBC 2.3.14 and
  sqliteodbc 0.99991 supplied a real driver. CMake's generated, build-tree-only
  SQLite `:memory:` DSN exercises prepared string/integer/float/null binding,
  two active statements, reset/re-execute, fetch, rollback/commit, catalogue
  metadata and diagnostics through both concrete VMs. The final ODBC panel
  passes 6/6 in Debug, Apple AddressSanitizer and ordinary Release: two C host
  mock/session tests, two mock runtime tests and two real-driver runtime tests.
- The final ordinary-Release `rxbvm` and `rxtvm` hashes remain byte-identical to
  the accepted timing artifacts, so the retained performance verdict remains
  authoritative. `CLOSEOUT-SHA256SUMS` records the final ABI, VM integration,
  ODBC and focused-test sources without replacing the frozen timing-source
  ledger in `SOURCE-SHA256SUMS`.
- The mock remains the deterministic authority for injected failures, retained
  driver pointers, concurrent two-context overlap, teardown and old-host
  compatibility. The real driver proves actual unixODBC/SQLite integration on
  Apple ARM64. Linux, Windows and clean-runner driver qualification remains a
  publication follow-up; it is not inferred from this Mac result.
