# CREXX sanitizer assurance worklist

This is the canonical live register for first-party findings from maintained
sanitizer configurations.  Repository policy and closure requirements are in
[`AGENTS.md`](../AGENTS.md) and
[`CREXX_ASAN_TESTING.md`](ai-context/CREXX_ASAN_TESTING.md).

Status at 2026-08-22: repairs and current Mac qualification are green; platform
closure remains active. Consolidated current Apple-ASan passes 2,356/2,356,
but any cross-platform sanitizer-clean or release claim remains blocked until
every open item below is closed on supported Linux ASan/LSan.

## Live items

### SAN-001 — RXAS SSA value pointer retained across growth

Status: repaired in the approved worktree; open pending the complete platform
sanitizer gates.

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
- Next action: complete the supported Linux ASan/LSan gate.
- Closure: focused Debug/ASan regression, optimized Towers artifact, applicable
  RXAS contract tests, ordinary Release performance verdict, and complete
  Apple-ASan plus supported Linux ASan/LSan gates.

### SAN-002 — RXVM string-buffer ownership across return and reuse

Status: repaired in `f95f906de` with permanent regressions now retained; open
pending the complete platform sanitizer gates.

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
- Next action: complete the supported Linux ASan/LSan gate.
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
