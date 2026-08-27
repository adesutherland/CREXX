# CREXX sanitizer assurance worklist

This is the canonical live register for first-party findings from maintained
sanitizer configurations.  Repository policy and closure requirements are in
[`AGENTS.md`](../AGENTS.md) and
[`CREXX_ASAN_TESTING.md`](ai-context/CREXX_ASAN_TESTING.md).

Status at 2026-08-23: every finding in this register is repaired and the
supported Linux ASan/LSan closure gate passes 2,363/2,363 on exact commit
`e3de72939df0dacf22c0793371233ed439227437` at
`cmake-build-debugasan/asan-logs/20260823-074630-full`. GitHub Sanitizer QA run
[32623796998](https://github.com/adesutherland/CREXX/actions/runs/32623796998)
also passes both Linux x64 ASan/LSan and macOS arm64 ASan on that commit.
SAN-001 through SAN-005 and SAN-QA-001 through SAN-QA-007 are therefore closed
as sanitizer findings.

Status at 2026-08-26: SAN-QA-008 is closed after the synchronized hotfix passed
the original broad macOS arm64 trigger locally and GitHub Sanitizer QA run
[32958593912](https://github.com/adesutherland/CREXX/actions/runs/32958593912)
passed both Linux x64 ASan/LSan and macOS arm64 ASan on final published code
revision `a744b4d2551d795cf9bbf09c46c5a3fd71e53d46`. The stronger diagnostic
remains as regression evidence; no sanitizer or product failure reproduced.

A later production process-channel repair in `c87809d2b` is not a sanitizer
finding. Its exact three-test process panel passes ordinary Debug at
`cmake-build-debug/asan-logs/20260823-100116-ctest` and leak-enabled Linux
ASan/LSan at `cmake-build-debugasan/asan-logs/20260823-100525-ctest`; the full
ordinary Debug gate then passes 2,363/2,363 at
`cmake-build-debug/asan-logs/20260823-100548-full`.

Status later on 2026-08-26: SAN-006 is a final-head closure candidate. The
bounded repair and all required local macOS gates pass. It remains open and
release-blocking while GitHub runs, and becomes closed only if GitHub Sanitizer
QA and Build CREXX both pass for the exact commit containing this record. A
failure in either workflow leaves SAN-006 open; do not claim the current hotfix
or Release 1 line is sanitizer-clean before the condition is satisfied.

## Open findings

### SAN-007 — imported inline-payload AST freed during recursive function replacement

Status: closure candidate and release-blocking pending exact-SHA hosted proof.
The ownership repair and permanent regression pass normal Debug and the full
maintained Apple-ASan gate. No suppression, exclusion or waiver is authorized;
publication still requires GitHub Sanitizer QA's supported Linux ASan/LSan and
macOS ASan lanes on the exact published revision.

- Scope: compiler imported-function replacement and inline-payload attachment
  during recursive source-import validation.
- Failure: `symbol_has_initializer_definition()` reads a freed imported inline
  metadata AST node at `rxcp_val_sym.c:123`. `add_func()` frees the former
  imported function context through `freimpfc()` while the active validation
  tree still retains an initializer node allocated by
  `inline_meta_import_node()` and attached by
  `rxcp_inline_attach_imported_symbol()`.
- Original trigger: `tools/asan-run.sh --phase full --test-jobs 8
  --build-leaks off --leaks off --no-live-tail --tail-lines 50` on macOS arm64.
  The instrumented build fails while generating
  `lib/rxfnsg/rexx/httpcore.rxbin` from `httpcore.crexx`.
- Focused reproducer: `tools/asan-run.sh --phase build --build-leaks off
  --build-target lib/rxfnsg/rexx/httpcore.rxbin --build-jobs 1`. The
  corresponding normal-Debug file target is the required non-sanitized
  control. The focused runner reproduces the same allocation, free and invalid
  read stacks deterministically at
  `cmake-build-debugasan/asan-logs/20260827-155705-build/build.log`.
- Affected revision: local `hotfix` at
  `0f6481a2bdc02a9f0c78d8e47f8c253112b422a9` plus the in-progress receiver,
  cleanup and optimizer repairs under QA on 2026-08-27. The exact final repair
  revision will replace this working-tree description when committed.
- Retained log:
  `cmake-build-debugasan/asan-logs/20260827-154123-full/build.log`.
- Permanent regression: `san007_imported_inline_payload_lifetime` compiles the
  maintained `httpcore.crexx` import chain in an isolated work directory. Its
  pre-repair Apple-ASan failure is retained at
  `cmake-build-debugasan/asan-logs/20260827-160157-ctest/ctest.log`.
- Repair: context teardown now disconnects every AST-to-symbol connector while
  both sides are live, and symbol teardown clears any surviving AST
  back-pointer before freeing its connector. This repairs both destruction
  orders without retaining a dead imported context or guarding the later
  initializer query. The deterministic `httpcore` target passed through
  `tools/asan-run.sh` at
  `cmake-build-debugasan/asan-logs/20260827-160329-build/build.log`; SAN-006 and
  SAN-007 then passed together at
  `cmake-build-debugasan/asan-logs/20260827-161107-ctest/ctest.log`.
- Broad Apple-ASan evidence: `tools/asan-run.sh --phase full --test-jobs 8
  --build-leaks off --leaks off --no-live-tail --tail-lines 50` passed the
  complete instrumented build and 2,398 of 2,398 CTests in 1,028.38 seconds.
  Logs are retained under
  `cmake-build-debugasan/asan-logs/20260827-161118-full/`. Apple leak detection
  is unsupported and was disabled as required; this is address-safety evidence,
  not Linux LSan closure.
- Owner/next action: hotfix QA. Rerun the final normal Debug and Release gates
  on the frozen source, then require exact-SHA hosted Build CREXX and Sanitizer
  QA before closing this item.
- Closure checks: the permanent focused regression must pass in normal Debug
  and Apple ASan; the focused `httpcore` target must pass through the runner;
  the complete normal Debug suite and full local Apple-ASan build/CTest must
  pass; and the exact published SHA must pass GitHub Sanitizer QA's Linux x64
  ASan/LSan and macOS arm64 ASan lanes plus Build CREXX. Apple leak detection
  remains unsupported and supplies no Linux LSan closure authority.

### SAN-006 — superseded imported class context freed during recursive validation

Status: closure candidate; the imported-context ownership repair is implemented
and passes focused plus broad macOS qualification. SAN-006 remains open and
release-blocking until the exact published commit passes GitHub Sanitizer QA,
including its supported Linux x64 ASan/LSan and macOS arm64 ASan lanes, plus
Build CREXX. When both workflows pass on that commit, this conditional record
marks SAN-006 closed without weakening any sanitizer closure requirement.

- Scope: compiler symbol construction in `sym_fn()` while a Level G class or
  interface import recursively loads further class metadata.
- Failure: Apple AddressSanitizer reports a one-byte heap-use-after-free read in
  `sym_fn+0x180`. The freed allocation is reused during recursive
  `load_another_file()` / `ensure_class_imported()` processing; ten parallel
  class-library member compiles reproduce the same address-safety failure.
- Original trigger:
  `tools/asan-run.sh --phase build --build-leaks off --build-target
  cri17_attached_provider_control-rxbvml --build-jobs 10`.
- Permanent focused reproducer: CTest `san006_import_context_lifetime` stages
  `classlib_native.rxbin`, the individual `KeyDB.rxbin`, `KeyDB.crexx`, and a
  five-line Level B main source in an isolated directory, then invokes `rxc -x
  --no-exe-import` against that directory. The aggregate and individual image
  first establish a duplicate imported `KeyDB`; recursive validation then
  replaces it with the richer source contract. Removing either duplicate image
  eliminates the pre-repair failure. Ordinary Debug passed before the repair at
  `cmake-build-debug/asan-logs/20260826-203459-ctest`, while Apple-ASan failed
  with the exact `sym_fn()` use-after-free at
  `cmake-build-debugasan/asan-logs/20260826-203511-ctest`.
- Affected revision: clean code base `2aa4dd371121c5e452d7e1e6768c1f090521291f`
  plus the in-progress CRI-17 test/runtime worktree. The compiler source itself
  was not edited by CRI-17, so discovery attribution does not reduce priority.
- Original retained log:
  `cmake-build-debugasan/asan-logs/20260826-174436-build/build.log`.
- Root cause: the richer-contract path in `add_class()` preserved the stable
  imported-class registry record but immediately freed its former parsed
  `Context` and shared file-name allocation. Active recursive import/validation
  frames could still reach that context's AST `node_string` and file-name
  pointers when `sym_fn()` resumed.
- Repair: each stable imported-class record now owns a chain of superseded
  `{Context *, file_name}` pairs. Richer replacement moves the former pair to
  that chain, transfers the new `context`, `file_name`, `contract_node`,
  `contract_type`, and merged implements metadata, and frees every retained
  pair only when the registry record is destroyed. Equal or poorer duplicates
  still free only their newly parsed inactive context. The adjacent audit also
  verified stable class identity storage, implements-copy ownership, and the
  imported-function duplicate paths; no recursive import behavior is disabled.
- Local repair evidence:
  - focused ordinary Debug pass:
    `cmake-build-debug/asan-logs/20260826-203608-ctest`;
  - focused Apple-ASan pass with leak detection off:
    `cmake-build-debugasan/asan-logs/20260826-203621-ctest`;
  - original CRI-17 sanitizer build trigger pass:
    `cmake-build-debugasan/asan-logs/20260826-203628-build`;
  - related CRI-17 ordinary Debug 56/56 and Apple-ASan 4/4 passes:
    `cmake-build-debug/asan-logs/20260826-204004-ctest` and
    `cmake-build-debugasan/asan-logs/20260826-204533-ctest`;
  - complete ordinary Debug 2,391/2,391 pass:
    `cmake-build-debug/asan-logs/20260826-205944-ctest`;
  - complete Apple-ASan build and 2,391/2,391 CTest pass with build/test leak
    detection off:
    `cmake-build-debugasan/asan-logs/20260826-210616-full`.
- Conditional closure gate: GitHub Sanitizer QA and Build CREXX must both pass
  for the exact published commit containing this record. GitHub Sanitizer QA is
  the named release-QA owner for the supported Linux x64 ASan/LSan and macOS
  arm64 ASan proof. Apple LeakSanitizer is unsupported, so the local macOS
  result is address-safety evidence only and does not satisfy the Linux
  leak-closure authority. If either workflow fails or does not run, SAN-006
  remains open and release-blocking.

## Platform coverage at task close

- Apple ASan on the maintained macOS host does not provide supported leak
  detection. This is not a sanitizer-closure gap: the GitHub Linux x64
  ASan/LSan lane is the leak-closure authority, while the macOS arm64 lane
  supplies Apple-platform address-safety coverage.
- GitHub Build CREXX supplies the final-head MinSizeRel build, CTest and package
  coverage across Linux, macOS and Windows. GitHub Sanitizer QA supplies the
  final-head Linux x64 ASan/LSan and macOS arm64 ASan gates.
- SAN-006 is the only currently registered open sanitizer finding.

## Qualification infrastructure repairs

### SAN-QA-008 — saturated child redirect loses typed timeout completion

Status: closed; the exact macOS arm64 ASan failure and retained runner artifact
are identified, the failure did not repeat in focused or contended local runs,
and the local plus final-head broad platform gates pass.

- Scope: the byte-channel child-process provider's deadline and saturated
  output-redirect completion path, exercised by `rxvmchannel_byte_provider`.
- Failure: macOS arm64 Sanitizer QA run
  [32879812936](https://github.com/adesutherland/CREXX/actions/runs/32879812936)
  failed only `rxvmchannel_byte_provider` at revision
  `6fd40945d8665c5184b1044269697412310b538f`. The retained `ctest.log`
  reports `FAIL: saturated redirect preserves typed timeout completion`.
  There is no AddressSanitizer memory diagnostic in the retained log; this is
  nevertheless a first-party failure in the maintained sanitizer lane and is
  release-blocking until qualified.
- Original trigger: `tools/asan-run.sh --phase full --build-jobs 4
  --test-jobs 8 --build-leaks off --leaks off`, as recorded in the uploaded
  `sanitizer-logs-macos` artifact for the run above.
- Smallest permanent reproducer: the saturated bounded-output deadline case in
  `interpreter/tests/test_rxvmchannel_byte.c`, run with
  `tools/asan-run.sh --phase ctest --regex '^rxvmchannel_byte_provider$'
  --leaks off --test-jobs 1` after building the focused target through the
  runner. The case now reports both the returned completion state and error
  code if it fails, so a future lane failure is diagnostic rather than another
  one-bit assertion.
- Local evidence: 200 serial ordinary Debug repetitions, 200 serial macOS
  arm64 ASan repetitions and 400 macOS arm64 ASan repetitions across eight
  concurrent runner-managed CTest processes all passed on the synchronized
  hotfix source. The original broad macOS arm64 trigger then passed its build
  and all 2,378/2,378 CTests on code commit `71aa1fd15` at
  `cmake-build-debugasan/asan-logs/20260826-104630-full`; the formerly failing
  `rxvmchannel_byte_provider` passed as test 1151 under eight-way load. No
  memory-safety report or typed-state failure was reproduced.
- Closure: GitHub Sanitizer QA run
  [32958593912](https://github.com/adesutherland/CREXX/actions/runs/32958593912)
  passed both macOS arm64 ASan and Linux x64 ASan/LSan on final published code
  revision `a744b4d2551d795cf9bbf09c46c5a3fd71e53d46`, after the complete local
  macOS arm64 ASan pass above. If the test recurs, its retained state and error
  code distinguish deadline publication, child launch and redirect shutdown;
  do not weaken the assertion or timeout.
- Acceptance: retain a focused regression that fails for the original reason,
  pass the same focused shape in ordinary Debug and maintained sanitizer
  builds, rerun the original broad trigger cleanly, and pass both GitHub
  sanitizer lanes on the final published revision.

### SAN-QA-007 — RXQUEUE export variants share a temporary file

Status: closed; the exact macOS arm64 ASan failure is retained, focused
normal-Debug and Linux ASan/LSan proof is green, and the complete Linux plus
GitHub sanitizer gates pass on `e3de72939`.

- Scope: CTest scheduling for `ts_rxqueue_noopt` and `ts_rxqueue_opt` only;
  queue implementation, export/import behavior, test operations, and assertions
  are unchanged.
- Failure: macOS arm64 Sanitizer QA run
  [32618328641](https://github.com/adesutherland/CREXX/actions/runs/32618328641)
  started both variants together. They then reported missing imported or
  replaced `REPORT` entries because each uses `ts_rxqueue_export.txt` in their
  shared working directory.
- Diagnosis: the two variants are individually process-local except for that
  fixed export file. Their overlapping `EXPORT`, `IMPORT`, replacement, and
  `erasefile` operations can consume or remove the other variant's data.
- Repair: assign both tests the narrow `rxqueue_export_file` CTest resource
  lock so the variants cannot overlap while unrelated tests remain parallel.
- Focused proof: with eight CTest jobs requested, both variants pass ordinary
  Debug at `cmake-build-debug/asan-logs/20260823-065551-ctest` and leak-enabled
  Linux ASan/LSan at
  `cmake-build-debugasan/asan-logs/20260823-074553-ctest`. CTest starts the
  second variant only after the first finishes, proving the generated resource
  lock is active rather than relying on incidental timing.
- Acceptance: both variants must pass together while requesting parallel CTest
  execution in ordinary Debug and Linux ASan/LSan, followed by complete Linux
  and GitHub sanitizer gates on the committed repair.

### SAN-QA-006 — RXPA signature matrix exceeds short ASan timeout

Status: closed; focused normal-Debug and Linux ASan/LSan replays and the
complete supported Linux sanitizer gate pass on `e3de72939`.

- Scope: the CTest timeout for `rxpa_signature_diagnostics` only; compiler,
  RXPA, optimizer, diagnostic assertions, and runtime behavior are unchanged.
- Failure: the full leak-enabled Linux ASan/LSan run at
  `cmake-build-debugasan/asan-logs/20260823-043515-full` timed out the test at
  120 seconds while three long performance tests were active. It emitted no
  sanitizer diagnostic or partial assertion failure.
- Diagnosis: this is a composite contract containing two valid compiles, two
  assembles, four VM executions, and twelve expected-invalid compiler runs.
  The exact test passes alone in ordinary Debug in 16.61 seconds at
  `cmake-build-debug/asan-logs/20260823-052452-ctest` and leak-enabled Linux
  ASan/LSan in 45.02 seconds at
  `cmake-build-debugasan/asan-logs/20260823-052515-ctest`.
- Repair: use the repository's established 300-second allowance for composite
  compiler/runtime contracts. The 20 subprocesses and all output/diagnostic
  assertions remain unchanged.
- Focused proof: generated CTest metadata reports the repaired 300-second
  property in both build trees. The repaired test passes ordinary Debug in
  16.64 seconds at `cmake-build-debug/asan-logs/20260823-053553-ctest` and
  leak-enabled Linux ASan/LSan in 45.44 seconds at
  `cmake-build-debugasan/asan-logs/20260823-053616-ctest`.
- Acceptance: the focused test must pass in normal Debug and leak-enabled
  Linux ASan/LSan, followed by the complete supported Linux sanitizer gate.

### SAN-QA-005 — parser snapshot tests require full-suite isolation

Status: closed; focused normal-Debug and Linux ASan/LSan replays and the
complete supported Linux sanitizer gate pass on `e3de72939`.

- Scope: CTest scheduling for the three preprocessor syntax-highlighting tests;
  parser, preprocessor, and compiler behavior is unchanged.
- Failure: the full leak-enabled Linux ASan/LSan run at
  `cmake-build-debugasan/asan-logs/20260823-011709-full` captured only the
  asynchronous emergency parse tree for `rxpp_sh_srcmap` while CPU-heavy
  compiler and E6 measurement tests were active. There was no sanitizer
  diagnostic, crash, or nonzero `parser_tester` result.
- Repair: apply the existing `syntax_highlighting_parser` resource lock and
  `RUN_SERIAL` isolation used by compiler parser-snapshot tests to all three
  preprocessor `parser_tester` contracts.
- Focused proof: `rxpp_sh_srcmap` passes ordinary Debug in 1.86 seconds at
  `cmake-build-debug/asan-logs/20260823-024503-ctest` and leak-enabled Linux
  ASan/LSan in 4.57 seconds at
  `cmake-build-debugasan/asan-logs/20260823-024513-ctest`. After the repair,
  the complete three-test parser panel passes while requesting eight CTest
  jobs in both ordinary Debug and leak-enabled Linux ASan/LSan as part of the
  nine-test panels at `20260823-025705-ctest` and `20260823-025810-ctest`.
- Acceptance: the parser-mode tests must pass in the complete supported Linux
  sanitizer gate with their serial property present.

### SAN-QA-004 — E6 measurements contend under parallel ASan

Status: closed; the exact six-cell Linux ASan/LSan replay and the complete
supported Linux sanitizer gate pass on `e3de72939`.

- Scope: CTest scheduling for E6 performance-measurement cells only; benchmark
  workloads, VM behavior, iteration counts, and 120-second limits are
  unchanged.
- Failure: in the full leak-enabled Linux run at
  `cmake-build-debugasan/asan-logs/20260823-011709-full`, three concurrent
  one-worker E6 cells timed out at 120 seconds. A fourth completed in 103.57
  seconds, showing severe shared-host distortion.
- Diagnosis: the exact six one-worker spin/churn cells pass serially under
  leak-enabled Linux ASan/LSan in 12.68 to 40.11 seconds each, with all six
  completing in 172.31 seconds at
  `cmake-build-debugasan/asan-logs/20260823-024140-ctest`.
- Repair: mark every E6 performance-measurement cell `RUN_SERIAL`. These tests
  are intended to measure scale; overlap with another benchmark or a
  compiler-heavy test invalidates both their recorded wall time and their
  timeout contract.
- Focused proof: a combined six-cell E6 plus three-test parser panel passes
  9/9 while requesting eight CTest jobs in ordinary Debug at
  `cmake-build-debug/asan-logs/20260823-025705-ctest` and leak-enabled Linux
  ASan/LSan at `cmake-build-debugasan/asan-logs/20260823-025810-ctest`. CTest's
  generated metadata reports `RUN_SERIAL=true` for all 24 E6 cells; the ASan
  panel completed in 180.41 seconds with no overlap and no timeout change.
- Acceptance: the complete supported Linux sanitizer gate must pass without
  increasing the individual test timeout.

### SAN-QA-003 — ASan fake stack defeats POSIX doorbell slot lookup

Status: closed; focused normal-Debug and Linux ASan/LSan qualification and the
complete supported Linux sanitizer gate pass on `e3de72939`.

- Scope: the POSIX native-doorbell signal handler under address
  instrumentation; ordinary production dispatch and executor semantics are
  unchanged.
- Failure: Linux Sanitizer QA run
  [32601922923](https://github.com/adesutherland/CREXX/actions/runs/32601922923)
  timed out the base and doorbell-stress persistent-worker panels for all three
  VM libraries. A serialized local replay also remained in `loop_forever` past
  300 seconds, so scheduling and timeout changes were rejected as the fix.
- Root cause: the handler identifies its target through the address of a local
  stack marker and compares it with registered pthread stack ranges. GCC ASan
  instrumented that handler and could place the marker on its fake stack,
  outside the real pthread range; `pthread_kill()` succeeded but the handler
  silently found no slot and never published `CANCEL`.
- Repair: exclude only the bounded async-signal handler from address
  instrumentation, alongside its existing stack-protector exclusion. This
  keeps the stack probe on the real target stack and also prevents sanitizer
  runtime traversal from entering the async-signal-safe handler. The ordinary
  120-second tests and their parallel scheduling remain unchanged.
- Focused proof: all six base/doorbell-stress variants pass concurrently in
  ordinary Debug at `cmake-build-debug/asan-logs/20260823-011534-ctest` and in
  leak-enabled Linux ASan/LSan at
  `cmake-build-debugasan/asan-logs/20260823-011247-ctest`; the latter completes
  in 7.77 seconds total. Disassembly of the rebuilt ASan
  `rxbvm_core_objects` and `rxtvm_core_objects` handlers contains no ASan
  runtime reference.
- Acceptance: all six cells must pass together through leak-enabled Linux
  ASan/LSan with eight requested CTest jobs, followed by the complete supported
  Linux sanitizer gate.

### SAN-QA-002 — composite control-flow test timeout under parallel ASan

Status: closed; focused normal-Debug and Linux ASan/LSan replays and the
complete supported Linux sanitizer gate pass on `e3de72939`.

- Scope: test infrastructure only; no production compiler or runtime code.
- Failure: the leak-enabled Linux full CTest run timed out
  `do_forever_return_contract` at its fixed 120-second test limit while eight
  instrumented compiler tests were active. The same exact test passed alone
  with leak detection in 33.22 seconds and emitted no sanitizer diagnostic.
  The full-run failure and isolated replay are retained in
  `20260822-231527-ctest` and `20260822-234647-ctest` respectively.
- Repair: use the repository's established 300-second long-test allowance for
  this composite compile/assemble/two-VM/negative-diagnostic contract.
- Focused proof: the repaired test passes normal Debug in 12.24 seconds at
  `cmake-build-debug/asan-logs/20260822-234817-ctest` and leak-enabled Linux
  ASan/LSan in 33.87 seconds at
  `cmake-build-debugasan/asan-logs/20260822-234906-ctest`.
- Acceptance: the focused test must pass in normal Debug and leak-enabled
  Linux ASan/LSan, followed by the complete supported Linux sanitizer gate.

### SAN-QA-001 — ASan static control kernels require PIC

Status: closed; the exact normal-Debug and Linux ASan/LSan fixture and the
complete supported Linux sanitizer gate pass on `e3de72939`.

- Surface: the test-only `rcc5f_stats_kernel` and `rxvector01_kernel` static
  controls in `tests/performance/CMakeLists.txt`.
- Failure: the Linux sanitizer fixture could not link either static kernel
  into its direct-control shared plugin because ASan instrumentation introduced
  non-PIC `R_X86_64_PC32` relocations.  GitHub Sanitizer QA run
  [32596937877](https://github.com/adesutherland/CREXX/actions/runs/32596937877)
  and local run `cmake-build-debugasan/asan-logs/20260822-213156-full` retain
  the exact `recompile with -fPIC` diagnostics.
- Repair: declare `POSITION_INDEPENDENT_CODE ON` on both static kernel targets.
  These controls are already linked into module libraries; the property makes
  that existing target relationship valid under instrumented ELF builds and
  does not change production providers or benchmark work.
- Focused proof: the complete linked-runtime fixture passes normally in
  `cmake-build-debug/asan-logs/20260822-221819-ctest`.  The first repaired ASan
  replay rebuilt 924/1,251 invalidated artifacts before reaching the fixture's
  1,800-second CTest timeout without a link or sanitizer error; its retained
  incremental replay passes in
  `cmake-build-debugasan/asan-logs/20260822-230331-ctest`.
- Closure proof: the supported Linux ASan/LSan build plus CTest gate and
  GitHub Sanitizer QA pass on `e3de72939`.

## Closed campaign findings

### SAN-005 — linked-list test exits with owned nodes

Status: closed using the plugin's documented teardown; focused Linux Debug and
ASan/LSan qualification and the complete supported Linux gate pass on
`e3de72939`.

- Surface: the optimized and unoptimized linked-runtime variants of
  `lib/plugins/llist/llist_test.crexx`; plugin allocation and API semantics are
  unchanged.
- Failure: the exact-commit full Linux ASan/LSan run at
  `cmake-build-debugasan/asan-logs/20260823-030158-full` reports 1,230 leaked
  bytes in ten allocations for both `llist_test_noopt` and `llist_test_opt`.
- Root cause: the test creates 20 nodes, removes ten, then exits while still
  owning the other ten. The two five-object LSan groups correspond to the
  remaining prepend and append chains.
- Permanent reproducer: the two existing linked-list test variants pass
  functionally in ordinary Debug at
  `cmake-build-debug/asan-logs/20260823-042146-ctest` but reproduce the exact
  leak in isolation at
  `cmake-build-debugasan/asan-logs/20260823-043336-ctest`.
- Repair: call the documented `llist.freellist(0)` operation after the active
  queue/pull checks and before program exit. This preserves the intended ten
  live-node intermediate behavior while satisfying the caller-owned teardown
  contract.
- Focused proof: both variants pass ordinary Debug at
  `cmake-build-debug/asan-logs/20260823-043406-ctest` and leak-enabled Linux
  ASan/LSan at `cmake-build-debugasan/asan-logs/20260823-043420-ctest`.
- Acceptance: both linked-runtime variants and the complete supported Linux
  ASan/LSan build plus CTest gate must pass with leak detection enabled.

### SAN-004 — statement-form parallel pending-result class ownership

Status: closed with a permanent minimal regression; focused Linux Debug and
ASan/LSan qualification, the original pooled-HTTP trigger, and the complete
supported Linux gate pass on `e3de72939`.

- Surface: `compiler/rxcp_task_lower.c`, teardown of pending typed-object task
  results created by statement-form `DO PARALLEL` lowering.
- Failure: four direct `strdup()` leaks totalling 80 bytes while the Linux x64
  sanitizer build compiles the optimized `ts_http_pooled` images.
- Root cause: `task_pending_add()` owns copies of both `handle_name` and
  `result_class`.  Parallel block-expression teardown released both, but the
  statement-form success and failure paths released only `handle_name` before
  freeing the pending array.
- Trigger: Linux x64 Sanitizer QA run
  [32593095785](https://github.com/adesutherland/CREXX/actions/runs/32593095785)
  fails while generating the optimized `ts_http_pooled` images with
  `SUMMARY: AddressSanitizer: 80 byte(s) leaked in 4 allocation(s)`.
- Affected revision: `653c1293f1b2f84faa75e86bddd50cce6979723a`.
- Permanent reproducer:
  `compiler/tests/rexx_src/san004_task_pending_object_result.crexx` assigns two
  typed-object task results inside statement-form `DO PARALLEL`; its direct
  optimized compiler test is labelled `san-004` and `sanitizer`.
- Repair: centralize pending-result teardown in `task_pending_clear()`, release
  both owned strings and the array on every statement/expression success or
  failure path, and reset the plan's pending storage state.
- Focused proof: the unfixed normal-Debug control passes in
  `cmake-build-debug/asan-logs/20260822-210428-ctest`, while the same unfixed
  leak-enabled cell reports two direct allocations totalling 20 bytes in
  `cmake-build-debugasan/asan-logs/20260822-210434-ctest`.  The repaired cells
  pass in `20260822-210534-ctest` and `20260822-210540-ctest`.
- Original-trigger proof: the repaired leak-enabled `ts_http_pooled` target
  regenerates all four pooled-HTTP images without a sanitizer finding in
  `cmake-build-debugasan/asan-logs/20260822-210605-build`; the immediately
  repeated incremental proof passes in `20260822-213117-build`.  The related
  normal-Debug panel passes 14/14 in `20260822-211330-ctest`, covering the
  minimal compiler cell, all four pooled-HTTP runtime modes, Level-G task
  lowering, and imported typed-object task methods.
- Output proof: the optimized reproducer RXAS is byte-identical before and
  after the repair, with SHA-256
  `5d34d6c7878e073bcd1d34bb635ad2da872499079ea904a4433025db379ca2a1`.
- Closure proof: the original optimized `ts_http_pooled` trigger and the
  complete supported Linux ASan/LSan build plus CTest gate pass on
  `e3de72939`.
- Closure: retained CI and minimal unfixed LSan evidence, repaired focused
  Debug/ASan test, byte-identical optimized RXAS, the original pooled-HTTP
  build trigger, applicable task/parallel tests, and a complete supported Linux
  ASan/LSan build plus CTest gate.

### SAN-003 — inline capture symbol shape replaced without releasing ownership

Status: closed with permanent regressions; focused Linux Debug and ASan/LSan
qualification and the complete supported Linux gate pass on `e3de72939`.

- Surface: `compiler/rxcp_remap_build.c`, value-shape copying into an existing
  inline capture symbol.
- Failure: eight direct four-byte leaks from `rxcp_remap_copy_dims()` while the
  optimized compiler builds `lib/rxfnsb/rexx/trace.crexx`.
- Root cause: fixed-point inlining can reuse a deterministic temporary symbol.
  The value-shape copy helpers replace its owned dimension metadata with fresh
  allocations without first releasing the previous shape.
- Trigger: Linux x64 Sanitizer QA run
  [32585975224](https://github.com/adesutherland/CREXX/actions/runs/32585975224)
  fails during the instrumented build of `lib/rxfnsb/rexx/trace.rxbin` with
  `SUMMARY: AddressSanitizer: 32 byte(s) leaked in 8 allocation(s)`.
- Affected revision: `62c96aa4d2435f8479924393cee7922a68775985`.
- Permanent reproducer:
  `compiler/tests/rexx_src/san003_inline_scoped_array_actual.crexx`, registered
  in optimized and unoptimized runtime modes.  Optimized compilation repeatedly
  attempts a method call that passes a class-attribute `.string[]` by value,
  creates the deterministic scoped-argument capture symbol, then rejects the
  candidate at the profitability gate.  The runtime cells retain a semantic
  result check.
- Repair: allocate the complete replacement value shape first, release the
  existing symbol-owned dimensions and class only after allocation succeeds,
  then install the new shape.  This preserves the supported optimized inline
  attempt and also leaves the old shape intact if allocation fails.
- Focused proof: the unfixed optimized compile reports the same 32-byte/eight-
  allocation leak in
  `cmake-build-debugasan/asan-logs/20260822-191323-ctest/ctest.log` while the
  normal-Debug semantic controls pass in `20260822-191337-ctest`.  The repaired
  direct compile and original `trace.crexx` build trigger pass leak-enabled in
  `20260822-192120-ctest` and `20260822-192126-build`; the combined permanent
  SAN-003 and four-cell `rxfs` panel passes 7/7 in `20260822-192836-ctest`, and
  29 applicable inliner tests pass in both normal Debug and Linux ASan/LSan in
  `20260822-192909-ctest` and `20260822-193021-ctest`.  The complete ordinary
  Linux Debug build and 2,362/2,362 CTest gate pass in
  `cmake-build-debug/asan-logs/20260822-193230-full`.
- Output proof: the optimized reproducer RXAS is byte-identical before and
  after the repair, with SHA-256
  `5fda0341f2af6b01e9236235b48eff831511221a3a8b65eaf23c0284e087c096`.
- Closure proof: the complete supported Linux ASan/LSan build plus CTest gate
  passes on `e3de72939`.
- Closure: retained unfixed LSan evidence, repaired focused Debug/ASan test,
  byte-identical optimized RXAS across the repair, the original `trace.crexx`
  build trigger, applicable inliner tests, and a complete supported Linux
  ASan/LSan build plus CTest gate.

### SAN-001 — RXAS SSA value pointer retained across growth

Status: closed for sanitizer assurance; the repair remains accepted for RCC-5
publication and the complete supported Linux gate passes on `e3de72939`.

- Surface: `assembler/rxas_flow_ssa.c`, `rxas_flow_value_node()`.
- Failure: heap-use-after-free at the write to `version->source_value_id` after
  `flow_ssa_resolve_value()` can append to and reallocate `value_versions`.
- Trigger: full Apple-ASan build while generating
  `benchmark_awfy_towers_opt.rxbin`.
- Affected revision: `298f412dc0e40ef12b4957df4b5f8b57a8a14d9f` plus the
  current approved RCC-5 decimal worktree.
- Retained logs:
  `cmake-build-debugasan/asan-logs/20260820-151050-ctest/ctest.log` and
  `cmake-build-debugasan/asan-logs/20260820-205831-full/build.log`.
- Permanent reproducer: `test_ssa_value_node_growth()` in
  `tests/test_rxas_flow_graph.c` constructs exactly 64 lazy integer-copy values
  and resolves the first source across the backing-array growth boundary.  The
  unfixed normal-Debug control fails at the intended assertion in
  `cmake-build-debug/asan-logs/20260821-095107-ctest/ctest.log`, and the unfixed
  Apple-ASan control reports the original heap-use-after-free in
  `cmake-build-debugasan/asan-logs/20260821-101842-ctest/ctest.log`.
- Repair: retain the stable source value ID and source attributes, perform the
  growth-capable resolution, and reacquire `value_versions[value_id]` before
  writing or reading the record.  An audit of adjacent `FlowValueVersion *`
  accessors found no other pointer retained across an append-capable call.
- Focused proof: repaired normal Debug and Apple-ASan pass in
  `20260821-101929-ctest` and `20260821-101939-ctest`; optimized Towers,
  bytecode/semantic equivalence, and 70 applicable RXAS flow/proof contract
  tests pass in normal Debug and Apple-ASan in `20260821-102446-ctest` and
  `20260821-102451-ctest`.
- Performance: Adrian accepted the profiling-off Release verdict on
  2026-08-21.  The retained 36-pair assembly result is neutral around zero,
  clears the 3% guard, preserves exact output and bytecode, and is recorded in
  `performance/evidence/2026-08-21-san-001-ssa-release-verdict/`.
- Platform proof: the earlier complete Apple-ASan build and 2,310/2,310 CTest
  gate pass in `cmake-build-debugasan/asan-logs/20260821-112920-full`. The
  consolidated current RCC-5 build and 2,356/2,356 CTest gate also pass with no
  sanitizer report in `20260822-104056-full`; its instrumented build includes
  the original optimized Towers trigger.
- Closure proof: the supported Linux ASan/LSan gate passes on `e3de72939`.
- Closure: focused Debug/ASan regression, optimized Towers artifact, applicable
  RXAS contract tests, ordinary Release performance verdict, and complete
  Apple-ASan plus supported Linux ASan/LSan gates.

### SAN-002 — RXVM string-buffer ownership across return and reuse

Status: closed for sanitizer assurance; the `f95f906de` repair remains
accepted for RCC-5 publication, its permanent regressions remain active, and
the complete supported Linux gate passes on `e3de72939`.

- Surface: decimal-to-string/extract capacity calculation for a value produced
  under a wider numeric context and returned to a narrower caller context.
- Root cause: the former `getRequiredStringSize()` used only the active
  context's digits.  A returned 32-digit value in an 18-digit caller therefore
  received a 32-byte standard slot even though DEXTR writes a 33-byte
  coefficient including the terminator.  The current value-aware contract
  sizes from the greater of the active context and stored `decNumber` digits.
- Manifestation A: the DEXTR overrun entered an adjacent live destination slot;
  preparing that destination for `string_concat_var_const()` poisoned the slot,
  and the subsequent 33-byte source copy reported a use-after-poison read.
- Manifestation B: the same DEXTR overrun entered an adjacent slot already
  poisoned during ordinary return/value reuse and reported a use-after-poison
  write in `mc_decimal.decimalExtract()`.
- Trigger: the temporary `rcc5_allocator_diag` test used while qualifying
  RCC-5B.  Because that diagnostic was not retained, absence from later focused
  tests is not closure evidence.
- Affected revision: the context-only sizing used in the work leading to
  `f95f906de`; the value-aware repair is present in that revision and current
  HEAD.
- Retained logs:
  `cmake-build-debugasan/asan-logs/20260820-113550-ctest/ctest.log`,
  `cmake-build-debugasan/asan-logs/20260820-113825-ctest/ctest.log`, and
  `cmake-build-debugasan/asan-logs/20260820-114917-ctest/ctest.log`.
- Permanent regressions:
  `tests/sanitizers/san002_concat_after_wide_decimal.crexx` and
  `tests/sanitizers/san002_wide_decimal_extract.crexx`, registered optimized
  and unoptimized under both `rxbvm` and `rxtvm`.
- Counterfactual proof: temporarily restoring only the former context-only
  sizing makes all eight Apple-ASan cells fail.  The concat case reports the
  retained `string_concat_var_const()` use-after-poison read and the extract
  case reports the retained `decimalExtract()` use-after-poison write in
  `cmake-build-debugasan/asan-logs/20260821-110014-ctest/ctest.log`.  The
  production file was then restored byte-for-byte.
- Focused proof: final repaired normal Debug passes 8/8 in
  `cmake-build-debug/asan-logs/20260821-110131-ctest/ctest.log`; identical
  Apple-ASan passes 8/8 in
  `cmake-build-debugasan/asan-logs/20260821-110400-ctest/ctest.log`.
  The retained `ts_math_numeric` family also passes optimized/unoptimized under
  both VMs in normal Debug and Apple-ASan (`20260821-110453-ctest` and
  `20260821-110620-ctest`).
- Classification: both retained manifestations are consequences of this one
  output-capacity defect, so no separate SAN-003 is required.  No production
  edit was needed during reconstruction, and therefore no new first-Release
  performance verdict was triggered beyond the already accepted RCC-5B/C
  verdicts.
- Platform proof: the earlier complete Apple-ASan build and 2,310/2,310 CTest
  gate pass in `cmake-build-debugasan/asan-logs/20260821-112920-full`. The
  consolidated current RCC-5 build and 2,356/2,356 CTest gate also pass with no
  sanitizer report in `20260822-104056-full`, including all eight permanent
  SAN-002 cells.
- Closure proof: the supported Linux ASan/LSan gate passes on `e3de72939`.
- Closure: permanent optimized/no-opt tests under both concrete VMs, focused
  Debug/ASan checks, any required ordinary Release performance verdict, and the
  complete platform sanitizer gates.

## Closed historical findings

Historical failures remain evidence rather than live debt only when their
repair and broad closure are retained:

- the RXAS sparse proof-snapshot use-after-free was fixed in `6cde2c509` and
  followed by complete Debug/ASan 1,999/1,999;
- the Apple-ASan provider unload/reload global-buffer-overflow and allocator
  extent poison handling were fixed in `f95f906de` and their focused triggers
  passed;
- the `rxtcp.tcpreceive` 10,000-byte leak was fixed in `d78c6fcfa` and followed
  by complete Linux ASan/LSan 1,925/1,925; and
- the `keyaccess.readkey()` four-byte leak and sibling parser-tool resource
  leaks have retained leak-enabled closure documented in
  `docs/ai-context/CREXX_ASAN_TESTING.md`.

Closed entries must not be deleted when this worklist is updated.  Move each
resolved live item into this historical section with its permanent regression,
repair commit or worktree identity, exact final commands, and broad result.

## CI enforcement

`.github/workflows/sanitizers.yml` adds automatic fresh Linux x64 ASan/LSan and
macOS arm64 ASan jobs for pushes and pull requests to `develop` or `master`.
Both use `tools/asan-run.sh --phase full`, so sanitizer-instrumented build-time
execution of `rxc`, `rxas`, and `rxlink` precedes complete CTest.  The workflow
is present locally but cannot supply the required Linux closure result until it
runs from a committed revision on an available runner.  Its stable job names
must also be configured as required checks in GitHub branch protection or a
repository ruleset before merge blocking is enforced.
