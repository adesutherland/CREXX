# CREXX sanitizer assurance worklist

This is the canonical live register for first-party findings from maintained
sanitizer configurations.  Repository policy and closure requirements are in
[`AGENTS.md`](../AGENTS.md) and
[`CREXX_ASAN_TESTING.md`](ai-context/CREXX_ASAN_TESTING.md).

Status at 2026-08-22: repairs and current Mac qualification are green; platform
closure remains active. Consolidated current Apple-ASan passes 2,356/2,356.
Adrian approved RCC-5 publication with the missing supported Linux ASan/LSan
proof assigned to RCC-8 release QA. This phase handoff does not close the live
items below: they continue to block any cross-platform sanitizer-clean or
release-complete claim until that gate passes.

## Live items

### SAN-004 — statement-form parallel pending-result class ownership

Status: repaired with a permanent minimal regression; focused Linux Debug and
ASan/LSan qualification and the original pooled-HTTP trigger are green, with
the complete supported Linux gate in progress.

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
- Owner/next action: pass the original optimized `ts_http_pooled` build trigger
  leak-enabled, then complete the supported Linux ASan/LSan build plus CTest
  gate on the exact repaired release-QA candidate.
- Closure: retained CI and minimal unfixed LSan evidence, repaired focused
  Debug/ASan test, byte-identical optimized RXAS, the original pooled-HTTP
  build trigger, applicable task/parallel tests, and a complete supported Linux
  ASan/LSan build plus CTest gate.

### SAN-003 — inline capture symbol shape replaced without releasing ownership

Status: repaired with permanent regressions; focused Linux Debug and ASan/LSan
qualification is green, with the complete supported Linux gate in progress.

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
- Owner/next action: complete the supported Linux ASan/LSan build plus CTest
  gate on the exact release-QA candidate revision.
- Closure: retained unfixed LSan evidence, repaired focused Debug/ASan test,
  byte-identical optimized RXAS across the repair, the original `trace.crexx`
  build trigger, applicable inliner tests, and a complete supported Linux
  ASan/LSan build plus CTest gate.

### SAN-001 — RXAS SSA value pointer retained across growth

Status: repaired and accepted for RCC-5 publication; open under RCC-8 release
QA pending the complete supported Linux sanitizer gate.

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
- Owner/next action: RCC-8 release QA; complete the supported Linux ASan/LSan
  gate on the exact release-QA candidate revision.
- Closure: focused Debug/ASan regression, optimized Towers artifact, applicable
  RXAS contract tests, ordinary Release performance verdict, and complete
  Apple-ASan plus supported Linux ASan/LSan gates.

### SAN-002 — RXVM string-buffer ownership across return and reuse

Status: repaired in `f95f906de`, accepted for RCC-5 publication, and retained
with permanent regressions; open under RCC-8 release QA pending the complete
supported Linux sanitizer gate.

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
- Owner/next action: RCC-8 release QA; complete the supported Linux ASan/LSan
  gate on the exact release-QA candidate revision.
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
