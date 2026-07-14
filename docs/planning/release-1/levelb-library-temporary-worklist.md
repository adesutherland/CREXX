# High-priority Level B library temporary work list

Status: **approved; programme steps -1 and 8 complete; all 122 selector rows
processed; final integration complete; parked co-dependencies remain queued**

This is the temporary execution ledger for the high-priority Level B library
review. It copies the 87 bootstrap-core candidate selectors and the 35
standard/default selectors from the Release 1 component catalogue, preserving
their supplied order. The 122 selectors match `lib/rxfnsb/rexx/CMakeLists.txt`.

The unit of work is a **selector/source module**, not necessarily one exported
procedure. Every public callable, class, interface, and code-adjacent RexxDoc
block in the selector is in scope. A selector is not marked done until all of
its public surface has passed every applicable gate.

## Operating rule

1. Complete programme step -1 before any implementation change.
2. Work from the first non-done row downward, with only one selector actively
   being implemented at a time. A selector blocked only by a co-dependency may
   be marked `parked` with its completed gates and exact remaining dependency;
   processing then continues at the next row.
3. Before editing, record the selector's exports, dependencies, current
   signatures, error behavior, tests, documentation, and any Level C contract.
4. Finish every independently testable gate, validate the selector through an
   optimized and unoptimized staging overlay, then mark it `done` or `parked`
   before beginning the next row.
5. Shared helpers may be changed when the active selector requires them, but a
   later selector is not marked done without its own complete review.
6. Stop for approval if a function requires a language, syntax, public-contract,
   or architectural decision not already fixed by repository guidance.
7. After every selector is done, complete programme step 8 before declaring the
   programme complete.
8. Do not register a selector's tests with CMake or rebuild the aggregate Level
   B/Level C libraries during row processing. Keep the test sources, append the
   required wiring to the deferred integration queue, and wire/build everything
   together after selector processing (or at a deliberate integration
   checkpoint).

## Dependency-batched execution queue — approved 2026-07-14

The original 122-row order remains the selector-level audit trail. The 54
parked rows are now processed through the dependency batches below so that one
contract decision, shared helper, fixture set, and aggregate rebuild can serve
all affected selectors. Within a batch, selectors are still reviewed, tested,
and marked one at a time.

For every batch: first review the existing situation and present its decisions
and options for approval; then implement and validate the approved option; then
mark every completed selector before reviewing the next batch. If an approved
batch encounters a new language, syntax, public-contract, compiler, or VM
decision, park it and move to the next approved batch rather than widening the
scope implicitly.

| Batch | Parked selectors | Shared dependency | State |
|---:|---|---|---|
| 1 | `insert`, `overlay`, `lastpos`, `strip`, `substr`, `abs`, `format`, `sign`, `trunc`, `b2d`, `x2b`, `x2d` | Class adapters, focused class tests, and one classlib/library rebuild | review in progress |
| 2 | `parsecompile`, `parsestring`, `parse`, `parseExec` | Frozen legacy plan/stream ABI and assertion harnesses; no producer/lowering change | queued |
| 3 | `_datei`, `_dateo`, `_jdn`, `date`, `time` | Calendar/error contract and frozen-clause-time service | queued |
| 4 | `arrayformat`, `arraydump` | One pure formatting contract/core, followed by the output wrapper | queued |
| 5 | `delword`, `word`, `words`, `wordindex`, `subword`, `wordlength`, `wordpos` | `Config_OtherBlankCharacters` | queued |
| 6 | `translate`, `xrange`, `c2x`, `c2d`, `d2c`, `x2c` | `Config_Xrange`, `Config_C2B`, and `Config_B2C` | queued |
| 7 | `qpos`, `qsplit`, `qsplitsafe`, `qextractall`, `qextractpair`, `qstripcomment`, `qremoveall`, `qword`, `qwordlength`, `qwords`, `qwordindex`, `qwordpos`, `qsubword` | Shared quote grammar, vectors, extraction bounds, and word spans | queued |
| 8 | `fsayfmt` | Approved placeholder contract on the frozen quote grammar | queued |
| 9 | `random` | Typed Level B contract and scoped RNG service | queued |
| 10 | `value` | Configuration-named external-pool service | queued |
| 11 | `datatype` | Classic configuration services, option set, and `D` decision | queued |
| 12 | `fnv` | Public hash contract and `rxhash` VM Unicode repair | queued |

Batch 1's aggregate rebuild should also prove the already corrected class
artifacts for `xrange`, `delword`, `subword`, `wordlength`, and `wordpos` without
claiming completion of their separate configuration-service gates.

## Efficient selector validation and deferred integration

Each active selector is compiled with the existing tool binaries into an
isolated directory under `cmake-build-debug/selector-tests`. Only that selector,
its explicit dependencies, and its new harness are assembled. The harness is
run in both optimized and unoptimized modes before the V gate is checked. This
proves the selector against its new tests without causing CMake's aggregate
`library` dependency to rebuild every Level B module.

The final integration pass will add the accumulated CMake registrations, build
the combined libraries once, run the queued tests, and perform the full test
sweep. A parked selector must name the co-dependency that remains and retain
its focused validation evidence.

### Deferred integration wiring queue — applied 2026-07-14

All entries below were applied in the final integration checkpoint. They are
retained as the audit trail rather than deleted from this temporary ledger.

Integration evidence:

- the combined Debug build completed after stale generated aggregate images
  were cleared;
- the focused Level B/standalone Level C integration set passed 183/183 tests;
- the eight compiler RXAS/AST expectations affected only by reviewed Level B
  source bodies or source locations were refreshed and pass 8/8; no compiler
  or interpreter implementation source was changed;
- the remaining DATE pair exposed a hidden dependency on the old WORDPOS
  prefix-matching bug. DATE now owns its month-prefix mapping, while WORDPOS
  remains exact; the enhanced `ts_date2` pair passes in both modes;
- the final full Debug sweep passes 1,794/1,794 tests. An earlier full run had
  three transient syntax-highlighting parser-thread misses; all three passed
  immediately when rerun serially, and the unchanged clean full rerun passed;
- `git diff --check` passes;
- the 2026-07-14 VM dependency checkpoint restores compiler call-window
  mappings only on exceptional branch unwind, maps decimal conversion syntax
  to catchable `CONVERSION_ERROR`, and passes the focused 63-test VM/Level B
  gate. This closes the shared ABI blocker for 18 selectors and the decimal
  signal gate for six numeric selectors without changing compiler lowering;
- step 8 reran the five coarse Release workloads with the baseline settings;
  every median improved, with changes from -27.38% to -38.41%. Detailed
  results are in
  `levelb-library-benchmark-baseline-2026-07-13.md`.

- Level B: register `ts_address_protocol` optimized/unoptimized.
- Level B: register `ts_rxsystem` optimized/unoptimized, passing the CTest test
  name argument.
- Level B: register `ts_rxsystem_exit` optimized/unoptimized and require exact
  process exit code 7.
- Level B: register `ts_loadmodule` optimized/unoptimized and build
  `ts_loadmodule_provider.rxbin` as its focused runtime fixture.
- Level B: register `ts_raise` optimized/unoptimized.
- Level B: register `ts_signal` optimized/unoptimized.
- Level B: register `ts_symbol` optimized/unoptimized.
- Level C: register `testRexxClassicBifAddress` optimized/unoptimized.
- Level C: add `RexxClassicBifSymbol` to the consolidated rxfnsc library and
  register `testRexxClassicBifSymbol` optimized/unoptimized.
- Level C: add `RexxClassicBifTrace` to the consolidated rxfnsc library and
  register `testRexxClassicBifTrace` optimized/unoptimized.
- Level B: register `ts_value` optimized/unoptimized.
- Level C: add `RexxClassicBifValue` to the consolidated rxfnsc library and
  register `testRexxClassicBifValue` optimized/unoptimized.
- Level B: register `ts_version` optimized/unoptimized.
- Level B: register `ts_abbrev` optimized/unoptimized.
- Level C: add `RexxClassicBifAbbrev` to the consolidated rxfnsc library,
  register `testRexxClassicBifAbbrev` optimized/unoptimized, switch the
  RexxScript and broad direct-test ABBREV branches to the standalone import,
  and then remove the legacy ABBREV export from `RexxClassicBifs` while keeping
  its internal deprecated dispatcher body until compiler lowering changes.
- Level B: register `ts_center` optimized/unoptimized.
- Level C: add `RexxClassicBifCenter` to the consolidated rxfnsc library and
  register `testRexxClassicBifCenter` optimized/unoptimized.
- Level B: register `ts_centre` optimized/unoptimized.
- Level C: register `testRexxClassicBifCentre` optimized/unoptimized against
  the CENTRE direct alias exported by `RexxClassicBifCenter`.
- Level B: register the enhanced `ts_changestr` optimized/unoptimized.
- Level C: add `RexxClassicBifChangestr` to the consolidated rxfnsc library and
  register `testRexxClassicBifChangestr` optimized/unoptimized.
- Level B: register the enhanced `ts_compare` optimized/unoptimized.
- Level C: add `RexxClassicBifCompare` to the consolidated rxfnsc library and
  register `testRexxClassicBifCompare` optimized/unoptimized.
- Level B: register the enhanced `tscopies` optimized/unoptimized.
- Level C: add `RexxClassicBifCopies` to the consolidated rxfnsc library,
  register `testRexxClassicBifCopies` optimized/unoptimized, switch RexxScript
  and the broad direct harness to the standalone import, then remove the legacy
  COPIES export from `RexxClassicBifs` while retaining its internal deprecated
  dispatcher body until compiler lowering changes.
- Level B: register the enhanced `ts_countstr` optimized/unoptimized.
- Level C: add `RexxClassicBifCountstr` to the consolidated rxfnsc library and
  register `testRexxClassicBifCountstr` optimized/unoptimized.
- Level B: register the enhanced `ts_delstr` optimized/unoptimized.
- Level C: add `RexxClassicBifDelstr` to the consolidated rxfnsc library and
  register `testRexxClassicBifDelstr` optimized/unoptimized.
- Level B: register the enhanced `ts_insert` optimized/unoptimized.
- Level C: add `RexxClassicBifInsert` to the consolidated rxfnsc library and
  register `testRexxClassicBifInsert` optimized/unoptimized.
- Level B: register the enhanced `ts_length` optimized/unoptimized.
- Level C: add `RexxClassicBifLength` to the consolidated rxfnsc library,
  register `testRexxClassicBifLength` optimized/unoptimized, and switch the
  RexxScript LENGTH branch plus broad context test to the standalone qualified
  entry. Retain the deprecated value-only helper in `RexxClassicBifs` for
  current compiler-generated artifacts until the later lowering change.
- Level B: register the enhanced `ts_lower` optimized/unoptimized.
- Level B: register the enhanced `ts_overlay` optimized/unoptimized.
- Level C: add `RexxClassicBifOverlay` to the consolidated rxfnsc library and
  register `testRexxClassicBifOverlay` optimized/unoptimized.
- Level B: register the enhanced `ts_pos` optimized/unoptimized.
- Level C: add `RexxClassicBifPos` to the consolidated rxfnsc library,
  register `testRexxClassicBifPos` optimized/unoptimized, switch RexxScript and
  the broad direct harness to the standalone qualified entry, then remove the
  legacy POS export from `RexxClassicBifs` while retaining its deprecated body
  for the compatibility controller until lowering changes.
- Level B: register the enhanced `tslastpos` optimized/unoptimized.
- Level C: add `RexxClassicBifLastpos` to the consolidated rxfnsc library and
  register `testRexxClassicBifLastpos` optimized/unoptimized.
- Level B: register the enhanced `ts_left` optimized/unoptimized.
- Level C: add `RexxClassicBifLeft` to the consolidated rxfnsc library,
  register `testRexxClassicBifLeft` optimized/unoptimized, switch RexxScript and
  the broad direct harness to the standalone qualified entry, then remove the
  legacy LEFT export while retaining its deprecated controller body.
- Level B: register the enhanced `ts_right` optimized/unoptimized.
- Level C: add `RexxClassicBifRight` to the consolidated rxfnsc library,
  register `testRexxClassicBifRight` optimized/unoptimized, switch RexxScript
  and the broad direct harness to the standalone qualified entry, then remove
  the legacy RIGHT export while retaining its deprecated controller body.
- Level B: register the enhanced `ts_reverse` optimized/unoptimized.
- Level C: add `RexxClassicBifReverse` to the consolidated rxfnsc library and
  register `testRexxClassicBifReverse` optimized/unoptimized. No deprecated
  common-controller REVERSE body exists to migrate or retain.
- Level B: register the enhanced `ts_space` optimized/unoptimized.
- Level C: add `RexxClassicBifSpace` to the consolidated rxfnsc library,
  register `testRexxClassicBifSpace` optimized/unoptimized, switch RexxScript
  and the broad direct harness to the standalone qualified entry, then remove
  the legacy SPACE export while retaining its deprecated controller body.
- Level B: register the enhanced `ts_strip` optimized/unoptimized.
- Level C: add `RexxClassicBifStrip` to the consolidated rxfnsc library,
  register `testRexxClassicBifStrip` optimized/unoptimized, switch RexxScript
  and the broad direct harness to the standalone qualified entry, then remove
  the legacy STRIP export while retaining its deprecated controller body.
- Level B: register the enhanced `tsubstr` optimized/unoptimized.
- Level C: add `RexxClassicBifSubstr` to the consolidated rxfnsc library,
  register `testRexxClassicBifSubstr` optimized/unoptimized, switch RexxScript
  and the broad direct harness to the standalone qualified entry, then remove
  the legacy SUBSTR export while retaining its deprecated controller body.
- Level B: register new `ts_substro` optimized/unoptimized.
- Level B: retain the existing `ts_upper` registration and ensure the enhanced
  harness is run optimized/unoptimized in the final aggregate sweep.
- Level B: retain the existing `ts_verify` registration and ensure the enhanced
  harness is run optimized/unoptimized in the final aggregate sweep.
- Level C: add `RexxClassicBifVerify` to the consolidated rxfnsc library,
  register `testRexxClassicBifVerify` optimized/unoptimized, switch RexxScript
  and the broad direct harness to the standalone qualified entry, then remove
  the legacy VERIFY export while retaining its deprecated controller body until
  compiler lowering changes.
- Level B: register new `ts_ftrunc` optimized/unoptimized.
- Level B: register new `ts_itrunc` optimized/unoptimized.
- Level B: retain the existing `ts_abs` registration and run the enhanced typed-
  core harness optimized/unoptimized, including the dynamic invalid-decimal
  signal case enabled by the shared VM conversion repair.
- Level C: add `RexxClassicBifAbs` to the consolidated rxfnsc library, register
  `testRexxClassicBifAbs` optimized/unoptimized, switch RexxScript and the broad
  direct harness to the standalone qualified entry, then remove the legacy ABS
  export while retaining its deprecated controller body until compiler lowering
  changes. Retain the shared Classic NUM leading-sign normalization.
- Level B: register new `ts_max` optimized/unoptimized, including the dynamic
  decimal-conversion signal path enabled by the shared VM repair.
- Level C: add `RexxClassicBifMax` to the consolidated rxfnsc library, retain
  the shared variadic CheckArgs export, register `testRexxClassicBifMax`
  optimized/unoptimized, switch RexxScript and the broad direct harness to the
  standalone qualified entry, then remove the legacy MAX export while retaining
  its deprecated controller body until compiler lowering changes.
- Level B: register new `ts_min` optimized/unoptimized, including the dynamic
  decimal-conversion signal path enabled by the shared VM repair.
- Level C: add `RexxClassicBifMin` to the consolidated rxfnsc library, register
  `testRexxClassicBifMin` optimized/unoptimized, switch RexxScript and the broad
  direct harness to the standalone qualified entry, then remove the legacy MIN
  export while retaining its deprecated controller body until compiler lowering
  changes.
- Level B: retain the existing `ts_numeric` registration and run the enhanced
  all-accessor harness optimized/unoptimized in the final aggregate sweep.
- Level C: add `RexxClassicBifNumeric` to the consolidated rxfnsc library and
  register `testRexxClassicBifNumeric` optimized/unoptimized. Wire the later
  direct lowering to the three qualified entries without adding them to the
  deprecated name controller.
- Level B: retain the `ts_sign` registration and run the simplified decimal-core
  harness optimized/unoptimized, including dynamic invalid-decimal signalling.
- Level C: add `RexxClassicBifSign` to the consolidated rxfnsc library, register
  `testRexxClassicBifSign` optimized/unoptimized, switch RexxScript and the broad
  direct harness to the qualified entry, then remove the legacy SIGN export
  while retaining its deprecated controller body until compiler lowering
  changes.
- Level B: retain the `ts_trunc` registration and run the simplified decimal-
  core/signal harness optimized/unoptimized, including dynamic invalid-decimal
  coverage.
- Level C: add `RexxClassicBifTrunc` to the consolidated rxfnsc library and
  register `testRexxClassicBifTrunc` optimized/unoptimized. Add RexxScript and
  broad-harness direct wiring without introducing a compatibility-controller
  body that does not currently exist.
- Level B: retain the `ts_format` registration and run the rewritten normative
  harness optimized/unoptimized, including dynamic invalid-decimal coverage.
- Level C: add `RexxClassicBifFormat` to the consolidated rxfnsc library and
  register `testRexxClassicBifFormat` optimized/unoptimized. Add RexxScript and
  broad-harness direct wiring without adding a compatibility-controller body.
- Level B: register new focused `ts_b2x` optimized/unoptimized; retain X2B-only
  coverage in `ts_x2b` after removing its contradictory B2X assertions.
- Level C: retain the shared `BIN` CheckArgs rule, add `RexxClassicBifB2x` to
  the consolidated rxfnsc library, and register `testRexxClassicBifB2x`
  optimized/unoptimized. Add direct RexxScript/broad-harness wiring without a
  compatibility-controller body.
- Level B: register new `ts_b2d` optimized/unoptimized. Rebuild classlib once at
  the final integration checkpoint so `.Rexx.b2d` advertises/returns `.int`,
  then add the focused adapter assertion against the rebuilt metadata.
- Level B: retain the existing `ts_binary` optimized/unoptimized registration
  and run the expanded whole-module harness in the final aggregate sweep.
- Level B: retain the existing `ts_c2x` registration and run the rewritten
  focused harness optimized/unoptimized in the final aggregate sweep.
- Level B: register new focused `ts_c2d` optimized/unoptimized.
- Level B: register new focused `ts_d2b` optimized/unoptimized.
- Level B: retain the existing `ts_d2c` registration and run the rewritten
  standalone harness optimized/unoptimized in the final aggregate sweep.
- Level B: retain the existing `ts_d2x` registration and run the rewritten
  focused harness optimized/unoptimized in the final aggregate sweep.
- Level C: add `RexxClassicBifD2x` to the consolidated rxfnsc library, retain
  the shared `WHOLENUM`/`WHOLENUM>=0` CheckArgs rules, and register
  `testRexxClassicBifD2x` optimized/unoptimized. Add direct RexxScript/broad-
  harness wiring later without adding a compatibility-controller body.
- Level B: retain the existing `ts_x2b` registration and run the rewritten
  focused harness optimized/unoptimized in the final aggregate sweep. Rebuild
  classlib once so `.Rexx.x2b()` uses the corrected one-argument native API.
- Level C: retain the shared `HEX` CheckArgs rule, add `RexxClassicBifX2b` to
  the consolidated rxfnsc library, and register `testRexxClassicBifX2b`
  optimized/unoptimized. Add direct RexxScript/broad-harness wiring later
  without adding a compatibility-controller body.
- Level B: retain the existing `ts_x2c` registration and run the rewritten
  standalone harness optimized/unoptimized in the final aggregate sweep.
- Level B: retain the existing `ts_x2d` registration and run the rewritten
  focused harness optimized/unoptimized in the final aggregate sweep. Rebuild
  classlib once to prove the presence-aware `.Rexx.x2d([length])` adapter.
- Level C: retain shared HEX validation, add `RexxClassicBifX2d` to the
  consolidated rxfnsc library, and register `testRexxClassicBifX2d`
  optimized/unoptimized. Add direct RexxScript/broad-harness wiring later
  without adding a compatibility-controller body.
- Level B: register new focused `ts_xrange` optimized/unoptimized and rebuild
  classlib once to prove `.Rexx.xrange(final)` calls the corrected native
  helper without printing or returning a deprecation message.
- Level B: register new focused `ts_arrayfind` optimized/unoptimized.
- Level B: retain the existing `ts_splice` registration and run the rewritten
  focused harness optimized/unoptimized in the final aggregate sweep.
- Level B: register new focused `ts_arrayinsert` optimized/unoptimized and run
  the updated broad `ts_arraydelete` harness after the aggregate rebuild.
- Level B: register new focused `ts_arraydelete_focused` optimized/unoptimized;
  retain and run the updated broad `ts_arraydelete` harness at final integration.
- Level B: register new focused `ts_arrayappend` optimized/unoptimized.
- Level B: register new focused `ts_arrayprepend` optimized/unoptimized.
- Level B: register new focused `ts_objectarrayinsert` optimized/unoptimized.
- Level B: register new focused `ts_objectarraydelete` optimized/unoptimized.
- Level B: register new focused `ts_objectarrayappend` optimized/unoptimized.
- Level B: register new focused `ts_objectarrayprepend` optimized/unoptimized.
- Level B: register new focused `ts_objectarraydrop` optimized/unoptimized.
- Level B: register new focused `ts_objectarraymove` optimized/unoptimized.
- Level B: register new focused `ts_arrayget` optimized/unoptimized.
- Level B: register new focused `ts_arrayset` optimized/unoptimized.
- Level B: register new focused `ts_arraycontains` optimized/unoptimized.
- Level B: register new focused `ts_arrayindexof` optimized/unoptimized.
- Level B: register new focused `ts_arraycopy` optimized/unoptimized.
- Level B: register new focused `ts_arraydrop` optimized/unoptimized.
- Level B: register new focused `ts_arrayhi` optimized/unoptimized.
- Level B: register new focused `ts_arraymove` optimized/unoptimized.
- Level B: retain enhanced `ts_stem` optimized/unoptimized and register the
  parameterized `ts_stem_errors` artifact as four fresh-VM cases (`key`,
  `value`, `valueat`, and `iterator`) in each mode.
- Level B: retain the rewritten `ts_delword` optimized/unoptimized and rebuild
  classlib once to prove the presence-aware `.Rexx.delword(start [,count])`
  adapter against the corrected native contract.
- Level C: add `RexxClassicBifDelword` to the consolidated rxfnsc library and
  register `testRexxClassicBifDelword` optimized/unoptimized. Add direct
  RexxScript/broad-harness wiring later without changing compiler lowering or
  adding a compatibility-controller route.
- Level B: retain the rewritten `tsword` optimized/unoptimized registration.
- Level C: add `RexxClassicBifWord` to the consolidated rxfnsc library and
  register `testRexxClassicBifWord` optimized/unoptimized. Switch RexxScript
  and the broad direct harness to the standalone qualified entry during final
  wiring; keep the deprecated common body for existing generated artifacts
  until the later lowering change.
- Level B: retain the rewritten `ts_words` optimized/unoptimized registration.
- Level C: add `RexxClassicBifWords` to the consolidated rxfnsc library and
  register `testRexxClassicBifWords` optimized/unoptimized. Switch RexxScript
  and the broad direct harness to the standalone qualified entry during final
  wiring; retain the deprecated common body until compiler lowering changes.
- Level B: retain the rewritten `ts_wrdix` optimized/unoptimized registration.
- Level C: add `RexxClassicBifWordindex` to the consolidated rxfnsc library and
  register `testRexxClassicBifWordindex` optimized/unoptimized. Add direct
  RexxScript/broad-harness wiring later without adding a controller body or
  changing compiler lowering.
- Level B: register new `ts_subword` optimized/unoptimized and rebuild classlib
  once to prove the corrected presence-aware `.Rexx.subword(start [,count])`
  adapter.
- Level C: add `RexxClassicBifSubword` to the consolidated rxfnsc library and
  register `testRexxClassicBifSubword` optimized/unoptimized. Add direct
  RexxScript/broad-harness wiring later without a controller or lowering change.
- Level B: retain the rewritten `ts_wordlength` optimized/unoptimized; rebuild
  library/classlib once so aggregate metadata and `.Rexx.wordlength` consume the
  corrected `.int` return, then run the adapter assertion.
- Level C: add `RexxClassicBifWordlength` to the consolidated rxfnsc library and
  register `testRexxClassicBifWordlength` optimized/unoptimized. Add direct
  RexxScript/broad-harness wiring later without a controller or lowering change.
- Level B: retain the rewritten `ts_wordpos` optimized/unoptimized and rebuild
  classlib once to prove corrected `.Rexx.wordpos(phrase [,start])` forwarding.
- Level C: add `RexxClassicBifWordpos` to the consolidated rxfnsc library and
  register `testRexxClassicBifWordpos` optimized/unoptimized. Add direct
  RexxScript/broad-harness wiring later without a controller or lowering change.
- Level B: register `ts_getenv` optimized/unoptimized and inject
  `CREXX_GETENV_SELECTOR_TEST_VALUE=alpha beta` into both test processes.
- Level B: retain the enhanced `tlinesz` optimized/unoptimized registration.
- Level B: retain the enhanced `ts_filter` optimized/unoptimized registration.
- Level B: retain the enhanced `ts_sequence` optimized/unoptimized and, after
  the one aggregate/classlib build, run the `.Rexx.sequence()` examples against
  the same corrected selector.
- Level B: retain the rewritten `ts_find` optimized/unoptimized against the
  reviewed `wordpos` dependency.
- Level B: retain the rewritten `ts_index` optimized/unoptimized.
- Level B: retain the rewritten `ts_reradix` optimized/unoptimized and run the
  `.Rexx.reradix()` forwarding examples after the final classlib build.
- Level B: retain the rewritten `ts_time` optimized/unoptimized with the active
  RXAS `_elapsed` dependency.
- Level B: register `ts_arraypop` optimized/unoptimized.
- Level B: register `ts_arrayshift` optimized/unoptimized.
- Level B: register `ts_arrayreverse` optimized/unoptimized.
- Level B: register `ts_arrayjoin` optimized/unoptimized.
- Level B: register `ts_arraysort` optimized/unoptimized and the parameterized
  `ts_arraysort_errors` artifact as three fresh-VM cases (`offset`, `order`, and
  `debug`) in each mode.
- Level B: register `ts_qwordlength` optimized/unoptimized after the one final
  aggregate rebuild updates QWORDLENGTH's return metadata from `.string` to
  `.int`; link the reviewed WORD dependency used by QWORD's quote-free path.

### Selector inventory and evidence — row 45 `binary`

- Public surface: 22 Level B procedures in `lib/rxfnsb/rexx/binary.crexx`:
  the 1-based copy/value helpers `binlength`, `binbyte`, `binsetbyte`,
  `binsubstr`, `binconcat`, `binoverlay`, `bininsert`, `bindelstr`, `binpos`,
  `bincompare`, `bin2x`, and `x2bin`; and the zero-based exposed packed-memory
  mutators `binresize`, `binclear`, `binfill`, `binfillat`, `bincopy`,
  `binmemmove`, `binappend`, `binupdate`, `binmakegap`, and `bindrop`.
- Types: binary payloads are `.binary`; byte positions, offsets, lengths, and
  byte values are `.int`; `bin2x`/`x2bin` are the only text boundary. Return
  types already match the documented surface. There is no matching Level C
  BIF contract in the repository recognition ledger, so no `rxfnsc` split is
  applicable.
- Dependencies: only Level B/VM binary instructions plus internal calls within
  this module. No compiler, classlib, Level C support, or aggregate-library
  dependency is required for focused validation.
- Current errors: VM writes/resizes/fills already raise `OUT_OF_RANGE`; packed
  span helpers add explicit `OUT_OF_RANGE` checks. `x2bin` silently returns an
  empty value for invalid hex and must instead signal `INVALID_ARGUMENTS`.
  Tests currently cover only two packed-range errors and must exercise every
  documented error family.
- Performance baseline: the original module emits 1,914 unoptimized and 2,648
  optimized RXAS lines. Unoptimized `binpos` calls `binbyte` twice per compared
  byte; `bincompare` does the same, and `binmemmove` allocates/copies a temporary
  chunk although RXAS supplies zero-copy compare and overlap-safe move
  instructions. Replace the hot call loops and temporary move while retaining
  selector-local compilation.
- Existing tests/docs: `ts_binary.crexx` is already registered in both modes
  and covers the main successful examples. The book BIF and binary-memory pages
  document both sub-surfaces, but there is no stable module-local Level B page
  and the current X2BIN invalid-input documentation contradicts the programme's
  signal rule. Add the module page, source RexxDoc, documented boundary cases,
  mutation-alias checks, and complete signal coverage.
- Completed library result: all 22 procedures retain typed binary/integer
  boundaries; invalid X2BIN text and invalid 1-based starts now signal; span
  arithmetic avoids overflow-prone end calculations; `binpos` uses `bcmpb`;
  `bincompare` has an equal-length zero-copy fast path and direct byte scan;
  `bincopy`/`binmemmove` use checked native moves; the structural helpers avoid
  internal Level B calls and temporary chunks.
- Focused validation: the expanded harness prints `PASS: binary helpers` in
  optimized and unoptimized isolated overlays. Current module RXAS is 2,055
  lines unoptimized and 2,051 optimized, with zero `call` instructions, two
  `bcmpb`, one `bmove`, and five `bmemmove` instructions in each. The original
  was 1,914/2,648 lines and the unoptimized hot loops called `binbyte` twice per
  compared byte. `git diff --check` passes and compiler/interpreter diffs remain
  empty.

### Selector inventory and evidence — row 46 `c2x`

- Public surface: one Level B `c2x(from = .string) -> .string` procedure and
  the applicable recognized Level C `C2X` BIF. There is no current standalone
  or deprecated-controller C2X body. `.Rexx.c2x()` is an unchanged classlib
  adapter over the Level B function.
- Contract: empty and multi-character strings are valid and each character
  currently contributes exactly two uppercase digits through RXAS `hexchar`.
  Argument count/omission are the only direct Level C errors (`40.3`, `40.4`,
  `40.5`). The repository's Unicode research fixture identifies UTF-8-byte
  output as a separate incompatible variant; preserve the established low-byte
  behavior during this row and park the wider Unicode representation decision.
- Current implementation/tests: the Level B loop repeatedly concatenates its
  growing result and has only a legacy no-pass-marker harness. Replace that
  append pattern, preserve typed input, cover empty/control/multi-character and
  established Unicode behavior, and add the direct `RexxValue` harness.
- Documentation: the book incorrectly restricts C2X to exactly one character,
  says leading zeroes are removed, and contains EBCDIC-only examples. Add
  separate stable Level B and Level C Markdown contracts and correct the book
  to the actual current surface without deciding the parked encoding change.
- Completed Level B result: the implementation scans once and uses
  capacity-aware `append` instead of repeatedly concatenating the growing
  result. The rewritten harness prints its PASS marker in optimized and
  unoptimized overlays, covering empty, one-/multi-character, leading-zero,
  long, non-mutation, and established Unicode behavior.
- Second review: Level B RXAS fell from 91 to 78 lines in both modes and changed
  from one loop `concat` to one loop `append`, with no calls. Separate B/C
  Markdown and both affected book sections now distinguish the tested native
  behavior from the configuration-coded Level C contract; `git diff --check`
  passes and compiler/interpreter diffs are empty.

### Selector inventory and evidence — row 47 `c2d`

- Public Level B surface: `c2d(from = .string) -> .int`, a deliberately typed
  single-Unicode-code-point helper used by `d2c` round trips and text parsing.
  It already scans length once, extracts once, and signals `CONVERSION_ERROR`
  unless the input contains exactly one code point; only authoring clarity,
  dedicated coverage, and stable Level B documentation are missing.
- Level C contract is different: repository guidance specifies
  `C2D(string [,length])`, CheckArgs `rANY oWHOLE>=0`, configuration-coded
  characters, rightmost-length signed twos-complement interpretation, caller
  `NUMERIC DIGITS`, and `40.35` overflow. It depends on the not-yet-defined
  `Config_C2B` coded-character service and must not be approximated by the
  one-character Level B helper.
- Current tests/docs: C2D is observed only inside the legacy `ts_d2c` harness;
  add a focused Level B PASS-marker harness for ASCII, control, Unicode,
  non-mutation, and both invalid-length signals. Correct the book's return-type
  wording, add the stable Level B page, and add a separate Level C contract page
  that records the configuration dependency without claiming implementation.
- Completed Level B result: source now has canonical RexxDoc/typed locals and
  removes the unreachable post-signal return while retaining the direct
  one-length/one-extraction algorithm. The new focused harness prints
  `PASS: c2d` in optimized and unoptimized overlays for ASCII, two-/four-byte
  Unicode, NUL/control code points, non-mutation, and both invalid lengths.
- Second review: RXAS fell from 65 to 59 lines in both modes, with exactly one
  `strlen`, one `strchar`, and no calls. Separate Level B and Level C Markdown
  plus both book sections now distinguish native integer code-point C2D from
  configuration-coded Classic C2D. `git diff --check` passes and compiler/
  interpreter diffs remain empty.

### Selector inventory and evidence — row 48 `d2b`

- Public surface: one native-only `d2b(dec = .int) -> .string`; there is no
  Level C recognition-ledger entry. `.Rexx.d2b()` is an unchanged string-object
  adapter over this one-argument Level B API.
- Current implementation delegates through the unreviewed later `d2x` and
  `x2b` selectors, inheriting their 16-bit negative-number behavior, leading-
  nibble rules, allocations, diagnostics, and two Level B calls. Replace the
  chain with a direct native-integer bit emitter so this row can be validated
  independently.
- Current contract/docs/tests: the only detailed class book text describes an
  optional width that neither the source function nor class method accepts. The
  actual one-argument API should accept non-negative integers, return minimal
  binary text (`"0"` for zero), and signal `INVALID_ARGUMENTS` for a negative
  input because no signed width exists. There is no dedicated test or stable
  module page; add both and correct the class book without changing classlib.
- Completed result: D2B now counts significant bits with logical shifts and
  emits them directly from most significant to least significant. It has no
  selector dependency or intermediate hexadecimal allocation, returns `"0"`
  for zero, and signals `INVALID_ARGUMENTS` for negative native integers.
- Test/doc result: the new focused harness covers the documented conversions,
  source non-mutation, the exact 63-bit `INT64_MAX` boundary, and the negative
  signal. A stable Level B page and corrected class documentation describe the
  actual one-argument native API and no longer claim an optional signed width.
- Focused validation and second review: optimized and unoptimized isolated
  overlays both report `PASS: d2b`. Original output was 59 noopt lines with two
  selector calls (302 opt lines after inlining); the direct implementation is
  137 noopt / 129 opt lines with zero calls, two shift sites, one mask site, and
  one capacity-aware character append. `git diff --check` passes and compiler/
  interpreter diffs remain empty.

### Selector inventory and evidence — row 49 `d2c`

- Public Level B surface: `d2c(from=.int, [slen=.int]) -> .string` is a native
  Unicode-code-point helper. Omitted length and length one emit one character;
  length zero returns empty; any other length or any encoded invalid Unicode
  scalar signals `CONVERSION_ERROR`. The emitted metadata already types both
  arguments as `.int`, despite the optional length being inferred from its
  integer default.
- Current Level B algorithm: direct constant-time validation followed by one
  `appendchar`; it has no selector dependency or intermediate conversion. The
  original isolated output is 105 noopt / 99 opt lines, with zero calls and one
  append. Retain that algorithm, add canonical RexxDoc/naming, and remove only
  unreachable post-signal returns.
- Test/doc finding: the registered legacy `ts_d2c` mixes D2C and C2D, has no
  PASS marker, and omits every signal and Unicode-boundary case. There is no
  selector-local Level B page. The language book conflates the native Unicode
  helper with Classic configuration-coded D2C, while the class book claims an
  optional `.Rexx.d2c` length that its method does not accept. Split and correct
  those contracts, and make the Level B harness standalone.
- Level C contract: `D2C(number [,length])` is recognized with CheckArgs
  `rWHOLENUM>=0`, or `rWHOLENUM rWHOLE>=0`. Unlike Level B, it accepts negative
  numbers when length is present, uses twos-complement, and pads/truncates in
  units of configuration-coded characters. A direct RexxValue implementation
  cannot be completed until the repository defines the shared `Config_B2C`
  bits-to-coded-character service; do not substitute Unicode `appendchar`.
- Completed Level B result: the signature now uses descriptive typed metadata
  `codepoint=.int, ?output_length=.int` and tests optional presence rather than
  accepting the old explicit `-1` omission sentinel. The constant-time scalar
  checks and single native `appendchar` remain; invalid explicit lengths and
  invalid Unicode scalars raise `CONVERSION_ERROR` without unreachable returns.
- Test/doc result: the rewritten standalone harness covers omitted/one/zero
  length, the zero-length no-encoding path, NUL/control/ASCII/two-/four-byte
  Unicode, both surrogate boundaries, `U+10FFFF`, source non-mutation, and all
  invalid length/scalar families. Separate stable B/C pages and corrected
  language/class book sections distinguish native Unicode D2C from Classic
  configuration-coded D2C and remove the nonexistent `.Rexx.d2c(length)` API.
- Focused validation and second review: optimized and unoptimized isolated
  overlays both report `PASS: d2c`. The original 105 noopt / 99 opt lines and
  the final 105/105 lines each contain zero calls and one append; the six-line
  optimized increase preserves optional-argument presence and is outside the
  scalar-emission hot path. `git diff --check` passes and compiler/interpreter
  diffs remain empty.

### Selector inventory and evidence — row 50 `d2x`

- Public Level B surface: `d2x(xint=.int, [slen=.int]) -> .string` is a native
  signed-64-bit converter. The optional integer metadata is already typed, but
  the implementation uses `-1` as an exposed omission sentinel, silently caps
  requested widths at 20, mutates its local input while dividing, and treats
  negatives through an incorrect 16-bit `32768 + value` workaround.
- Performance baseline: original isolated output is 315 noopt / 313 opt lines,
  with one divide/remainder pair per nibble, two reverse-building append sites,
  a second character-by-character reversal, and two calls to the unreviewed
  `right` selector. Replace it with direct fixed-position native bit extraction,
  a single left-pad operation, and no selector calls or intermediate reversal.
- Level B contract/test finding: omitted length requires a non-negative input
  and minimal uppercase output; supplied non-negative length permits signed
  twos-complement and exact zero/F padding or low-digit truncation. The legacy
  harness covers useful book examples but has no PASS marker, signal cases,
  `INT64_MIN`/`INT64_MAX`, presence distinction, non-mutation, or width above
  the stale cap. Add those cases and a stable native page.
- Level C status: `D2X(number [,length])` is recognized but has no standalone
  implementation, compatibility-controller body, or direct harness. It needs
  CheckArgs `rWHOLENUM>=0` without length or `rWHOLENUM rWHOLE>=0` with length,
  caller numeric settings, arbitrary RexxValue whole numbers, signed
  twos-complement, and standard context errors. The shared CheckArgs helper
  currently lacks both `WHOLENUM` rules; implement them once here for D2X and
  the parked D2C contract.
- Documentation/class finding: the language and class books mix Classic and
  native rules. `.Rexx.d2x()` forwards only its receiver and accepts no width,
  despite the class page's runnable examples claiming otherwise. Add separate
  B/C Markdown, correct both book surfaces, and leave compiler lowering and the
  deprecated common controller unchanged.
- Completed Level B result: D2X now tests optional presence, signals
  `INVALID_ARGUMENTS` for a negative width or a negative value without width,
  supports every non-negative native width without the stale 20-digit cap, and
  emits the required low nibbles directly with zero/F bulk padding. It handles
  the complete signed-64-bit domain without input mutation, per-digit division,
  an intermediate reverse, or another selector.
- Level C result: standalone
  `rexxclassicbifd2x.rexxclassicbif_d2x(context_ref)` implements the direct
  RexxValue contract with standard context errors and no compatibility-
  controller body. Shared CheckArgs now normalizes `WHOLENUM` and
  `WHOLENUM>=0` under inherited caller digits/form. D2X converts arbitrary
  plain decimal magnitudes with mutable base-1e9 limbs, complements its nibble
  array in place for negatives, and applies exact width without native-int
  narrowing.
- Test/doc result: rewritten Level B and new direct Level C harnesses cover all
  book examples, zero, omission versus explicit zero, signed truncation/padding,
  widths above the old cap, `INT64_MIN`/`INT64_MAX`, arbitrary values above
  64 bits, caller rounding and engineering form, non-mutation, and every
  applicable `40.3`/`40.4`/`40.5`/`40.12`/`40.13` or Level B signal family.
  Separate B/C pages plus both books now distinguish the contracts; the
  simplified `.Rexx.d2x()` examples also pass against current class artifacts.
- Focused validation and second review: all native, direct Classic, and class-
  example overlays report PASS in optimized and unoptimized modes. Native RXAS
  fell from 315/313 lines with two `right` calls, divide/remainder, and reversal
  to 260/252 lines with zero calls, two shift sites, one mask, one bulk pad, and
  one append site. Direct Classic is 788 noopt / 765 opt lines; its only calls
  are context/shared validation, RexxValue construction, and private conversion
  helpers, with no general Level B selector in the conversion core. `git diff
  --check` passes and compiler/interpreter diffs remain empty.

### Selector inventory and evidence — row 51 `x2b`

- Public Level B surface: current `x2b(hex=.string, [slen=.int]) -> .string`
  accepts an unused optional length and `.Rexx.x2b(slen)` forwards that inert
  argument. The documented and Classic surface is one hexadecimal string only;
  remove the meaningless parameter from both Level B entry points rather than
  preserving a misleading typed argument.
- Correctness/error finding: the implementation drops leading zero nibbles,
  collapses an all-zero input to four bits, accepts leading/trailing or wrongly
  grouped blanks, prints an error to stdout, and returns empty for invalid text.
  Standard HEX grouping preserves every digit, permits interior blanks only
  when an even number of hex digits lies to their right, and requires failures
  to be signals in Level B or `40.25` in Level C.
- Performance baseline: original isolated output is 221 noopt / 219 opt lines,
  with one `poschar` over a duplicated mixed-case table per source character,
  an inner four-character copy, and a diagnostic `say`. Replace it with one
  bounded right-to-left validation pass and one left-to-right nibble emission
  pass using direct ASCII range arithmetic and no helper calls.
- Level C status: recognized `X2B(hex)` has CheckArgs `rHEX` but no standalone
  implementation, controller body, or direct harness. Add the shared HEX rule
  once for X2B/X2C/X2D, then implement a direct RexxValue converter without
  compiler changes or a compatibility-controller branch.
- Test/doc finding: the legacy harness contains useful conversions but depends
  on D2X, lacks a PASS marker and signals, and asserts the invalid trailing-
  blank behavior. Neither B nor C has a stable page; both books omit grouping,
  empty input, errors, and leading-zero preservation. Rewrite the harnesses and
  align the class adapter/docs with the actual one-argument contract.
- Completed Level B result: X2B now exposes only its meaningful typed string
  argument, preserves every leading/all-zero nibble, validates standard HEX
  blank grouping right-to-left, signals `INVALID_ARGUMENTS`, and emits four
  bits per digit in a second direct scan. The stdout diagnostic, mixed-case
  search table, unused length, D2X dependency, and empty-on-error behavior are
  removed.
- Level C result: shared CheckArgs now implements HEX and standard `40.25`;
  standalone `rexxclassicbifx2b.rexxclassicbif_x2b(context_ref)` performs the
  direct RexxValue conversion. It has no common-controller body and needs no
  compiler or configuration service.
- Test/doc result: rewritten B and new C harnesses cover all book examples,
  mixed case, empty input, leading/all-zero nibbles, valid/consecutive grouping,
  non-mutation, 256 bits, every invalid blank/character family, argument count,
  omission, and `40.25`. Separate stable B/C pages and both books now document
  the same scope. `.Rexx.x2b` source and its focused class test now use the
  correct no-argument adapter, pending the single deferred classlib rebuild.
- Focused validation and second review: native and direct Classic overlays all
  report PASS optimized/unoptimized. Native output is 344/328 lines versus the
  original 221/219, but replaces silent corruption with two bounded scans,
  zero calls, no `say`, direct range arithmetic, and one four-bit append site.
  Direct Classic is 245/249 lines; after shared HEX validation it performs one
  conversion scan and its only non-context work is one character read and one
  four-bit append site. `git diff --check` passes and compiler/interpreter diffs
  remain empty.

### Selector inventory and evidence — row 52 `x2c`

- Public Level B surface: `x2c(hex=.string) -> .string` maps each parsed byte
  to the corresponding Unicode code point U+0000 through U+00FF, left-padding
  an odd leading nibble. That preserves the current valid-UTF-8 `.string`
  invariant and is intentionally not a raw-byte or configured-character API.
- Correctness/error finding: current source deletes every blank before checking
  grouping, finds digits through a duplicated mixed-case table, prints invalid
  input to stdout, and returns numeric zero through a string signature. Apply
  the same standard HEX grouping already proven for X2B and signal
  `INVALID_ARGUMENTS`; preserve empty input, odd-nibble padding, every leading
  zero byte, and the native Unicode mapping.
- Performance baseline: original isolated output is 229 noopt / 226 opt lines,
  with a full `dropchar` copy, two `poschar` searches per byte, one append, and
  a `say`. Replace it with one right-to-left validation/count pass and one
  left-to-right stateful pair conversion, direct ASCII arithmetic, no cleaned
  intermediate string, and no helper calls.
- Level C contract: recognized `X2C(hex)` uses CheckArgs `rHEX`, pads to a full
  byte, and converts bits through the configuration-coded `Config_B2C` service.
  The shared HEX rule is now complete, but the encoding service is still
  undefined. Park standalone RexxValue X2C rather than hard-coding the Level B
  Unicode U+00xx behavior.
- Test/doc finding: the current harness round-trips through C2X and therefore
  hides the actual string bytes; it has no PASS marker, signal tests, or direct
  Unicode boundaries. The books mix ASCII/Unicode/EBCDIC examples and even
  describe X2C as a single-character conversion. Add standalone native tests,
  separate stable B/C pages, and correct the two book surfaces.
- Completed Level B result: X2C now preserves its typed one-string native API,
  validates standard HEX grouping with `INVALID_ARGUMENTS`, counts digits
  without allocating a cleaned copy, left-pads an odd nibble in conversion
  state, and appends each parsed byte as U+00xx. Empty input and every leading
  zero byte are retained; invalid text never writes stdout or returns numeric
  data through the string signature.
- Test/doc result: the standalone harness directly verifies ASCII, NUL/leading
  zero, odd nibble, U+0080, U+00FF, mixed-case U+00xx UTF-8 bytes, valid grouping,
  non-mutation, 64-byte input, and every invalid blank/character family without
  C2X or another selector. Separate B/C pages and corrected language/class books
  now distinguish Unicode-byte Level B from configured-character Level C.
- Focused validation and second review: optimized and unoptimized overlays both
  report `PASS: x2c`. The implementation is 354/334 lines versus the original
  229/226, but the added code is required validation; runtime is two bounded
  scans with zero calls, no `dropchar`, no `poschar`, no `say`, direct nibble
  arithmetic, and one append site. `git diff --check` passes and compiler/
  interpreter diffs remain empty.

### Selector inventory and evidence — row 53 `x2d`

- Public Level B surface: `x2d(hex=.string, [slen=.int]) -> .int` is a native
  signed-64-bit converter. Omitted length is unsigned; a supplied non-negative
  length selects/pads the rightmost hexadecimal digits and interprets their
  high bit as twos-complement. The `-1` sentinel currently makes explicit
  negative lengths look omitted, and `.Rexx.x2d` always forwards that sentinel
  instead of preserving presence.
- Correctness/error finding: source caps length at 20, calls `right` on grouped
  input before validating/removing blanks, searches a mixed-case table, can
  overflow silently while accumulating, writes invalid input to stdout, and
  returns zero for errors. Replace those paths with shared-standard HEX
  grouping, signal-based invalid/overflow handling, and a bounded native-domain
  accumulation that accepts only valid sign extension above 64 bits.
- Performance baseline: original isolated output is 296 noopt / 286 opt lines,
  with one `right` call, a `poschar` per digit, a power-building loop for signed
  values, and `say`. Use validation/count plus one selected-digit scan, direct
  nibble arithmetic, no cleaned/right-aligned string, and no helper calls.
- Level C status: recognized `X2D(hex [,length])` has CheckArgs
  `rHEX oWHOLE>=0` but no standalone function, controller body, or direct test.
  It must return an arbitrary whole-number RexxValue under caller `NUMERIC
  DIGITS`, apply signed twos-complement only when length is present, and report
  `40.35` if the result cannot be expressed. Shared HEX is already complete;
  no configuration service or compiler change is needed.
- Test/doc finding: the 300-line legacy harness is mostly duplicate or commented
  experiments, has no PASS marker or errors, and does not isolate native
  overflow. The books contain the core Classic examples but no stable B/C
  pages, grouping/error boundaries, or native limit distinction. Replace the
  harness, add direct Level C coverage, and correct the presence-losing class
  adapter at the deferred classlib checkpoint.
- Completed Level B result: `x2d(.string [, .int]) -> .int` now preserves
  optional presence, applies standard HEX grouping, and signals
  `INVALID_ARGUMENTS` or `OVERFLOW_UNDERFLOW`. Unsigned accumulation checks the
  signed-64-bit maximum before each nibble; signed conversion selects without a
  copy and accepts arbitrarily long valid zero/sign extension while rejecting
  values that cannot fit the native result.
- Direct Level C result: `RexxClassicBifX2d` validates `rHEX oWHOLE>=0`, caps a
  validated arbitrary-width length before native narrowing, and converts the
  selected field through mutable little-endian base-1e9 limbs. It returns exact
  RexxValue whole-number text, handles signed twos-complement, enforces caller
  `NUMERIC DIGITS` with `RXC-LC-40.35`, and never calls the compatibility name
  controller.
- Test/doc result: focused B and direct C harnesses cover all documented
  unsigned/signed examples, valid/invalid grouping, mixed-case sign nibbles,
  empty/zero width, huge and leading-zero widths, native extrema/sign extension,
  arbitrary-width results, non-mutation, standard context errors, and caller
  digits. Separate stable B/C pages plus corrected language/class books now
  state the distinct native and RexxValue limits.
- Focused validation and second review: optimized and unoptimized overlays both
  report `PASS: x2d` and `PASS: Level C X2D BIF`. Level B output is 846/754
  lines versus the incomplete original 296/286; the added body is explicit HEX
  and overflow handling, while runtime remains two linear scans, direct nibble
  arithmetic, no allocation/right/table search/stdout, and zero optimized
  selector calls. Direct C output is 1243/1192 lines and uses the conventional
  arbitrary-precision limb algorithm. `git diff --check` passes and compiler/
  interpreter diffs remain empty.

### Selector inventory and evidence — row 54 `xrange`

- Public Level B surface: `xrange(from=.string, tos=.string) -> .string` is
  documented in its source as the deprecated byte-oriented wrapping range, but
  its body writes a warning to stdout and calls `sequence`. `sequence` is an
  unreviewed Unicode non-wrapping selector that emits its own stdout diagnostic
  and `BAD` value for descending endpoints, so the current implementation does
  not provide the stated XRANGE behavior. `.Rexx.xrange` is worse: it never
  calls the selector and always returns a deprecation-message object.
- Native contract available without a language decision: retain the typed
  single-character `.string` endpoints and valid UTF-8 `.string` result, map
  only codepoints U+0000 through U+00FF, include both endpoints, and wrap at
  U+00FF. Reject non-single-character or out-of-domain endpoints with
  `INVALID_ARGUMENTS`. This is the existing stated Level B byte-range contract,
  not the configuration-coded Classic BIF policy.
- Performance baseline: original isolated output is 41/41 lines and performs
  one `say` plus one selector call. Replace it with bounded direct codepoint
  validation and at most 256 `appendchar` operations; no helper call, allocation
  of an aligned copy, table search, or stdout path is needed.
- Level C status: the repository recognizes `XRANGE([start [,end]])` with
  `oPAD oPAD`. Its specification explicitly delegates defaults, ordering, and
  encoded-character production to `Config_Xrange`; no service, standalone BIF,
  controller body, or test exists. UTF-8 Level B strings must not be guessed as
  that coded-character configuration. Park C until the configuration contract
  is approved/implemented, which also keeps row 31 `translate` parked.
- Test/doc status: no focused Level B or Level C XRANGE harness exists. The
  language book documents only `SEQUENCE`; the class reference exposes the
  broken deprecation method; and there are no stable selector-local B/C pages.
  Add focused native wrap/boundary/error/non-mutation coverage, correct the
  adapter source and its deferred class test, and document the distinct native
  contract plus the parked Level C configuration dependency.
- Completed Level B result: `xrange(.string, .string) -> .string` now validates
  two one-character U+0000..U+00FF endpoints, appends the inclusive range
  directly, wraps at U+00FF, and signals `INVALID_ARGUMENTS`. It has no stdout
  path or dependency on the unreviewed `sequence` selector. `.Rexx.xrange` now
  forwards to that implementation in source instead of returning a warning.
- Test/doc result: the focused native harness covers ascending/equal/wrapped
  ranges, the full 256-character domain including NUL and high characters,
  endpoint non-mutation, and every endpoint error family. Separate B/C pages,
  the language book, the class book, and RexxDoc now distinguish the native
  UTF-8 byte-domain helper from configuration-coded Classic XRANGE. The direct
  C page records the exact `Config_Xrange` dependency instead of claiming an
  implementation.
- Focused validation and second review: optimized and unoptimized overlays both
  report `PASS: xrange`. Output is 161/161 lines versus the broken original
  41/41; the executable path is bounded to 256 iterations with direct
  `appendchar`, no allocation/copy/table search, no helper call, and no `say`.
  `git diff --check` passes and compiler/interpreter diffs remain empty.

### Selector inventory and evidence — row 55 `arrayfind`

- Public surface: one Level B-only
  `arrayfind(find=.string, array=.string[] [,from=.int [,case=.int]]) -> .int`
  procedure over the established one-based string-array/high-water convention.
  It returns the first element whose text contains the needle, or zero. `from`
  defaults to one and values below one currently clamp to one; `case` defaults
  to `1` for case-sensitive matching and `0` selects case-insensitive matching.
  No Level C BIF, class method, or non-array public surface exists.
- Correctness/error finding: the array is read-only and correctly typed, but
  the current `case` branch silently treats every value other than exactly one
  as case-insensitive. Restrict the integer flag to 0/1 and signal
  `INVALID_ARGUMENTS`; retain the documented low-`from` clamp. Preserve POS
  semantics that an empty needle has no match and return zero immediately for
  an empty array or a start above the high-water mark.
- Performance baseline: original isolated output is 208 noopt / 221 opt lines.
  It calls the public `pos` selector once per candidate in both modes and, for
  case-insensitive matching, calls `upper` for the needle and every candidate
  in noopt (the optimizer only inlines `upper`). Use direct `strpos` with a
  one-based search register and direct `strupper` copies so this foundation
  scan has no selector calls in either mode. Uppercase the needle only once.
- Test/doc finding: there is no selector-focused harness. A broad array helper
  test covers `arraycontains` and `arrayindexof` but not `arrayfind`. The two
  language-book tables give only a one-line description and a mandatory-looking
  signature; no stable selector-local page or RexxDoc exists. Add focused
  match/start/case/empty/Unicode/non-mutation/error coverage, a stable Level B
  page, corrected optional signatures, and code-adjacent RexxDoc.
- Completed result: the typed read-only array surface retains the low-`from`
  clamp and now rejects case flags outside 0/1 with `INVALID_ARGUMENTS`. Empty
  needles/arrays and starts above the high-water mark return zero without
  scanning. Sensitive and insensitive paths use direct VM `strpos`; the latter
  folds the needle once and each visited candidate once.
- Test/doc result: the new focused harness covers first/later/skipped matches,
  low/high starts, both case modes, Unicode substrings, empty input, array
  non-mutation, and both invalid flag directions. The stable selector page,
  RexxDoc, and both language-book inventories now document the typed optional
  arguments and error behavior.
- Focused validation and second review: optimized and unoptimized overlays both
  report `PASS: arrayfind`. Output is 242/242 lines versus 208/221 originally;
  the small increase is explicit flag/empty validation, while both runtime
  paths now have zero selector calls (formerly `pos` per candidate and `upper`
  calls in noopt), one direct substring search per visited element, and only
  the required insensitive copies. `git diff --check` passes and compiler/
  interpreter diffs remain empty.

### Selector inventory and evidence — row 56 `splice`

- Public surface: one Level B-only string helper,
  `splice(needle=.string, haystack=.string, at=.int, len=.int) -> .string`.
  It removes up to `len` characters at the one-based position and inserts the
  replacement without padding. There is no Level C BIF or class adapter.
  Existing source misleadingly defaults `at` even though required `len` follows
  it; make both positional integer arguments explicitly required.
- Correctness/error finding: existing comments/tests treat `at < 1` as a silent
  no-op and `len < 0` as insertion. These are invalid typed positions/counts;
  per the programme error rule they should signal `INVALID_ARGUMENTS`. Preserve
  the useful documented behavior that a start beyond the end clamps to append
  and an overlong removal clamps to the remaining characters. Avoid adding
  `at + len` before clamping because large native counts could overflow.
- Performance baseline: original isolated output is 138 noopt / 458 opt lines.
  Noopt calls `length`, `left`, and `substr`; opt inlines all three large public
  selectors, including irrelevant pad/error branches, and creates multiple
  concatenation temporaries. Replace this with `strlen`, at most two direct
  cursor substrings and appends, clamped arithmetic, and fast paths for no-op,
  append, and whole-tail replacement.
- Test/doc finding: the existing noisy 15-case harness prints every successful
  case, has no PASS marker, encodes the two silent-invalid behaviors, lacks
  signals/non-mutation/Unicode coverage, and its helper itself calls `substr`.
  The language book only lists a signature and no stable selector page or
  RexxDoc exists. Replace/simplify the harness and add focused stable docs.
- Completed result: both positional controls are now required typed integers;
  invalid positions/removal lengths signal `INVALID_ARGUMENTS`. Starts beyond
  the source clamp to append, removals clamp before addition so native overflow
  is avoided, and all positions operate in Unicode characters. No-op, append,
  and whole-tail cases use direct fast paths.
- Test/doc result: the concise replacement harness covers grow/shrink/insert/
  delete, start/whole/last replacements, empty strings, explicit and clamped
  append, overlong removal, Unicode positions, input non-mutation, and both
  invalid signals with one PASS marker. Stable Level B Markdown, RexxDoc, and
  the language inventory now state the exact required typed contract.
- Focused validation and second review: optimized and unoptimized overlays both
  report `PASS: splice`. Output is 260/260 lines versus 138/458 originally.
  The new runtime path has zero selector calls (formerly three in noopt and a
  large three-selector inline expansion in opt), one `strlen`, no risky
  unclamped addition, and at most two bounded substrings plus appends.
  `git diff --check` passes and compiler/interpreter diffs remain empty.

### Selector inventory and evidence — row 57 `arrayinsert`

- Public surface: one Level B-only in-place string-array helper,
  `arrayinsert(array=.string[] expose, from=.int, count=.int
  [,default=.string]) -> .int`, used by `StringArrayList` and the array helper
  family. The source currently gives misleading defaults to the documented
  required `from` and `count`; make them explicitly required while retaining
  only the empty-string fill default. There is no Level C BIF.
- Correctness/error finding: `count = 0` is a valid no-op, positions beyond the
  end clamp to append, and the bulk insertion/fill behavior is correct. A
  negative count or position below one is currently a silent no-op; these
  invalid controls should signal `INVALID_ARGUMENTS`. Because this is an
  `arg expose` mutator, caught-signal caller preservation must be observed
  against the already parked exposed-argument signal-unwind ABI issue rather
  than repaired in compiler/runtime during this selector row.
- Performance baseline: original isolated output is 136/136 lines and already
  uses one `insattrs1` bulk pointer shift followed by exactly one assignment per
  inserted string slot. Retain that algorithm; only validation, explicit types,
  and clearer bounded loop arithmetic should change. No selector calls or
  avoidable array-copy loop exists.
- Test/doc finding: insertion has several cases embedded in the broad
  `ts_arraydelete` harness, including the obsolete low-index no-op expectation,
  but no focused harness. The two books list a mandatory-looking signature and
  short note; no stable selector page or RexxDoc exists. Add a focused harness
  for mutation, fill/copy, clamp/no-op, signals, and reference preservation;
  update the broad invalid case and create stable documentation.
- Completed normal-path result: `from` and `count` are now explicitly required
  integers, count zero remains a no-op, beyond-end insertion clamps to append,
  and invalid controls signal `INVALID_ARGUMENTS`. The implementation retains
  one `insattrs1` and replaces the end-index expression with a count-controlled
  fill loop. `StringArrayList` callers already supply all required controls.
- Test/doc result: the focused harness covers empty/middle/prepend/clamped-
  append insertions, omitted fill, count zero, fill-value copying, high-water
  returns, and isolated invalid signals. The broad helper harness now expects a
  signal for a low insertion index and compiles against the new signature.
  Stable Level B Markdown, book signatures, and RexxDoc document the API.
- Focused validation and second review: both overlays report
  `PASS: arrayinsert`; the updated broad harness compiles in both modes. Output
  improves from 136/136 to 125/125 lines with one bulk `insattrs1`, no selector
  calls, and one required assignment per inserted slot. A dedicated signal
  probe confirmed the signal is catchable but the existing `arg expose` unwind
  displaces the caller array (`a[0]` observed as 0), so only that shared ABI
  preservation edge remains parked. `git diff --check` passes and compiler/
  interpreter diffs remain empty.

### Selector inventory and evidence — row 58 `arraydelete`

- Public surface: one Level B-only in-place string-array helper,
  `arraydelete(array=.string[] expose, from=.int, count=.int) -> .int`.
  It deletes a one-based range, shifts the tail down, and returns the new
  high-water mark. Source currently supplies defaults despite all book/caller
  signatures making `from` and `count` required. No Level C BIF exists.
- Correctness/error finding: count zero and a start beyond the high-water mark
  are valid no-ops; a negative count or position below one should signal
  `INVALID_ARGUMENTS` rather than silently returning. The current tail-clamp
  test computes `from + count - 1` before clamping and can overflow; compare
  count with the already bounded available tail instead. The shared caught-
  signal `arg expose` displacement is already proved by row 57 and will be
  parked without another destructive probe.
- Performance baseline: original isolated output is 121/121 lines and already
  uses the desired single `delattrs1` bulk pointer shift with no selector calls
  or Rexx-level copy loop. Preserve that core and simplify validation/clamping.
- Test/doc finding: deletion cases are mixed into `ts_arraydelete`, including
  obsolete low-index no-op behavior, with no focused selector harness. The
  books contain only signature/table notes and no stable selector page or
  RexxDoc. Add focused middle/edge/tail/no-op/signal coverage, update the broad
  expectation, and document the in-place/bulk behavior.
- Completed normal-path result: `from` and `count` are explicitly required
  integers; invalid controls signal `INVALID_ARGUMENTS`. Zero count, empty
  input, and above-high starts remain no-ops. Tail clamping now compares against
  `hi - from + 1`, eliminating the original overflowing `from + count - 1`
  expression, then performs exactly one `delattrs1`.
- Test/doc result: the focused harness covers middle/first/last/whole/tail
  deletion, a very large clamped count, empty and both no-op paths, high-water
  results, and isolated invalid signals. The broad harness's low-index case now
  expects a signal and compiles with both corrected mutators. Stable Level B
  Markdown and RexxDoc describe the bulk in-place behavior.
- Focused validation and second review: both overlays report
  `PASS: arraydelete`; output improves from 121/121 to 109/109 lines with one
  `delattrs1`, zero selector calls, no copy loop, and overflow-safe arithmetic.
  The row shares the already demonstrated `arg expose` caught-signal caller
  displacement, so preservation is parked without another probe.
  `git diff --check` passes and compiler/interpreter diffs remain empty.

### Selector inventory and evidence — row 59 `arrayappend`

- Public surface: one Level B-only in-place string-array helper,
  `arrayappend(array=.string[] expose, value=.string [,count=.int]) -> .int`.
  Count defaults to one and the result is the new high-water mark. It is used
  by the broad helper test and is part of the documented supported array
  family; no Level C BIF exists.
- Correctness/error finding: count zero is a valid no-op, but a negative count
  currently silently returns and should signal `INVALID_ARGUMENTS`. The
  exposed-signal caller-preservation ABI is already parked. Successful append
  semantics and typed arguments are otherwise correct.
- Performance baseline: original isolated output is 106/106 lines, using one
  `insattrs1` followed by one value assignment per appended slot, with no
  selector call or tail-copy loop. Retain that algorithm and use a count-driven
  fill loop rather than computing `from + count - 1` before iteration.
- Test/doc finding: only one single and one repeated append assertion exist in
  the mixed broad array test. There is no zero/negative/non-mutation/fill-copy
  focused coverage, stable selector page, or RexxDoc; book detail tables also
  make optional count look required. Add a focused harness and correct docs.
- Completed normal-path result: count remains an optional integer defaulting to
  one, zero is a no-op, and negatives now signal `INVALID_ARGUMENTS`. The fill
  loop is count-controlled, avoiding the original computed last index, while
  preserving a single bulk growth operation and value-copy semantics.
- Test/doc result: the focused harness covers empty/default append, repeated
  Unicode values, copied fill values, high-water returns, zero no-op, and an
  isolated negative-count signal. Stable Level B Markdown, RexxDoc, and both
  detail-table signatures now state optional count and error behavior.
- Focused validation and second review: both overlays report
  `PASS: arrayappend`. Output is 107/107 versus 106/106 originally; the one-line
  validation cost retains one `insattrs1`, zero selector calls/copy loops, and
  exactly one assignment per new slot. Caught-signal caller preservation shares
  the proven exposed-argument ABI dependency and is parked without another
  probe. `git diff --check` passes and compiler/interpreter diffs remain empty.

### Selector inventory and evidence — row 60 `arrayprepend`

- Public surface: one Level B-only in-place string-array helper,
  `arrayprepend(array=.string[] expose, value=.string [,count=.int]) -> .int`.
  Count defaults to one; the helper shifts the existing array upward and fills
  the new front slots. No Level C BIF exists.
- Correctness/error finding: as with `arrayappend`, zero count is a valid no-op
  but negative count silently returns and should signal `INVALID_ARGUMENTS`.
  The source types/normal semantics are otherwise correct. The top language
  inventory incorrectly labels the optional third argument `default` rather
  than `count`.
- Performance baseline: original isolated output is 95/95 lines with the
  desired one `insattrs1` tail shift and one assignment per new slot, no
  selector call, and no Rexx-level movement of existing values. Preserve it
  and use a count-controlled fill loop.
- Test/doc finding: the mixed broad array test covers only a single prepend.
  Repeated/zero/negative/copy/Unicode behavior has no focused coverage; stable
  selector Markdown and RexxDoc are absent, and detailed book tables make count
  look required. Add the focused surface and correct all signatures.
- Completed normal-path result: optional integer count still defaults to one;
  zero remains a no-op and negatives now signal `INVALID_ARGUMENTS`. A
  count-controlled fill loop preserves the one bulk shift and avoids computing
  an upper index.
- Test/doc result: the focused harness covers existing-array/default prepend,
  repeated Unicode values, copied fill values, high-water returns, zero no-op,
  and an isolated negative-count signal. Stable Level B Markdown, RexxDoc, and
  all book signatures now name the optional `count` correctly.
- Focused validation and second review: both overlays report
  `PASS: arrayprepend`. Output is 106/106 versus 95/95 originally; the added
  validation retains one `insattrs1`, zero selector calls/copy loops, and one
  assignment per prepended slot. Caught-signal caller preservation is the same
  parked exposed-argument ABI issue. `git diff --check` passes and compiler/
  interpreter diffs remain empty.

### Selector inventory and evidence — row 61 `objectarrayinsert`

- Public surface: one Level B-only in-place object-array helper,
  `objectarrayinsert(array=.object[] expose, from=.int, count=.int,
  value=.object) -> .int`, used by `ObjectArrayList` and `ObjectLinkedList`.
  Source currently defaults required `from` and `count`; the object fill value
  is correctly required. No Level C BIF exists.
- Correctness/error finding: successful bulk gap/fill behavior, append clamping,
  and object-value assignment are structurally correct. Make position/count
  explicitly required, retain count-zero no-op, and signal negative count or
  position below one. Caught-signal caller preservation shares the known
  exposed-array ABI dependency.
- Performance baseline: original isolated output is 131/131 lines with one
  `insattrs1`, no selector calls, and one object-value assignment per new
  slot. Preserve it and replace the computed loop end with a count-controlled
  loop.
- Test/doc finding: only two insertion cases exist in the broad object-array
  harness. There is no focused coverage for empty/prepend/repeated value/
  zero/signal behavior, no selector-local page or RexxDoc, and the language
  array-helper tables omit the entire object-array family. Add the focused
  harness/page and the active selector's book row without changing class APIs.
- Completed normal-path result: position/count are required integers, zero
  count is a no-op, beyond-end insertion clamps to append, and invalid controls
  signal `INVALID_ARGUMENTS`. The count-controlled fill retains one bulk gap
  operation. Review/test corrected an initial assumption: ordinary Level B
  object assignment copies object values rather than creating shared references.
- Test/doc result: the focused harness covers empty/repeated value copies,
  middle/prepend/clamped-append insertion, zero count, high-water returns, and
  isolated invalid signals. Mutating the caller's box after insertion proves
  stored values are independent copies. Stable Markdown, RexxDoc, and new book
  rows document that object-value contract.
- Focused validation and second review: both overlays report
  `PASS: objectarrayinsert`. Output improves from 131/131 to 120/120 lines with
  one `insattrs1`, no selector calls/tail-copy loop, and one necessary object-
  value assignment per slot. Signal preservation shares the common exposed-
  argument ABI dependency. `git diff --check` passes and compiler/interpreter
  diffs remain empty.

### Selector inventory and evidence — row 62 `objectarraydelete`

- Public surface: one Level B-only in-place object-array helper,
  `objectarraydelete(array=.object[] expose, from=.int, count=.int) -> .int`,
  used by `ObjectArrayList` and `ObjectLinkedList`. Source defaults controls
  that all callers/docs treat as required. No Level C BIF exists.
- Correctness/error finding: zero count, empty input, and starts above the
  high-water mark are valid no-ops. Positions below one and negative counts
  should signal. Replace the overflowing `from + count - 1` tail check with an
  available-count comparison and return the VM-maintained `array[0]` rather
  than separately recomputing `hi - count`.
- Performance baseline: original isolated output is 122/122 lines with one
  desired `delattrs1`, no selector calls, and no object copy/movement loop.
  Preserve that bulk algorithm.
- Test/doc finding: the broad object-array harness covers middle and clamped-
  tail deletion only. Focused edge/no-op/signal/high-water coverage, stable
  Markdown, RexxDoc, and language-book rows are missing. Add them, with the
  common exposed-signal caller-preservation issue parked.
- Completed normal-path result: position/count are required integers; zero,
  empty, and above-high paths remain no-ops; invalid controls signal. The tail
  clamps through an available-count comparison and the result now comes from
  VM-maintained `array[0]` after one bulk deletion.
- Test/doc result: the focused harness covers middle/first/last/whole/tail
  deletion, very large clamping, empty/no-op paths, high-water returns, and
  isolated invalid signals. Stable Markdown, RexxDoc, and new book rows document
  object-array deletion.
- Focused validation and second review: both overlays report
  `PASS: objectarraydelete`. Output improves from 122/122 to 109/109 lines with
  one `delattrs1`, zero selector calls/object-copy loops, and overflow-safe
  arithmetic. Signal preservation shares the common exposed-argument ABI park.
  `git diff --check` passes and compiler/interpreter diffs remain empty.

### Selector inventory and evidence — row 63 `objectarrayappend`

- Public surface: one Level B-only in-place object-array helper,
  `objectarrayappend(array=.object[] expose, value=.object [,count=.int]) ->
  .int`, used by both object list classes. Count defaults to one. No Level C
  BIF exists.
- Correctness/error finding: zero count is a valid no-op; negative count should
  signal instead of silently returning. Repeated fills copy the supplied object
  value into each slot under ordinary Level B value semantics. Exposed-signal
  preservation shares the known ABI dependency.
- Performance baseline: original isolated output is 106/106 lines with one
  `insattrs1`, no selector calls, and one object assignment per appended slot.
  Retain it and remove the computed last-index loop bound.
- Test/doc finding: the broad object-array harness covers single/repeated append
  but not value-copy independence, zero/negative paths, or a focused selector
  surface. Stable Markdown/RexxDoc/book rows are absent. Add them without
  changing class consumers.
- Completed normal-path result: optional count retains its default of one,
  zero is a no-op, and negatives signal. The count-controlled fill preserves
  one bulk growth and ordinary object-value copies.
- Test/doc result: the focused harness covers default/repeated append, high-
  water returns, zero no-op, isolated invalid count, and proves stored object
  values remain unchanged after the caller object is mutated. Stable Markdown,
  RexxDoc, and book rows document value-copy semantics.
- Focused validation and second review: both overlays report
  `PASS: objectarrayappend`. Output is 107/107 versus 106/106 originally, with
  one `insattrs1`, zero selector calls/tail loops, and one necessary object-
  value assignment per slot. Signal preservation shares the exposed-argument
  ABI park. `git diff --check` passes and compiler/interpreter diffs remain
  empty.

### Selector inventory and evidence — row 64 `objectarrayprepend`

- Public surface: one Level B-only in-place object-array helper,
  `objectarrayprepend(array=.object[] expose, value=.object [,count=.int]) ->
  .int`, used by both object list classes. Count defaults to one; no Level C
  BIF exists.
- Correctness/error finding: mirror `objectarrayappend`: count zero is a valid
  no-op, negatives should signal, and each new slot receives an ordinary object-
  value copy. Caller preservation on caught signals shares the known exposed-
  argument ABI issue.
- Performance baseline: original isolated output is 95/95 lines with one
  `insattrs1`, no selector calls/tail-copy loop, and one object assignment per
  slot. Retain it with a count-controlled fill loop.
- Test/doc finding: the broad harness covers only one prepend. There is no
  repeated value-copy/zero/negative focused coverage, selector Markdown,
  RexxDoc, or language-book row. Add the symmetric focused surface.
- Completed normal-path result: optional count defaults to one, zero is a
  no-op, negatives signal, and a count-controlled loop fills object-value
  copies after one bulk front shift.
- Test/doc result: focused coverage proves default/repeated prepend, high-water
  returns, zero no-op, isolated invalid count, and object-value independence
  from later caller mutation. Stable Markdown, RexxDoc, and book rows document
  the contract.
- Focused validation and second review: both overlays report
  `PASS: objectarrayprepend`. Output is 106/106 versus 95/95, retaining one
  `insattrs1`, no selector calls/tail-copy loop, and one object-value assignment
  per new slot. Signal preservation shares the common exposed-argument ABI
  park. `git diff --check` passes and compiler/interpreter diffs remain empty.

### Selector inventory and evidence — row 65 `objectarraydrop`

- Public surface: one Level B-only in-place
  `objectarraydrop(array=.object[] expose) -> .int` helper used by object list
  classes and `StringObjectHashMap`; it clears the array and returns zero. No
  Level C BIF exists and there are no invalid control arguments.
- Correctness/performance finding: the existing typed surface and algorithm are
  already ideal: one `setattrs array,0` releases all logical slots and updates
  the caller-owned high-water mark without a Rexx loop. Original isolated
  output is 22/22 lines with zero calls. Preserve this body.
- Test/doc finding: the broad object-array harness checks one populated clear,
  but there is no focused empty/idempotent/reuse test, stable selector page,
  RexxDoc, or language-book row. Add these assets. Because the function never
  signals, the exposed-signal unwind dependency does not block this row.
- Completed result: the typed exposed-array surface and one-instruction
  `setattrs array,0` body are unchanged. Focused tests now prove empty clear,
  populated clear, idempotence, zero return/high-water, and reuse after clear.
  Stable Markdown, RexxDoc, and book rows document the behavior.
- Focused validation and second review: both overlays report
  `PASS: objectarraydrop`; output remains the optimal 22/22 lines with one
  `setattrs`, zero calls, and no object loop. `git diff --check` passes and
  compiler/interpreter diffs remain empty.

### Selector inventory and evidence — row 66 `objectarraymove`

- Public surface: one Level B-only in-place object-array helper,
  `objectarraymove(array=.object[] expose, from=.int, count=.int, to=.int) ->
  .int`. It moves a block within the same one-based array and returns the
  unchanged high-water mark. Source defaults all three documented required
  controls. No Level C BIF exists.
- Correctness/error finding: count zero, empty input, source above high-water,
  and a destination inside/immediately after the selected block are valid
  no-ops; source counts clamp to the available tail and high destinations clamp
  to append. Make controls required, signal negative count or source/
  destination below one, and avoid the overflowing `from + count - 1` check.
  Signal preservation shares the known exposed-array ABI dependency.
- Performance baseline: original isolated output is 291/291 lines. It copies
  every object value into a temporary array, bulk-deletes the source, bulk-
  inserts a gap, then copies every value again into the destination: two deep
  object copies per moved item plus temp-array allocation. No VM move-block
  opcode exists, but inserting the destination gap first makes source and target
  non-overlapping; values can be copied directly once before one bulk deletion,
  eliminating the temporary and one complete copy pass.
- Test/doc finding: the broad object-array harness covers only one backward
  move. Forward/append/prepend/clamp/overlap/no-op/error behavior, stable
  Markdown, RexxDoc, and a language-book row are absent. Add focused coverage
  that specifically exercises both insert-first index transformations.
- Completed result: the public controls are required `.int` values. Invalid
  source, count, and destination values signal `INVALID_ARGUMENTS`; count zero,
  empty/source-high inputs, and destinations within or immediately after the
  selected block are documented no-ops. Count and destination clamping avoid
  overflow-prone arithmetic.
- Focused validation and second review: both isolated overlays report
  `PASS: objectarraymove` across backward, forward, prepend, append, clamp,
  overlap, no-op, empty, and signal cases. Output fell from 291/291 to 239/239
  lines and contains one `insattrs1`, one `delattrs1`, no temporary array, and
  one object `copy` per moved value instead of two. Stable Markdown, RexxDoc,
  and book rows now describe the contract. `git diff --check` passes and
  compiler/interpreter diffs remain empty. Final B/V closure is parked only on
  the shared exposed-argument signal unwind ABI.

### Selector inventory and evidence — row 67 `arrayget`

- Public surface: Level B-only
  `arrayget(array=.string[], index=.int, default=.string optional) -> .string`.
  It returns the indexed string for a one-based in-range index and `default`
  for an empty array or any index outside `1..array[0]`; omitted `default` is
  the empty string. There is no matching Level C recognition-ledger entry or
  class adapter.
- Correctness/error finding: the array and index types are already correct and
  required. Out-of-range access is the documented successful fallback path,
  not an error, so this selector has no runtime signal case. The implementation
  does not expose or mutate its input.
- Performance baseline: the current selector is 59/59 RXAS lines, with two
  bounds branches and one linked attribute read only on the success path. It
  has no calls or array copies; preserve this body unless focused validation
  exposes a semantic defect.
- Test/doc finding: the shared broad array test checks one present and one high
  fallback case only. Add a focused harness for first/last, low/high, empty,
  omitted/explicit defaults, and input preservation; add stable Level B
  Markdown and RexxDoc, and correct the malformed `array=` book signature.
- Completed result: the optimal typed selector body is unchanged. Focused
  coverage now proves first/last retrieval, low/high and empty fallbacks,
  omitted and explicit defaults, and input preservation. The test derives miss
  indexes at runtime because optimized import with a literal non-positive index
  otherwise triggers the compiler's unrelated static array-bound diagnostic.
- Focused validation and second review: both overlays report `PASS: arrayget`.
  Output remains 59/59 lines, with two bounds branches, one `linkattr1` read on
  the success path, and no calls, copies, or mutation. Stable Markdown and
  RexxDoc document the distinct fallback contract, the book signature is
  corrected, `git diff --check` passes, and compiler/interpreter diffs remain
  empty.

### Selector inventory and evidence — row 68 `arrayset`

- Public surface: Level B-only in-place helper
  `arrayset(array=.string[] expose, index=.int, value=.string,
  fill=.string optional) -> .int`. It overwrites an existing one-based element
  or grows the array through `index`, filling any intervening gap, and returns
  the new high-water mark. There is no matching Level C recognition-ledger
  entry or class adapter.
- Correctness/error finding: types are correct, but non-positive indexes
  silently return without setting anything. Align this mutator with the array
  family by signalling `INVALID_ARGUMENTS`; signal preservation then shares
  the known exposed-array ABI dependency. Required controls must remain
  required and omitted `fill` remains the empty string.
- Performance baseline: current output is 122/122 lines. Growth uses one
  `insattrs1`, then writes `fill` into every new slot including the target and
  immediately overwrites the target with `value`. Fill only the intervening
  `index-hi-1` slots, and skip that loop entirely for the common empty-fill
  case because inserted string attributes are already empty. Existing-index
  assignment remains a single string copy.
- Test/doc finding: the shared broad harness covers one filled growth and two
  overwrites, but not empty/default growth, single append, input preservation,
  or invalid-index signals. Add focused coverage plus stable Level B Markdown
  and RexxDoc; retain the existing book row and clarify error/fill behavior in
  the stable page.
- Completed result: non-positive indexes now signal `INVALID_ARGUMENTS`.
  Growth still uses one bulk insertion, but an empty/default fill performs no
  per-gap string assignments and a non-empty fill writes only intervening
  slots; the target receives `value` exactly once. Existing-index assignment
  remains direct and preserves neighbors/high-water.
- Focused validation and second review: both overlays report `PASS: arrayset`
  for filled/default growth, overwrite, append, preservation, and signal cases.
  Output is 139/139 lines versus 122/122 because of the required signal and
  empty-fill branch, while eliminating up to `index-hi` redundant string
  writes on the common default-fill path and one write otherwise. Stable
  Markdown and RexxDoc are complete; `git diff --check` passes and compiler/
  interpreter diffs remain empty. B/V closure is parked only on the shared
  exposed-argument signal unwind ABI.

### Selector inventory and evidence — row 69 `arraycontains`

- Public surface: Level B-only read helper
  `arraycontains(array=.string[], value=.string, case=.int optional) -> .int`.
  It returns one when a complete element matches and zero otherwise; `case`
  defaults to one for case-sensitive matching and zero requests uppercase-
  folded matching. There is no matching Level C recognition-ledger entry or
  class adapter.
- Correctness/error finding: current types are correct, but every flag other
  than one silently selects case-insensitive behavior. Accept only zero/one and
  signal `INVALID_ARGUMENTS` otherwise, matching the reviewed array search
  family. Alias the read-only array with `arg expose` so the foundational
  predicate does not value-copy its entire input; signal preservation then
  shares the known exposed-array ABI dependency.
- Performance baseline: current isolated output is 148 unoptimized and 159
  optimized lines. Entry emits an array `copy`; unoptimized insensitive search
  calls `upper` once for the needle and once per visited element, while the
  optimizer expands those calls. Use direct `strupper`, fold the needle once,
  reuse one folded candidate, cache the high-water mark, and retain early exit.
- Test/doc finding: the broad harness covers one hit, one sensitive miss, and
  one insensitive hit. Add a focused harness for empty/first/last/duplicate,
  empty-string, sensitive/insensitive, invalid flags, and input preservation;
  add stable Level B Markdown and RexxDoc.
- Completed result: `case` now accepts only zero/one and otherwise signals
  `INVALID_ARGUMENTS`. The read-only array is exposed to avoid a whole-array
  value copy; exact matching uses strict string equality, and insensitive
  matching folds the value once and each visited element through direct VM
  instructions while preserving early exit.
- Focused validation and second review: both overlays report
  `PASS: arraycontains` across empty, first/last, duplicate, empty-string,
  sensitive/insensitive, miss, invalid-flag, and preservation cases. Output is
  178/178 lines versus 148/159, but removes the entry array `copy` and all
  per-search `upper` calls; the remaining size is the explicit validation and
  direct two-mode loops. Stable Markdown and RexxDoc are complete;
  `git diff --check` passes and compiler/interpreter diffs remain empty. B/V
  closure is parked only on the shared exposed-argument signal unwind ABI.

### Selector inventory and evidence — row 70 `arrayindexof`

- Public surface: Level B-only read helper
  `arrayindexof(array=.string[], value=.string, from=.int optional,
  case=.int optional) -> .int`. It returns the first complete-element match at
  or after `from`, or zero; `from` defaults to one and values below one clamp
  to one. `case` defaults to one for sensitive matching and zero selects folded
  matching. There is no matching Level C recognition-ledger entry or adapter.
- Correctness/error finding: current typed defaults and `from` clamping are
  appropriate, but every case flag other than one silently selects insensitive
  behavior. Accept only zero/one and signal `INVALID_ARGUMENTS`. Alias the
  read-only array with `arg expose` to avoid a full value copy; caught invalid-
  flag signals then share the exposed-array ABI dependency.
- Performance baseline: current isolated output is 185 unoptimized and 196
  optimized lines. Entry copies the array; unoptimized insensitive search
  calls `upper` for the value and every visited element. Cache `array[0]`, use
  strict equality, fold through direct `strupper`, fold the value once, reuse a
  candidate buffer, and preserve early exit/start clamping.
- Test/doc finding: the broad harness covers one exact match, a `from` miss,
  and one insensitive match. Add focused coverage for empty/first/last/
  duplicate/empty-string, low/high starts, sensitive/insensitive misses,
  invalid flags, and preservation; add stable Level B Markdown/RexxDoc and
  correct the malformed book signature spacing.
- Completed result: the read-only array is exposed to remove its entry value
  copy; only zero/one case flags are accepted, exact matching is strict, and
  folded matching uses direct VM uppercase operations with a once-folded value
  and reusable candidate. Start clamping and early exit are preserved.
- Focused validation and second review: both overlays report
  `PASS: arrayindexof` across all start, match, case, empty, signal, and
  preservation paths. Output is 204/204 lines versus 185/196, with no entry
  array `copy` and no `upper` calls; remaining growth is explicit flag
  validation. Stable Markdown/RexxDoc and the corrected book signature are
  complete; `git diff --check` passes and compiler/interpreter diffs remain
  empty. B/V closure is parked only on the shared exposed-argument signal
  unwind ABI.

### Selector inventory and evidence — row 71 `arraycopy`

- Public surface: Level B-only slice helper
  `arraycopy(array=.string[], from=.int optional, count=.int optional) ->
  .string[]`. It returns independent string copies; `from` defaults to one,
  negative starts count back from the end, and zero `count` means through the
  end. Starts outside the array return empty and oversized counts clamp. There
  is no matching Level C recognition-ledger entry or class adapter.
- Correctness/error finding: typed defaults are correct, but a negative count
  currently aliases the zero/to-end sentinel. Treat negative lengths as
  `INVALID_ARGUMENTS`, retaining only zero as the documented omitted/to-end
  value. The function is non-mutating and does not require `arg expose`; its
  current straight-line generated entry already borrows the array without an
  input `copy`, so the signal has no exposed-array unwind dependency.
- Performance baseline: current output is 180/180 lines. It calculates the
  slice safely but grows the result implicitly one slot at a time; every copied
  value executes two `minattrs`, two `linkattr1`, one `scopy`, and two unlinks.
  Allocate the result once with `setattrs`, then link the already-proven source
  and target slots directly, removing both per-element growth checks.
- Test/doc finding: the broad harness covers full and last-element copies only.
  Add focused coverage for omitted/positive/negative starts, bounded/zero/
  clamped counts, empty/out-of-range results, negative-count signalling,
  source preservation, and result independence. Add stable Level B Markdown/
  RexxDoc and correct the book signature to show optional `from`.
- Completed result: the source is explicitly exposed read-only so adding the
  negative-count signal cannot force a full input copy. The result is sized
  once; every element then uses two direct links, one `scopy`, and two unlinks,
  with no per-element bounds-growth instructions. Zero remains the to-end
  sentinel, oversized counts clamp, and negative counts signal.
- Focused validation and second review: both overlays report `PASS: arraycopy`
  for full/bounded/clamped/negative-start/empty/out-of-range/independence and
  signal cases. Output is 190/190 lines versus 180/180, but the loop removes
  both `minattrs` operations per copied string and retains no whole-array copy;
  extra static size is the signal, preallocation, and direct-link setup. Stable
  Markdown/RexxDoc and the optional-`from` book signature are complete;
  `git diff --check` passes and compiler/interpreter diffs remain empty. B/V
  closure is parked because a caught signal can still displace the borrowed
  caller array under the shared unwind ABI.

### Selector inventory and evidence — row 72 `arraydrop`

- Public surface: Level B-only in-place
  `arraydrop(array=.string[] expose) -> .int`, used by the string collection
  classes. It clears the array and returns zero. There is no Level C BIF or
  class adapter for this library procedure.
- Correctness/error finding: the typed exposed-array contract and behavior are
  correct. Clearing empty or already-cleared arrays is a successful idempotent
  operation; there are no value-domain arguments and no signal case.
- Performance baseline: current output is the optimal 22/22 lines and one
  `setattrs array,0` instruction, with no loop, calls, or copied elements.
  Preserve the body.
- Test/doc finding: the broad harness checks one populated clear only. Add a
  focused empty/populated/idempotent/reuse test, stable Level B Markdown, and
  RexxDoc. Existing language-book rows already distinguish the string and
  object variants.
- Completed result: the typed exposed-array surface and one-instruction body
  are unchanged. Focused coverage now proves empty/populated clear,
  idempotence, zero return/high-water, and reuse after clear. Stable Markdown
  and RexxDoc document the string-specific helper.
- Focused validation and second review: both overlays report `PASS: arraydrop`;
  output remains the optimal 22/22 lines with one `setattrs`, no loop, and no
  call/copy. `git diff --check` passes and compiler/interpreter diffs remain
  empty.

### Selector inventory and evidence — row 73 `arrayhi`

- Public surface: Level B-only in-place helper
  `arrayhi(array=.string[] expose, mode=.string optional,
  newhi=.int optional) -> .int`. Omitted/`GET` returns the high-water mark;
  case-insensitive `SET` can shrink to a positive smaller mark. Same/larger or
  non-positive requested marks are documented no-ops. There is no Level C BIF
  or class adapter.
- Correctness/error finding: types are correct, but all unknown modes silently
  act as GET, SET without a supplied mark silently uses zero, and GET silently
  ignores a supplied mark. Preserve the documented no-op range policy, while
  signalling `INVALID_ARGUMENTS` for invalid mode/argument combinations using
  optional-presence checks. Those signals share the exposed-array unwind ABI.
- Performance baseline: current output is 101 unoptimized and 100 optimized
  lines. Every call invokes/expands `upper(mode)`, even the dominant omitted
  GET form. Return `array[0]` immediately when mode is omitted, use one direct
  `strupper` only for explicit mode, cache high-water, and retain one
  `setattrs` only for an actual shrink.
- Test/doc finding: the broad harness covers omitted GET, one shrink, and one
  ignored growth. Add focused empty/explicit/lowercase GET, actual shrink,
  same/grow/non-positive no-ops, invalid mode, missing/extraneous mark, and
  preservation tests; add stable Level B Markdown and RexxDoc.
- Completed result: optional-presence checks now distinguish omitted GET,
  explicit GET, and SET. Invalid modes, SET without a mark, and GET with a mark
  signal `INVALID_ARGUMENTS`; documented range no-ops remain unchanged. The
  omitted GET path returns before folding, explicit mode uses direct
  `strupper`, high-water is cached, and only an actual shrink uses `setattrs`.
- Focused validation and second review: both overlays report `PASS: arrayhi`
  for empty/omitted/explicit/case-insensitive GET, shrink/no-op SET, invalid
  mode/combinations, and data preservation. Output is 142/142 lines versus
  101/100 because three error contracts are now explicit, while the dominant
  omitted GET eliminates the old `upper` call and actual mutation remains one
  instruction. Stable Markdown/RexxDoc are complete; `git diff --check` passes
  and compiler/interpreter diffs remain empty. B/V closure is parked only on
  the shared exposed-argument signal unwind ABI.

### Selector inventory and evidence — row 74 `arraymove`

- Public surface: Level B-only in-place string-array helper
  `arraymove(array=.string[] expose, from=.int, count=.int, to=.int) -> .int`.
  It moves a block within the same one-based array and returns the unchanged
  high-water mark. No Level C BIF or class adapter exists.
- Correctness/error finding: all three controls are documented required but
  currently have defaults; invalid source/count values silently no-op and a
  low destination silently clamps. Make controls required and signal source or
  destination below one and negative count. Count zero, empty/source-high, and
  destinations within/immediately after the block remain no-ops; clamp source
  count and high destination with overflow-safe comparisons. Signals share the
  exposed-array unwind ABI.
- Performance baseline: current output is 293/293 lines. It copies every
  string into a temporary array, deletes the source, inserts a gap, then copies
  every string again—two `scopy` operations per value plus temp growth. Use the
  proven insert-destination-first transformation from `objectarraymove`, then
  link non-overlapping source/target slots directly for one `scopy` per value,
  with no temporary or per-slot `minattrs`.
- Test/doc finding: the broad array harness exercises only one move helper
  path. Add focused backward/forward/prepend/append/clamp/overlap/no-op/empty/
  error coverage for both index transformations, stable Level B Markdown, and
  RexxDoc; the language-book row already exists.
- Completed result: required typed controls and `INVALID_ARGUMENTS` signals now
  match `objectarraymove`. The destination gap is inserted first, source and
  target are linked directly, and each value receives one `scopy`; one bulk
  delete completes the move. Count/destination clamps and overlap/no-op paths
  avoid the old overflow-prone end calculation.
- Focused validation and second review: both overlays report `PASS: arraymove`
  for backward/forward/prepend/append/clamp/overlap/no-op/empty and all signal
  paths. Output falls from 293/293 to 246/246 lines, eliminates the temporary
  array, both per-value `minattrs`, and the second string-copy pass, retaining
  one `insattrs1` and one `delattrs1`. Stable Markdown/RexxDoc are complete;
  `git diff --check` passes and compiler/interpreter diffs remain empty. B/V
  closure is parked only on the shared exposed-argument signal unwind ABI.

### Selector inventory and evidence — row 75 `stem`

- Public surface: Level B-only `.stem` string-to-string map class plus
  `.stemIterator`, with `get`, `set`, `size`, indexed `key`/`value`/`valueAt`,
  `tails`, `values`, live/snapshot iterators, and compiler property/bracket
  sugar. The Level C `RexxStem` pool-support class is a different API, not a
  same-name Classic BIF, so no Level C split applies.
- Correctness/error finding: `stem.value` documents an internal array index but
  types it as `.string`; change it to `.int`. Indexed access currently permits
  implicit out-of-range array behavior, and the iterator factory accepts every
  non-one flag as live. Signal `INVALID_ARGUMENTS` for out-of-range indexes and
  non-boolean snapshot flags. Use strict string equality for keys. Caught
  method/factory signals need the same final argument-unwind audit as adjacent
  object/array rows.
- Performance baseline: the module emits 1,164 unoptimized and 2,728 optimized
  RXAS lines. Its hot hash loop calls `length` once, then `substr` and `c2d` for
  every codepoint; optimizer expansion more than doubles the module. Replace
  these with direct `strlen`/`strchar`, reduce modulo 256 on every polynomial
  step to avoid integer overflow while preserving the final bucket, and make
  no broader hand-written array RXAS changes contrary to Level B authoring
  guidance.
- Test/doc finding: existing `ts_stem` covers property/bracket lookup, updates,
  Unicode, and live/snapshot iteration, but not empty keys, hash collisions,
  direct indexed methods/types, extracted arrays, or error signals. Enhance the
  existing harness and stable `stem.md`; remove stale promises that hashing
  must later move to native C and document codepoint/modular hashing and the
  actual API/error surface. The library-book stem overview can remain a pointer
  to this stable implementation contract.
- Completed result: `stem.value` now takes the documented `.int` index; all
  indexed methods and the iterator flag signal `INVALID_ARGUMENTS` for invalid
  values, and key comparisons are strict. Hashing uses direct `strlen` and
  `strchar`, reduces each multiplier-31 step modulo 256, and makes no selector
  calls. Stable docs now describe the real API, collision model, codepoint
  policy, independent extracted arrays, and signals without a promised C
  replacement.
- Focused validation and second review: enhanced `ts_stem` passes both modes
  for empty/colliding/Unicode keys, update/size, indexed access, independent
  tails/values, and live/snapshot iteration. Four parameterized error cases
  also pass in fresh optimized/unoptimized VM processes. Unoptimized output is
  1,172 versus 1,164 lines due to the four signals; optimized output falls from
  2,728 to 1,573 lines (42% smaller) by eliminating per-codepoint
  `length`/`substr`/`c2d` expansion. `git diff --check` passes and compiler/
  interpreter diffs remain empty. B/V closure is parked because a second
  caught object-method signal in one optimized process is not reliably handled;
  the isolated contracts themselves are proven.

### Selector inventory and evidence — row 76 `delword`

- Public surfaces: Level B
  `delword(string=.string, start=.int, count=.int optional) -> .string`, plus
  recognized Classic Level C `DELWORD(string, start [,count])` with CheckArgs
  `rANY rWHOLE>0 oWHOLE>=0`. The `.Rexx.delword` method is a Level B adapter.
- Correctness/error result: Level B now distinguishes omitted count from an
  explicit count, signals `INVALID_ARGUMENTS` for non-positive start or a
  supplied negative count, and retains supplied zero as a no-op. The standalone
  Level C entry applies `rANY rWHOLE>0 oWHOLE>=0` through CheckArgs and reports
  the standard count/presence, `40.12`, `40.13`, and `40.14` context errors.
  The class adapter source now preserves count omission; its compiled artifact
  waits for the single deferred classlib rebuild.
- Focused validation: the compact Level B harness passes optimized and
  unoptimized overlays for omission, exact/oversized/zero counts, boundaries,
  leading/multiple/Unicode blanks, empty input, and both signals. The direct
  Level C harness passes both modes for the same values and every documented
  argument error without using the controller.
- Performance and style review: Level B output falls from 435 unoptimized and
  2,156 optimized lines to 355 and 353 respectively. Both implementations use
  direct `fndnblnk`/`fndblnk` scans, compute one deletion span, and extract at
  most one prefix and one tail. Generated RXAS contains no `WORDS`,
  `WORDINDEX`, `WORD`, or `SUBSTR` selector calls. The standalone Level C entry
  is 475/476 lines including context validation and RexxValue construction.
- Level C dependency: the default VM scanner treats all Unicode whitespace as
  blank, which is the current Level B behavior and a useful directly testable
  default. Full configurable Classic compliance still depends on the unbuilt
  `Config_OtherBlankCharacters` service shared by rows 76-82; record that as a
  parked final dependency rather than adding compiler lowering or a controller
  route.
- Documentation result: distinct stable Level B and Level C Markdown now cover
  signatures, omission, errors, whitespace, direct implementation shape, and
  their focused harnesses; RexxDoc is retained and corrected. The existing
  language-book explanation requires no behavior correction.
- Completion summary: algorithm, tests, second review, and documentation are
  complete and independently proven. B/C/V closure is parked only for the
  shared `Config_OtherBlankCharacters` Level C service and the one final
  classlib adapter build/test. Row 77 `word` is the sole active selector.

### Selector inventory and evidence — row 77 `word`

- Public surfaces: Level B `word(string=.string, wordnum=.int) -> .string`, the
  `.Rexx.word(wordnum)` adapter, and implemented common-controller Level C
  `WORD(string, n)` with CheckArgs `rANY rWHOLE>0`.
- Dependencies and consumers: Level B has no import but calls `length` and
  `substr`; direct and transitive consumers include word-family helpers, date
  formatting, ADDRESS setup, quoted-word fast paths, RexxScript, and the class
  adapter. The adapter is already correctly typed and needs no source change.
- Correctness/error result: the typed Level B surface now signals
  `INVALID_ARGUMENTS` for non-positive `wordnum`; an absent word still returns
  empty. The standalone Level C entry applies `rANY rWHOLE>0` and reports the
  standard count/presence, `40.12`, and `40.14` context errors.
- Focused validation: the compact Level B harness passes optimized and
  unoptimized overlays for first/middle/final/absent words, leading/trailing/
  multiple/Unicode blanks, empty/blank-only input, and both invalid-number
  signals. The direct Level C harness passes both modes for those values,
  source non-mutation, and every documented argument error.
- Performance and style review: Level B output falls from the 604-line
  aggregate baseline to 177 lines in both modes. It has one `strlen`, linear
  `fndnblnk`/`fndblnk` scanning only as far as the requested word, and at most
  one `substring`; it contains no helper calls. The direct Level C entry is
  280/286 lines including validation and RexxValue construction and uses the
  same scan/single-extraction shape without a Level B or dispatcher call.
- Level C extraction: the common body is marked deprecated and retained only
  for existing compiler artifacts. The new harness calls the standalone
  qualified entry; compiler lowering remains untouched.
- Documentation result: code-adjacent RexxDoc plus distinct stable Level B and
  Level C Markdown now cover the typed/direct contracts, whitespace behavior,
  errors, implementation shape, and tests. Existing book examples pass in the
  focused harness.
- Completion summary: B/T/P/D work is complete and independently proven; C/V
  closure is parked only on the shared `Config_OtherBlankCharacters` service.
  Row 78 `words` is the sole active selector.

### Selector inventory and evidence — row 78 `words`

- Public surfaces: Level B `words(string=.string) -> .int`, the zero-argument
  `.Rexx.words()` adapter, and implemented common-controller Level C
  `WORDS(string)` with CheckArgs `rANY`.
- Dependencies and consumers: the Level B module has no import but calls
  `length`; ADDRESS setup, date parsing, WORDPOS, RexxScript, and the class
  adapter consume it. The public types and class adapter are already correct.
- Correctness/error result: the typed Level B surface retains its correct zero
  result for empty/blank-only input and has no value-dependent error. The
  standalone Level C entry applies `rANY` and reports standard `40.3`, `40.4`,
  and `40.5` context errors.
- Focused validation: the rewritten Level B harness passes optimized and
  unoptimized overlays for book examples, empty/blank-only/single-word input,
  leading/trailing/repeated blanks, and two Unicode blank classes. The direct
  Level C harness passes both modes for the same values, source non-mutation,
  and every documented argument error.
- Performance and style review: Level B remains the correct single
  allocation-free scan and falls from 106 aggregate RXAS lines to 100 in both
  modes by replacing the only `length` call with `strlen`. The direct Level C
  entry is 162/171 lines including context validation and RexxValue result
  construction; it has the same scan and no Level B or dispatcher call.
- Level C extraction: the common body is marked deprecated and retained only
  for existing generated artifacts. The focused harness calls the new
  standalone qualified entry; compiler lowering is unchanged.
- Documentation result: distinct Level B and Level C Markdown plus corrected
  RexxDoc cover the typed/direct contracts, zero cases, Unicode whitespace,
  standard errors, implementation shape, and test scope.
- Completion summary: B/T/P/D work is complete and independently proven; C/V
  closure waits only for `Config_OtherBlankCharacters`. Row 79 `wordindex` is
  the sole active selector.

### Selector inventory and evidence — row 79 `wordindex`

- Public surfaces: Level B
  `wordindex(string=.string, wordnum=.int) -> .int`, the typed
  `.Rexx.wordindex(wordnum)` adapter, and recognized Level C
  `WORDINDEX(string, n)` with contract `rANY rWHOLE>0`. No common or standalone
  Level C implementation exists.
- Dependencies and consumers: Level B has no import but calls `length`;
  SUBWORD, WORDPOS, WORDREP, ordinary library callers, and the class adapter
  consume it. The signature, return type, and adapter are already correct.
- Correctness/error result: Level B now signals `INVALID_ARGUMENTS` for a
  non-positive `wordnum` while retaining zero for a valid absent word. The
  standalone Level C entry applies `rANY rWHOLE>0` and reports all standard
  count/presence, `40.12`, and `40.14` context errors.
- Focused validation: the rewritten Level B harness passes optimized and
  unoptimized overlays for book positions, first/middle/last/absent words,
  leading/repeated/trailing/Unicode blanks, empty/blank-only input, and both
  signals. The direct Level C harness passes both modes for the same positions,
  source non-mutation, and every documented error.
- Performance and style review: Level B falls from the 138-line aggregate to
  131 lines in both modes even after adding the signal path. More importantly,
  the hot path uses `strlen`, returns immediately after the target `fndnblnk`,
  and performs no target-end scan, substring, allocation, or helper call. The
  direct Level C entry is 235/241 lines including context/RexxValue work and
  preserves the same early-return scan.
- Level C implementation: the standalone entry and qualified harness require no
  controller body and no lowering change.
- Documentation result: RexxDoc and separate Level B/Level C Markdown cover
  typed/direct contracts, one-based Unicode positions, zero versus invalid
  results, context errors, implementation shape, and tests.
- Completion summary: B/T/P/D are complete and independently proven. C/V wait
  only for `Config_OtherBlankCharacters`; row 80 `subword` is the sole active
  selector.

### Selector inventory and evidence — row 80 `subword`

- Public surfaces: current Level B is only
  `subword(string=.string, wordnum=.int) -> .string`, while both stable books
  document `subword(string, start [,count])`. The `.Rexx.subword` adapter also
  omits its documented optional count. Level C recognizes
  `SUBWORD(string, start [,count])` with `rANY rWHOLE>0 oWHOLE>=0`; no Level C
  implementation exists.
- Dependencies and surface result: native Level B and the class adapter now
  expose the documented optional typed count and preserve omission. The adapter
  source waits for the single final classlib build/test.
- Correctness/error result: Level B now signals for non-positive start and a
  supplied negative count, supports explicit zero/omitted/oversized counts,
  excludes outer whitespace, and preserves every internal blank. Standalone
  Level C applies `rANY rWHOLE>0 oWHOLE>=0` with all standard count/presence,
  `40.12`, `40.13`, and `40.14` errors.
- Focused validation: the new Level B and direct Level C harnesses pass both
  modes for the book examples, count omission/zero/one/oversized, absent/empty
  input, internal and outer blanks, Unicode delimiters, source non-mutation,
  Level B signals, and every documented Level C error.
- Performance and style review: Level B falls from the 391-line aggregate to
  304 lines in both modes and replaces WORDINDEX plus general SUBSTR expansion
  with one direct scan and one `substring`. It preserves internal blanks by
  slicing the single original span and never rebuilds per word. Direct Level C
  is 424/427 lines including context/RexxValue work and has the same no-helper,
  single-extraction shape.
- Documentation result: corrected RexxDoc and separate stable Level B/Level C
  Markdown cover the now-implemented signature, omission, blanks, signals,
  context errors, algorithm, and test scope; book examples pass unchanged.
- Completion summary: T/P/D and standalone native/direct work are complete.
  B/C/V closure is parked only for the rebuilt class adapter and shared
  `Config_OtherBlankCharacters`; row 81 `wordlength` is sole active.

### Selector inventory and evidence — row 81 `wordlength`

- Public surfaces: Level B currently declares
  `wordlength(expose string=.string, wordnum=.int) -> .string`, while the stable
  docs and `.Rexx.wordlength` adapter correctly require an integer result.
  Level C recognizes `WORDLENGTH(string, n)` with `rANY rWHOLE>0`; no Level C
  implementation exists.
- Dependencies and surface result: Level B now returns `.int`, takes the source
  by value instead of unnecessary `arg expose`, and the class adapter's public
  `.int` contract matches. Stale aggregate metadata was hidden only during
  focused harness compilation and immediately restored; the final aggregate
  rebuild will publish the corrected signature.
- Correctness/error result: non-positive Level B numbers signal
  `INVALID_ARGUMENTS`, valid absent words return zero, and Unicode character
  length is returned. Direct Level C applies `rANY rWHOLE>0` with standard
  count/presence, `40.12`, and `40.14` errors.
- Focused validation: the rewritten WORDLENGTH-only Level B harness passes both
  modes for book examples, first/final/absent words, Unicode word length,
  empty/blank-only input, and both signals. The direct Level C harness passes
  both modes for the same values, non-mutation, and every documented error.
- Performance and style review: static Level B size grows from a 76-line
  delegating wrapper to 153 lines in both modes, but generated RXAS has no call
  and no substring. It performs one `strlen`, scans only to the requested word,
  and subtracts positions, eliminating WORD allocation plus two length paths.
  Direct Level C is 257/263 lines including context/RexxValue work with the same
  no-allocation core.
- Documentation result: corrected RexxDoc and separate Level B/Level C Markdown
  cover integer/Unicode semantics, zero versus invalid, standard errors,
  no-allocation implementation shape, and focused tests.
- Completion summary: T/P/D and standalone native/direct behavior are complete.
  B/C/V wait only for corrected aggregate/class artifacts and the shared blank
  configuration; row 82 `wordpos` is sole active.

### Selector inventory and evidence — row 82 `wordpos`

- Public surfaces: Level B
  `wordpos(phrase=.string, string=.string [,start=.int]) -> .int`, the
  `.Rexx.wordpos(phrase [,start])` adapter, and recognized Level C
  `WORDPOS(phrase, string [,start])` with `rANY rANY oWHOLE>0`. No Level C
  implementation exists.
- Dependencies and consumer result: direct Level B no longer calls WORDS,
  STRIP, WORDINDEX, WORD, ABBREV, or POS. The `.Rexx.wordpos` source now forwards
  phrase and receiver in the correct order; its compiled artifact waits for the
  final classlib build.
- Correctness/error result: word comparisons are exact and case-sensitive,
  prefix false positives are removed, and arbitrary nonempty Unicode blank runs
  in phrase/target compare as equivalent separators. Non-positive Level B start
  signals `INVALID_ARGUMENTS`; standalone Level C applies
  `rANY rANY oWHOLE>0` with standard count/presence, `40.12`, and `40.14` errors.
- Focused validation: compact Level B and direct Level C harnesses pass both
  modes for exact/case/prefix behavior, single/multiword phrases, repeated and
  Unicode blanks, duplicates and explicit/omitted starts, empty/blank input,
  non-mutation, signals, and every documented Level C error.
- Performance and style review: Level B falls from 566 aggregate RXAS lines to
  492 in both modes. It stops at the requested start, advances candidates by
  word, rejects unequal lengths before codepoint comparison, and allocates no
  substring or normalized full copy; generated RXAS has no helper call. This
  short-phrase-oriented scan avoids the high Level B array cost of a token KMP
  table while removing the prior repeated general-selector rescans. Direct
  Level C is 632/638 lines including context/RexxValue work with the same core.
- Documentation result: corrected RexxDoc and separate Level B/Level C Markdown
  now state exact word semantics, blank normalization, start/errors, direct
  algorithm, and tests; the missing book example is covered.
- Completion summary: T/P/D and standalone native/direct behavior are complete.
  B/C/V wait only for the class adapter artifact and shared configured blanks;
  row 83 `parsecompile` is sole active.

### Selector inventory and evidence — row 83 `parsecompile`

- Public surface: Level B
  `parseCompile(template=.string, expose token=.string[], expose
  token_type=.string[]) -> .int`; there is no Level C BIF contract.
- Co-dependency finding: the returned count plus undocumented numeric token
  types are the direct plan ABI consumed by `parsestring` and `parse`, with
  downstream regex, preprocessor, and process-plugin consumers. A producer-only
  rewrite cannot establish correctness without first freezing and testing that
  shared plan/executor contract.
- Correctness/error finding: unmatched quotes print to stdout and return `-1`
  instead of signalling. Converting that path to a signal also intersects the
  already parked caught-signal `arg expose` array-unwind defect. The source has
  several bespoke whitespace/quote classifications whose intended compatibility
  is described only in comments, not a stable API document.
- Performance/test evidence: the current aggregate is 6,121 RXAS lines and
  repeatedly expands LENGTH/SUBSTR/POS/STRIP/VERIFY while concatenating tokens.
  The sole `ts_parse` coverage is a timing/printing demonstration with no
  assertions, pass marker, error case, or token-plan checks. Source inspection
  is sufficient to reject selector-local optimization without a contract test;
  no code or aggregate build was attempted.
- Completion summary: parked before implementation on the shared legacy
  parsecompile/parsestring plan ABI, assertion harness, and caught-signal expose
  dependency. Resume as one dependency checkpoint while still recording rows
  separately. Row 84 `parsestring` is sole active.

### Selector inventory and evidence — row 84 `parsestring`

- Public surface: Level B `parseString(parse_string=.string, tokenhi=.int,
  token=.string[], token_type=.string[], expose variable=.string[], expose
  variable_content=.string[] [,template=.string])`; it is a procedure with no
  value contract and has no Level C BIF.
- Co-dependency finding: this is the executor for the undocumented numeric plan
  emitted by `parsecompile` and orchestrated by `parse`. Its token kinds,
  variable ordering, absolute/relative cursor rules, delimiter fallback, and
  output-array layout cannot be changed or optimized independently of that plan
  producer and their regex/preprocessor/process consumers.
- Correctness/error finding: there is no argument/plan validation or signal
  contract; unknown kinds and malformed plan arrays can fall through silently.
  The two output arrays are exposed, so new error signalling also intersects the
  shared caught-signal alias-unwind dependency.
- Performance/test evidence: the aggregate is 4,640 RXAS lines. It copies input
  arrays by value, repeatedly materializes the remaining source with SUBSTR,
  then calls POS/LENGTH or performs per-character SUBSTR/POS whitespace loops.
  `ts_parse` provides no assertions for cursor, token-kind, output, invalid-plan,
  or error behavior. There is no stable selector-local Markdown. No code or
  build was attempted because a local rewrite would guess the shared ABI.
- Completion summary: parked on the same legacy plan-contract/assertion harness
  and exposed-array signal dependencies as row 83. Row 85 `parse` is sole active.

### Selector inventory and evidence — row 85 `parse`

- Public surface: Level B `parse(source=.string, template=.string, expose
  variable=.string[], expose variable_content=.string[] [,strip_option=.int,
  case_option=.int]) -> .int`; the two option types are currently only inferred
  from zero defaults. There is no Level C BIF contract.
- Co-dependency finding: this 277-line wrapper constructs the private token
  arrays, calls `parsecompile`, executes them through `parsestring`, and then
  optionally strips/folds every output. Its return/output/error behavior is
  therefore inseparable from rows 83-84 and their downstream regex,
  preprocessor, and process-plugin consumers.
- Correctness/error finding: it does not stop or signal when `parsecompile`
  returns `-1`, does not validate strip/case option ranges, and relies on output
  array `.0` state after every failure. Adding canonical errors must be designed
  with the shared exposed-array signal-unwind repair.
- Performance/test evidence: most cost is delegated to the 6,121/4,640-line
  producer/executor; the wrapper additionally loops over and rewrites every
  output for options. `ts_parse` is still only a print/timing demo, so neither
  wrapper results/options nor failures are asserted. No stable Markdown exists.
  A local wrapper edit would not make the public operation proven or efficient.
- Completion summary: parked with rows 83-84 on the legacy plan ABI, assertion
  harness, option/error contract, and exposed-array signal dependency. Row 86
  `datatype` is sole active.

### Selector inventory and evidence — row 86 `datatype`

- Public surfaces: Level B `datatype(value=.string [,type=.string]) -> .string`,
  `.Rexx.datatype([type])`, and common-controller Level C
  `DATATYPE(string [,type])` with catalog checklist `rANY oABLMNSUWX`.
- Contract conflict: both stable language/class books include option `D`
  (digits), while the authoritative current Level C catalog and CheckArgs option
  set exclude it. This is a public language decision and cannot be guessed in a
  library-only row. The class adapter's unambiguous omission bug was corrected
  in source so omitted type no longer becomes explicit empty; artifact proof is
  deferred.
- Configuration dependencies: exact Classic behavior requires configured extra
  letters/digits, blank grouping, exponent-digit limits, and caller numeric
  context. Those services are absent. The common body currently substitutes
  ASCII sets and permissive B/X blanks; Level B strips all values and uses a
  binary-float tolerance/exponent path, so neither is an acceptable normative
  base for the standalone split.
- Correctness/error finding: Level B treats explicit empty/unknown options as
  false instead of signals and its default/N/W numeric grammars disagree with
  Classic signed/exponent/whole rules. The common Level C W helper accepts only
  signed digits, B/X allow blanks anywhere, and invalid option coverage is only
  in the broad controller test. These routines also underpin CheckArgs BIN/HEX/
  SYM/WHOLE behavior, making a partial fix cross-selector semantics.
- Performance/test evidence: Level B is 4,847 aggregate RXAS lines with repeated
  STRIP/LENGTH/VERIFY/POS/SUBSTR/LASTPOS, reverse per-character extraction, and
  float exponent arithmetic. `ts_datatype` is verbose and tolerance-based;
  `ts_datatype-allchecks` disables nearly all checks. No standalone Level C
  harness or separate stable Markdown exists. No implementation or aggregate
  build was attempted because the missing services and D decision determine the
  correct algorithm and tests.
- Completion summary: parked before implementation on the DATATYPE option-set
  decision and shared Classic character/numeric configuration services. Resume
  it before closing dependent CheckArgs semantics. Row 87 `parseExec` is sole
  active.

### Selector inventory and evidence — row 87 `parseExec`

- Public surface: Level B `parseExec(src=.string, splan=.string,
  template=.string [,debug=.int]) -> .string[]`; there is no Level C BIF
  contract. The source currently infers the debug type from its zero default.
- Co-dependency finding: `splan` is a private length-prefixed stream ABI emitted
  by `compiler/exits/parse/Parse.crexx`. Its item kinds and cursor semantics are
  jointly owned by that producer and this decoder. The user has explicitly
  excluded compiler/lowering changes from this library programme, so the ABI
  cannot be redesigned selector-locally.
- Correctness/error finding: malformed lengths, separators, and item kinds do
  not signal. `expect_char` logs to stdout and advances, while unknown runtime
  kinds optionally log and continue at the old cursor. Standardising those
  failures requires a frozen malformed-plan contract shared with the producer,
  not a guessed library-only rule.
- Performance/test evidence: the aggregate is 10,886 RXAS lines. It first
  decodes the complete stream into parallel arrays, then repeatedly uses
  general LENGTH/SUBSTR/POS selectors and one-character substrings while
  executing it. The compiler-exit runtime suite has over one hundred useful
  end-to-end PARSE assertions, but its plans are generated internally; the
  direct exit harness checks emitted call text rather than `parseExec`'s encoded
  input, malformed-stream behavior, or public callable boundary.
- Completion summary: parked before implementation on the shared stream-plan
  ABI and a direct executor contract harness. Resume without changing lowering:
  first capture representative generated plan strings and malformed-plan
  expectations in a selector-local harness, then optimize the decoder/executor
  while preserving those bytes. Row 88 `getenv` is sole active.

### Active parked co-dependency queue

The 2026-07-14 VM dependency checkpoint removed the shared caught-signal call-
window blocker for 18 selectors and the decimal-conversion signal blocker for
six numeric selectors. The four numeric rows that remain parked below are held
only for their separate `.Rexx` class-adapter work.

- Date/time conversion cluster (`_datei`, `_dateo`, `_jdn`, `date`, and Level C
  `time`): freeze the
  extended Level B format/validation contract separately from Classic Level C
  DATE, add direct calendar/JDN round-trip and invalid-date harnesses, and define
  the frozen-clause-time service before rewriting the shared conversion core.
- `random` service/contract: decide the typed Level B one-argument and explicit
  omission semantics separately from Classic RANDOM, then provide a scoped
  seed/next-value service that can implement unbiased bounded generation and
  the Level C `40.31`/`40.32`/`40.33` contract without VM-global `rand()` state.
- `fnv` hash contract/VM opcode: decide whether the public legacy name preserves
  reverse-order SDBM or migrates to conventional FNV-1a, and repair `rxhash`'s
  Unicode character-count-as-byte-limit defect before freezing vectors. The VM
  change is outside this library-only programme.
- `arraydump`/`arrayformat` shared output contract: freeze one common definition
  of range errors, accepted flags, headers, index padding, quote escaping,
  non-printable rendering, and empty ranges before replacing their duplicated
  formatters. Preserve `arraydump` as the compiler `DUMP ARRAY` exit target and
  do not change that lowering in this library programme.
- Quote-aware parser grammar (`qpos` and dependent `q*` selectors): decide
  whether doubled delimiters escape quotes, how unmatched quotes behave, and
  whether matching is by Unicode character or encoded byte. Also freeze empty
  separators, whether split fields preserve or trim whitespace, start-prefix
  handling, pair-string validation, and mismatched/unclosed nesting. Freeze
  those cases in shared vectors before optimizing the structural scanners; the
  current source comments, tests, and adjacent selectors contradict one another.
- Quote-aware extraction contract (`qextractall`, `qextractpair`, and their
  removal/comment consumers): reconcile the documented start argument with the
  implemented mode argument, define inclusive/exclusive/comment modes, empty
  delimiters and incomplete pairs, and expose the consumed end position so an
  all-pairs scan cannot infer cursor movement from differently shaped results.
- Quote-aware comment contract (`qstripcomment`): define CRLF/LF/CR line ends,
  empty markers, unterminated blocks, nesting, and preservation of line endings
  on top of the shared quote/extraction grammar before changing the parser.
- Quote-aware word-span contract (`qword`, `qwordlength`, `qwords`,
  `qwordindex`, `qwordpos`, and `qsubword`): define whether returned spans keep
  quote delimiters, how doubled/unmatched quotes and attached quoted/unquoted
  text tokenize, what counts as a blank, and invalid index/start signals. Drive
  all six results from one scan so count/index/length/value cannot diverge;
  define qwordpos multiword equality versus the current single-word prefix test.
- FSAY placeholder-expression contract (`fsayfmt` and the existing compiler
  exit consumer): define unmatched/escaped braces, quote-like literal text,
  empty/invalid placeholders, unaligned width semantics, and diagnostic signals
  before changing emitted source. Keep compiler exit/lowering changes out of
  this library programme.
- `parseExec` stream-plan ABI: freeze representative compiler-emitted plan
  bytes and malformed-plan behavior in a direct executor harness before
  changing the decoder or its errors. Keep the producer and compiler lowering
  unchanged during this library programme.

- `delword` configured blank set and class artifact: native Level B and direct
  Level C default-Unicode behavior, errors, tests, docs, and linear algorithms
  are complete. Full Classic Level C compliance shares the unimplemented
  `Config_OtherBlankCharacters` service with rows 77-82. The presence-aware
  `.Rexx.delword` adapter is corrected in source; compile and test it after the
  single deferred classlib build.
- `word` configured blank set: native Level B and direct Level C default-
  Unicode behavior, errors, tests, docs, and scan/slice algorithms are complete.
  Full Classic Level C compliance waits only for the word-family
  `Config_OtherBlankCharacters` service; the class adapter needs no source
  change.
- `words` configured blank set: the native Level B and direct Level C one-pass
  counters, tests, docs, and standard errors are complete. Full Classic Level C
  closure waits only for the same `Config_OtherBlankCharacters` service; the
  class adapter needs no source change.
- `wordindex` configured blank set: native Level B and direct Level C position
  scans, validation, tests, and docs are complete. Full Classic Level C closure
  waits only for the shared `Config_OtherBlankCharacters` service; no class
  adapter source work remains.
- `subword` configured blank set and class artifact: the corrected native
  optional-count implementation, direct Level C entry, tests, docs, and
  single-span algorithm are complete. Full Classic Level C closure waits for
  `Config_OtherBlankCharacters`; compile/test the corrected presence-aware
  `.Rexx.subword` adapter after the single classlib rebuild.
- `wordlength` configured blank set and aggregate metadata: the corrected
  native `.int` no-allocation implementation and direct Level C entry pass all
  focused tests. Rebuild library/classlib once to publish/prove the corrected
  signature and adapter; full Classic Level C closure also waits for
  `Config_OtherBlankCharacters`.
- `wordpos` configured blank set and class artifact: exact native and direct
  Level C matching, validation, tests, docs, and no-allocation algorithm are
  complete. Full Classic Level C closure waits for
  `Config_OtherBlankCharacters`; compile/test the corrected receiver/phrase
  forwarding after the single classlib rebuild.
- `parsecompile` legacy plan ABI: freeze/document token kinds and error
  semantics jointly with `parsestring`, replace the print-only `ts_parse` demo
  with assertions, and repair/prove caught-signal exposed-array preservation
  before rewriting the 6,121-line producer. No selector-local code change or
  aggregate rebuild was made.
- `parsestring` legacy plan ABI: freeze its cursor, delimiter, token-kind, and
  output-array semantics with `parsecompile`; replace `ts_parse` printing with
  assertions and settle exposed-output signal handling before removing its
  repeated remainder copies/per-character selector calls. No code was changed.
- `parse` legacy wrapper contract: define validated strip/case options and
  failure propagation only after the shared parsecompile/parsestring plan and
  output-array contracts have assertion coverage and exposed-signal preservation.
  No selector-local code or build change was made.
- `datatype` Classic configuration/option contract: decide whether `D` joins the
  Level C catalog, then provide extra-letter/digit, blank-grouping,
  exponent-limit, and caller-numeric services before replacing the ASCII/float
  implementations. This checkpoint must also reconcile dependent CheckArgs
  BIN/HEX/SYM/WHOLE behavior. Only adapter omission source was corrected.
- `value` Level C external-pool success paths: define and implement a
  configuration-named pool get/set service. The internal Level C BIF and all
  Level B work are complete; the direct harness currently proves the required
  `40.37` error for every external pool name. This is not blocked on compiler
  lowering.
- `insert` class adapter: `lib/classlib/Rexx.crexx` still forwards legacy `-1`
  sentinels for omitted position and length, which the corrected Level B
  contract properly rejects. At the final integration checkpoint, change the
  adapter to preserve optional-argument presence and correct its receiver/new
  argument order, add a focused class-method test, and validate it with the
  rebuilt classlib. The standalone Level B and Level C implementations and
  their focused tests are complete.
- `overlay` class adapter: `lib/classlib/Rexx.crexx` always forwards its default
  `len=0`, so it cannot distinguish an omitted overlay length from a supplied
  zero after the standard correction. At the final integration checkpoint,
  preserve optional length presence in the adapter and add a focused class
  method test. The standalone B/C implementations and tests are complete.
- `lastpos` class adapter: `lib/classlib/Rexx.crexx` always forwards its default
  `upto=0`, while standard Level B now distinguishes omission from an invalid
  supplied zero. Preserve optional presence and add a focused method test at
  the final classlib integration checkpoint. Standalone B/C work is complete.
- `strip` class adapter: `lib/classlib/Rexx.crexx` always forwards the legacy
  `UTF8WSP` sentinel instead of preserving omission of `char`. At the final
  classlib integration checkpoint, preserve optional char presence, remove the
  sentinel/default from the adapter, correct its char documentation, and add a
  focused method test. Standalone B/C work is complete.
- `substr` class adapter: `lib/classlib/Rexx.crexx` always forwards the legacy
  `len=-256` omission sentinel and pad. At the final classlib integration
  checkpoint, preserve optional length/pad presence, remove the sentinel,
  correct its pad documentation, and add a focused method test. Standalone B/C
  work is complete.
- `translate` configuration range: the standard omitted-input-table path
  requires `Config_Xrange`, but the repository has no Classic coded-character
  configuration service. Row 54 repaired the distinct native byte-domain
  helper without pretending it defines that policy. Resume row 31 only after
  the configuration-range/XRANGE contract is approved and implemented; do not
  preserve the current nonstandard literal-blank fallback.
- `abs` class adapter: `.Rexx.abs` passes Classic string text, including blank-
  separated signs, directly to the native `.decimal` Level B function.
  Normalize the adapter input through the Classic path and add its focused
  method test at the final classlib checkpoint.
- `sign` class adapter: `.Rexx.sign` forwards Classic receiver text directly to
  the native helper. Normalize it through the Classic path and add its focused
  method test at the final classlib checkpoint.
- `trunc` class adapter: `.Rexx.trunc` passes Classic receiver text into the
  typed helper. Normalize it through the Classic path and add a focused method
  test at the final classlib checkpoint.
- `format` class adapter: `.Rexx.format` forwards five concrete defaults and
  therefore loses Classic omission. Preserve option presence and add its
  focused method test at the final classlib checkpoint.
- `b2d` class adapter metadata: the native one-argument unsigned converter and
  its boundary/error harness are complete. `lib/classlib/Rexx.crexx` now
  correctly declares `.Rexx.b2d() -> .int`, but the current aggregate classlib
  still advertises the old `.rexx` return. Prove the adapter after the single
  deferred classlib rebuild rather than rebuilding it during this row.
- `c2x` configuration encoding: native Level B C2X and its tests/docs are
  complete with the established `hexchar` low-byte behavior. Classic Level C
  instead requires configuration-coded character-to-bits conversion. The
  provisional low-byte direct implementation was removed after consulting the
  authoritative conversion table; implement and test direct C2X only after the
  shared `Config_C2B` service is defined.
- `c2d` configuration encoding: native Level B single-code-point C2D is
  complete and independently tested. Classic Level C `C2D(string [,length])`
  additionally requires configuration-coded character-to-bits conversion,
  signed twos-complement width handling, caller `NUMERIC DIGITS`, and `40.35`.
  Implement its standalone direct BIF only after the same `Config_C2B` service
  parked by C2X is defined; do not substitute the Level B code-point helper.
- `d2c` configuration encoding: native Level B Unicode-scalar D2C and its
  tests/docs are complete. Classic Level C `D2C(number [,length])` requires the
  shared `Config_B2C` bits-to-coded-character service, including configured
  zero/high padding characters and signed twos-complement width. Implement its
  standalone direct BIF and harness after that service exists; do not substitute
  Unicode `appendchar`.
- `x2b` class adapter artifact: native Level B and direct Level C X2B plus all
  standalone tests/docs are complete. `.Rexx.x2b()` no longer advertises or
  forwards the old unused length, and its focused class assertions include
  leading-zero preservation. Compile and run that adapter after the single
  deferred classlib/library rebuild; do not rebuild the aggregate for this row.
- `x2c` configuration encoding: native Level B Unicode U+00xx X2C and its
  direct UTF-8 tests/docs are complete. Classic Level C `X2C(hex)` shares the
  completed HEX validator but must convert through `Config_B2C`. Implement its
  standalone BIF and harness after that service exists; do not substitute the
  native Unicode-byte mapping.
- `x2d` class adapter artifact: native Level B and direct Level C X2D plus all
  standalone tests/docs are complete. `.Rexx.x2d([length])` now preserves
  omission instead of forwarding the old `-1` sentinel, and its focused class
  assertions cover omitted, signed, and explicit-zero widths. Compile the
  adapter and run the tagged class examples after the single deferred
  classlib/library rebuild.
- `xrange` configuration and class artifact: the native byte-domain helper and
  its standalone tests/docs are complete, while direct Classic XRANGE remains
  blocked on the undefined `Config_Xrange` coded-character service shared with
  TRANSLATE. `.Rexx.xrange` now forwards correctly in source; compile and run
  its focused assertion after the single deferred classlib rebuild.

## Programme benchmark gates

Step -1 evidence: [Level B library programme benchmark baseline —
2026-07-13](levelb-library-benchmark-baseline-2026-07-13.md).

### Step -1 — pre-change baseline

Before any selector implementation edit:

1. Record the exact commit, branch, worktree state, platform/architecture, CPU
   count, build type, compiler/tool versions, and benchmark commands.
2. Use the existing five-program `tests/benchmarks` language suite as an
   overall sanity baseline. Its RexxCPS Level B adaptation supplies the closest
   current foundation-library workload; this programme does not create a new
   performance-management framework or selector-by-selector benchmark suite.
3. Build the benchmark artifacts once in the selected Release configuration
   and use the runner's existing warm-up, repetition, correctness, and
   min/median/mean/max reporting policy.
4. Keep benchmark output in a timestamped Markdown evidence report under
   `docs/planning/release-1`; do not infer performance from source shape.
5. Treat the figures as coarse regression evidence only. Focused measurements
   may still be added under a selector's P gate when source inspection cannot
   settle a performance question.

### Step 8 — final comparison and report

After all 122 selector rows are done:

1. Rerun the same five-program step -1 command on the same platform, build
   type, workloads, input sizes, warm-up policy, and repetition count.
2. Report before and after absolute results, percentage deltas, variability,
   and any benchmark or environment caveat.
3. Call out improvements, neutral results, and regressions separately. A
   regression is not hidden by an aggregate improvement.
4. Relate measured changes to the completed selector work only where evidence
   supports that attribution.
5. Save the final Markdown comparison report, link it from this ledger, and do
   not mark the programme complete until the report and final validation pass.

## Approved Level C library boundary

The Level C direction is fixed for this programme:

- Each applicable selector gains a standalone, directly callable
  `lib/rxfnsc` BIF implementation over `RexxValue` and the shared Level C
  execution/error support required by its exact contract.
- Level C validation is performed through focused optimized and unoptimized
  library harnesses. This programme does not change `rxc`, its lowering tables,
  target-shape tests, or other compiler artifacts.
- The current `rexxclassicbif_call` path remains as a deprecated compatibility
  artifact for existing compiler output. It is removed only after a later bulk
  Level C lowering change makes emitted code call the specific functions.
- RexxScript owns its sandbox allow-list and name controller and may call the
  same specific `rxfnsc` exports. Its controller is not the Level C library API.
- Shared `rxfnsc` machinery such as `RexxValue`, argument-presence frames,
  caller state, variable-pool references, `CheckArgs`, and standard `RXC-LC-*`
  error construction is support infrastructure.
- A BIF with fixed arity may have a compact direct signature. Optional,
  omitted, variadic, or stateful BIFs must retain full Classic invocation
  semantics through explicit arguments or a shared call frame.

### Compatibility boundary evidence — 2026-07-13

- All task-local compiler code, test, target-shape, and compiler-documentation
  edits were cleared before continuing row 1.
- `rxc`, `rxas`, `rxlink`, `rxvm`, the Level B library, `rxfnsc`, and the row-1
  focused harness artifacts were rebuilt successfully from the restored
  compiler sources.
- `rexxclassicbif_call` is retained and marked deprecated for current compiler
  compatibility. New Level C row tests call the specific BIF function directly.
- Broad CTest sweeps are deferred until the complete work list; each row uses
  only its focused library build and harness commands.

## Completion gates

- **B** — Level B review and implementation: correct typed arguments and return
  type, efficient algorithm, canonical signal-based failures, preserved or
  updated RexxDoc, and no accidental Classic/Level C coercion semantics.
- **C** — Separate Level C implementation when applicable: a standalone,
  directly exported `lib/rxfnsc` function over `RexxValue` and the necessary
  Level C execution context, with Classic argument-presence and numeric-context
  rules and standard `RXC-LC-*` error handling. Do not change compiler lowering
  in this programme. Same-named B and C functions remain separate contracts
  even if a private algorithm can safely be shared.
- **T** — Tests: review and simplify or extend Level B tests; add a focused
  direct Level C library harness where applicable; include success, boundary,
  omitted/default argument, type, signal/error, and documented examples.
  Exercise optimized and non-optimized paths where registered.
- **P** — Independent second performance/style review after the implementation
  passes tests: re-read the algorithm as a foundation hot path, inspect
  allocation/scanning/copy behavior and optimized RXAS only when meaningful,
  and add a repeatable focused benchmark only when source inspection is
  insufficient.
- **D** — Documentation: preserve current Markdown organization and keep Level
  B and Level C material separate. Use existing Level B book pages or
  `lib/rxfnsb/rexx/*.md` module pages as applicable, and keep direct Level C BIF
  contracts under `lib/rxfnsc/*.md`. Compiler documentation is outside this
  programme. Do not use generated TeX or the mixed legacy `docs/bifs/*.rexx`
  material as authoritative current documentation.
- **V** — Final validation: focused build/test commands pass, documented
  examples run, `git diff --check` passes, and the active selector has no
  unexplained regression or undocumented contract change.

Checkboxes are deliberately separate. `C = —` means there is no matching
Level C BIF contract in the repository's 70-name recognition ledger.

Level C status annotations in the table are discovery baselines, not completed
work:

- **R** — recognized by the Level C compiler table only;
- **M** — implemented in the current monolithic `RexxClassicBifs.crexx` and
  still requires standalone extraction and direct harness coverage; the
  deprecated compiler compatibility dispatcher remains until the later bulk
  lowering change;
- **L** — a compiled Level C lowering path already exists (`LENGTH` and
  `SUBSTR` only), but this programme changes only the standalone library
  implementation, documentation, and harness coverage.

`ADDRESS` and `TRACE`, marked with `*`, are semantic associations rather than
same-name Level B exports: `_address.crexx` supplies the ADDRESS runtime
machinery, and `trace.crexx` supplies trace runtime machinery. The proposed
workflow treats their Level C BIF contracts as applicable while those selectors
are active. This association is an approval point before row 1 starts.

## Bootstrap-core candidate; required (87)

| # | Selector | Level C BIF contract/status | B | C | T | P | D | V | Row status |
|---:|---|---|:---:|:---:|:---:|:---:|:---:|:---:|---|
| 1 | `_address` | `ADDRESS` R* | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 2 | `_rxsystem` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 3 | `loadmodule` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 4 | `raise` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 5 | `signal` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 6 | `symbol` | `SYMBOL` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 7 | `trace` | `TRACE` R* | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 8 | `value` | `VALUE` R | ☒ | ☐ | ☒ | ☒ | ☒ | ☒ | parked — external pool service |
| 9 | `version` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 10 | `abbrev` | `ABBREV` M | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 11 | `center` | `CENTER` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 12 | `centre` | `CENTRE` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 13 | `changestr` | `CHANGESTR` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 14 | `compare` | `COMPARE` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 15 | `copies` | `COPIES` M | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 16 | `countstr` | `COUNTSTR` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 17 | `delstr` | `DELSTR` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 18 | `insert` | `INSERT` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | parked — `.Rexx.insert` adapter |
| 19 | `length` | `LENGTH` L | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 20 | `lower` | — (dispatcher compatibility helper only) | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 21 | `overlay` | `OVERLAY` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | parked — `.Rexx.overlay` adapter |
| 22 | `pos` | `POS` M | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 23 | `lastpos` | `LASTPOS` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | parked — `.Rexx.lastpos` adapter |
| 24 | `left` | `LEFT` M | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 25 | `right` | `RIGHT` M | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 26 | `reverse` | `REVERSE` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 27 | `space` | `SPACE` M | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 28 | `strip` | `STRIP` M | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | parked — `.Rexx.strip` adapter |
| 29 | `substr` | `SUBSTR` L | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | parked — `.Rexx.substr` adapter |
| 30 | `substro` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 31 | `translate` | `TRANSLATE` R | ☐ | ☐ | ☐ | ☐ | ☐ | ☐ | parked — `Config_Xrange` service |
| 32 | `upper` | — (dispatcher compatibility helper only) | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 33 | `verify` | `VERIFY` M | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 34 | `_ftrunc` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 35 | `_itrunc` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 36 | `abs` | `ABS` M | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | parked — `.Rexx.abs` adapter |
| 37 | `format` | `FORMAT` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | parked — `.Rexx.format` adapter test |
| 38 | `max` | `MAX` M | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 39 | `min` | `MIN` M | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 40 | `numeric` | `DIGITS` R; `FORM` R; `FUZZ` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 41 | `sign` | `SIGN` M | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | parked — `.Rexx.sign` adapter |
| 42 | `trunc` | `TRUNC` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | parked — `.Rexx.trunc` adapter |
| 43 | `b2x` | `B2X` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 44 | `b2d` | — | ☐ | — | ☒ | ☒ | ☒ | ☐ | parked — rebuilt `.Rexx.b2d` metadata/test |
| 45 | `binary` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 46 | `c2x` | `C2X` R | ☒ | ☐ | ☒ | ☒ | ☒ | ☒ | parked — `Config_C2B` service |
| 47 | `c2d` | `C2D` R | ☒ | ☐ | ☒ | ☒ | ☒ | ☒ | parked — `Config_C2B` service |
| 48 | `d2b` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 49 | `d2c` | `D2C` R | ☒ | ☐ | ☒ | ☒ | ☒ | ☒ | parked — `Config_B2C` service |
| 50 | `d2x` | `D2X` R | ☒ | ☒ | ☒ | ☒ | ☒ | ☒ | done |
| 51 | `x2b` | `X2B` R | ☐ | ☒ | ☒ | ☒ | ☒ | ☐ | parked — rebuilt `.Rexx.x2b` adapter test |
| 52 | `x2c` | `X2C` R | ☒ | ☐ | ☒ | ☒ | ☒ | ☒ | parked — `Config_B2C` service |
| 53 | `x2d` | `X2D` R | ☐ | ☒ | ☒ | ☒ | ☒ | ☐ | parked — rebuilt `.Rexx.x2d` adapter/examples |
| 54 | `xrange` | `XRANGE` R | ☐ | ☐ | ☒ | ☒ | ☒ | ☐ | parked — `Config_Xrange` and rebuilt adapter |
| 55 | `arrayfind` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 56 | `splice` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 57 | `arrayinsert` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 58 | `arraydelete` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 59 | `arrayappend` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 60 | `arrayprepend` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 61 | `objectarrayinsert` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 62 | `objectarraydelete` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 63 | `objectarrayappend` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 64 | `objectarrayprepend` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 65 | `objectarraydrop` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 66 | `objectarraymove` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 67 | `arrayget` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 68 | `arrayset` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 69 | `arraycontains` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 70 | `arrayindexof` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 71 | `arraycopy` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 72 | `arraydrop` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 73 | `arrayhi` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 74 | `arraymove` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 75 | `stem` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 76 | `delword` | `DELWORD` R | ☐ | ☐ | ☒ | ☒ | ☒ | ☐ | parked — configured blanks / rebuilt adapter |
| 77 | `word` | `WORD` M | ☒ | ☐ | ☒ | ☒ | ☒ | ☐ | parked — configured blanks |
| 78 | `words` | `WORDS` M | ☒ | ☐ | ☒ | ☒ | ☒ | ☐ | parked — configured blanks |
| 79 | `wordindex` | `WORDINDEX` R | ☒ | ☐ | ☒ | ☒ | ☒ | ☐ | parked — configured blanks |
| 80 | `subword` | `SUBWORD` R | ☐ | ☐ | ☒ | ☒ | ☒ | ☐ | parked — configured blanks / rebuilt adapter |
| 81 | `wordlength` | `WORDLENGTH` R | ☐ | ☐ | ☒ | ☒ | ☒ | ☐ | parked — metadata / configured blanks |
| 82 | `wordpos` | `WORDPOS` R | ☐ | ☐ | ☒ | ☒ | ☒ | ☐ | parked — configured blanks / rebuilt adapter |
| 83 | `parsecompile` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — legacy parse plan ABI |
| 84 | `parsestring` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — legacy parse plan ABI |
| 85 | `parse` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — legacy parse plan ABI |
| 86 | `datatype` | `DATATYPE` M | ☐ | ☐ | ☐ | ☐ | ☐ | ☐ | parked — Classic config / `D` decision |
| 87 | `parseExec` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — stream-plan ABI/harness |

## B standard; default (35)

| # | Selector | Level C BIF contract/status | B | C | T | P | D | V | Row status |
|---:|---|---|:---:|:---:|:---:|:---:|:---:|:---:|---|
| 88 | `getenv` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 89 | `linesize` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 90 | `filter` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 91 | `sequence` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 92 | `find` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 93 | `index` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 94 | `_datei` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — shared DATE contract |
| 95 | `_dateo` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — shared DATE contract |
| 96 | `_jdn` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — DATE error translation |
| 97 | `date` | `DATE` R | ☐ | ☐ | ☐ | ☐ | ☐ | ☐ | parked — DATE core/clause time |
| 98 | `random` | `RANDOM` R | ☐ | ☐ | ☐ | ☐ | ☐ | ☐ | parked — RNG service/contract |
| 99 | `reradix` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 100 | `time` | `TIME` R | ☒ | ☐ | ☐ | ☒ | ☒ | ☐ | parked — Level C clause-time service |
| 101 | `fnv` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — hash contract/VM Unicode |
| 102 | `arraypop` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 103 | `arrayshift` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 104 | `arrayreverse` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 105 | `arrayjoin` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 106 | `arraysort` | — | ☒ | — | ☒ | ☒ | ☒ | ☒ | done |
| 107 | `arraydump` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — shared formatter contract |
| 108 | `arrayformat` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — shared formatter contract |
| 109 | `qpos` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — shared quote grammar |
| 110 | `qsplit` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — quote/field grammar |
| 111 | `qsplitsafe` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — quote/nesting grammar |
| 112 | `qextractall` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — extraction ABI/grammar |
| 113 | `qextractpair` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — extraction modes/grammar |
| 114 | `qstripcomment` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — comment/quote grammar |
| 115 | `qremoveall` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — extraction bounds/grammar |
| 116 | `qword` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — shared qword grammar |
| 117 | `qwordlength` | — | ☐ | — | ☒ | ☒ | ☒ | ☐ | parked — shared qword grammar |
| 118 | `qwords` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — shared qword scanner |
| 119 | `qwordindex` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — shared qword scanner |
| 120 | `qwordpos` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — match/qword grammar |
| 121 | `qsubword` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — shared qword scanner |
| 122 | `fsayfmt` | — | ☐ | — | ☐ | ☐ | ☐ | ☐ | parked — FSAY/quote grammar |

## Active-item evidence record

### 1. `_address` — done 2026-07-13

- Source/public surface: `lib/rxfnsb/rexx/_address.crexx`; 54 namespace exports
  and 129 catalogued public class/interface members covering command/function
  dispatch, environment registration, requests, responses, bindings,
  sandboxes, stems, redirects, providers, and compatibility wrappers.
- Dependencies and known consumers: the compiler ADDRESS exit, Level B file and
  runtime code, native host callbacks, and `interpreter/rxvml.c`. The native
  bridge depends on the attribute ordering of the request/response, binding,
  standard sandbox, and standard stem classes; that layout was preserved.
- Level B result: iteration cursors and native stem counts now use `.int`;
  registry/key normalization is performed once on insertion/search rather than
  for every stored entry; blank driver/environment names and negative stem
  counts use the `INVALID_ARGUMENTS` signal. Existing response-based provider
  failures remain response values.
- Level C result: `RexxClassicBifAddress.crexx` is a standalone direct
  `RexxValue` implementation of `ADDRESS([option])`. `RexxClassicState.crexx`
  and `RexxVariablePool` hold the caller activation's environment and I/O/E
  connection state. The compiler was not changed.
- Tests: the existing `ts_address` integration matrix was retained;
  `ts_address_protocol.crexx` adds pure Level B protocol/signal coverage; and
  `testRexxClassicBifAddress.crexx` directly covers every option, defaults,
  custom/inherited state, omission, formatting, and all documented Level C
  errors. Each focused harness passed in opt and noopt mode.
- Documentation: Level B protocol documentation is
  `lib/rxfnsb/rexx/_address.md`; the distinct Level C BIF contract is
  `lib/rxfnsc/address.md`. Their examples are exercised by the focused
  harnesses.
- Performance/style review: native ABI layout was rechecked; stored registry
  entries are no longer repeatedly normalized; normalized sandbox/stem keys
  are reused; stem-count coercion occurs once; and the Level C RXAS calls the
  direct ADDRESS function, shared validator, and pool accessors without the
  compatibility name dispatcher. No focused benchmark was justified for this
  control-plane row.
- Validation: the restored compiler/tool binaries and both library images
  rebuilt; `ts_address_protocol` noopt/opt, `ts_address` noopt/opt,
  `testRexxClassicBifAddress` noopt/opt, and the compatibility BIF harness
  passed. `git diff --check` passed and `git diff -- compiler` is empty. Broad
  CTest is deferred to programme completion as requested.
- Completion summary: all B/C/T/P/D/V gates are complete; row 2 is now the
  sole active item.

### 2. `_rxsystem` — done 2026-07-13

- Source/public surface: `lib/rxfnsb/rexx/_rxsystem.crexx`, 102 lines and three
  namespace exports: `_open`, `_close`, and `_exit`.
- Dependencies and known consumers: `fileio.crexx` calls `_open` and `_close`;
  no repository call site for the Level B `_exit` helper was found. The module
  uses inline `fopen`, `fclose`, and `exit` assembler operations and maintains
  exposed filename/id/mode cache arrays.
- Level B result: `_exit` now has an optional integer return code and emits it
  directly; `_close` retains its compatible optional `nomsg` position as a
  validated `.int` 0/1 flag; invalid names/modes use `INVALID_ARGUMENTS`; and
  invalid close state/native close failures use `NOTREADY`. `_open` validates
  its dependency-free lowercase mode tokens, canonicalizes `w`/`a` before the
  scan, reuses the first empty slot, clears mode-change state safely, and never
  caches a failed OS open. Operational open failure remains the documented zero
  result required by current `fileio` consumers.
- Level C contract and current implementation status: none in the repository's
  Level C BIF ledger (`C = —`).
- Tests: `ts_rxsystem.crexx` now covers cached handle reuse, canonical modes,
  mode changes, data preservation, failed-open behavior, invalid-argument and
  `NOTREADY` signals, and silent cleanup. `ts_rxsystem_exit.crexx` verifies the
  exact process exit code. Both run in opt/noopt mode; the affected
  `ts_fileio_chars` consumer matrix also passes.
- Existing Level C tests: not applicable.
- Documentation: `lib/rxfnsb/rexx/_rxsystem.md` now documents the distinct
  typed contracts, cache/write semantics, status-versus-signal boundary,
  cleanup flag, example, tests, and performance model; source RexxDoc points to
  it.
- Approved implementation notes or decisions: use the programme's typed Level
  B and signal-error rules; preserve caller-visible file semantics unless the
  focused review proves a defect.
- Performance/RXAS review evidence: the cache remains allocation-free and
  linear over the normally small open-handle set; canonicalization occurs once;
  failed handles are not retained; and the first free slot is reused. RXAS
  metadata confirms `_exit(?return_value=.int)`, `_open(.string,.string) ->
  .int`, and `_close(.string,?nomsg=.int) -> .void`.
- Validation: focused builds succeeded; `ts_rxsystem` noopt/opt,
  `ts_rxsystem_exit` noopt/opt with exact expected code 7, and
  `ts_fileio_chars` noopt/opt passed. `git diff --check` and the empty compiler
  diff passed.
- Completion summary: all applicable B/T/P/D/V gates are complete; no Level C
  contract applies. Row 3 is now the sole active item.

### 3. `loadmodule` — done

- Source/public surface: `lib/rxfnsb/rexx/loadmodule.crexx`, 35 lines and one
  namespace export, `loadmodule(module_name=.string) -> .int`.
- Dependencies and known consumers: a single `metaloadmodule` VM instruction;
  dynamic-load compiler/VM regression programs and the explicit late-load
  guidance in `RXVM_INTERPRETER.md`, `CREXX_LIBS.md`, and the running guide.
- Current Level B signature/error/algorithm findings: the public argument and
  result are correctly typed and the implementation is constant-size. It
  returns the VM module number or a non-positive failure status. Blank names,
  missing files, and invalid artifacts deliberately use that status protocol;
  adding a signal would contradict `METALOADMODULE`.
- Level C contract and current implementation status: none in the repository's
  Level C BIF ledger (`C = —`).
- Existing Level B tests: no selector-local harness; dynamic-load integration
  coverage exists under compiler/VM tests.
- Existing Level C tests: not applicable.
- Existing documentation: high-level late-load behavior is documented in three
  existing Markdown guides, but no Level B selector page or source RexxDoc
  contract exists.
- Approved implementation notes or decisions: preserve the VM late-load
  protocol and keep the wrapper constant-size; use canonical signals only where
  the VM/status contract permits.
- Focused validation commands/results: compiled only `loadmodule`, its provider,
  and `ts_loadmodule` in isolated noopt/opt staging overlays. Both runs printed
  `PASS: Level B loadmodule` and returned zero. The test covers successful
  explicit loading plus missing and blank path statuses. Aggregate CMake wiring
  is recorded in the deferred integration queue.
- Performance/RXAS review evidence: the generated metadata is
  `loadmodule(module_name=.string) -> .int`, followed by one
  `metaloadmodule r0,r1`. A second source/VM review found no wrapper-side scan,
  copy, conversion, allocation, or redundant work; loading and dirty-checked
  relinking are the required VM operation after an explicit successful call.
- Completion summary: B/T/P/D/V are complete; no Level C contract applies.
  `git diff --check` and the empty compiler diff pass. Row 4 is now active.

### 4. `raise` — done

- Source/public surface: `lib/rxfnsb/rexx/raise.crexx`, 38 lines and one
  internal `_rxsysb` export, `raise(type=.string, code=.string,
  parm1=.string) -> .int`.
- Dependencies and known consumers: used by `_datei` to report invalid DATE
  formats; compiler optimizer goldens also contain its currently inlined body.
  It has no Level B library dependency.
- Current Level B signature/error/algorithm findings: the old implementation
  only printed `RUNTIME ... ERROR` and returned zero, so it did not raise a
  runtime error. It now raises a VM signal with the diagnostic payload. Review
  of all callers found both `syntax`/`SYNTAX` and `error`; these are normalized
  to canonical `ERROR`, while unknown dynamic names become
  `INVALID_SIGNAL_CODE`. The compatibility `.int` result remains correctly
  typed and is unused by all known callers.
- Level C contract and current implementation status: none in the Level C BIF
  ledger (`C = —`). This is an internal Level B runtime helper, not the Classic
  REXX `SIGNAL` BIF/statement surface.
- Existing Level B tests: no selector-local harness; `_datei` tests exercise
  normal conversion paths but do not prove signal name or message delivery.
- Existing Level C tests: not applicable.
- Existing documentation: no selector Markdown page or source RexxDoc contract.
- Approved implementation notes or decisions: use the existing dynamic-name VM
  signal instruction and preserve the two message fragments with one
  separating blank. Retain the `.int` compatibility result so stale aggregate
  metadata does not force a bulk rebuild; return zero only after a handler
  explicitly consumes the signal. No compiler change is involved.
- Focused validation commands/results: `ts_raise` passed in isolated noopt and
  opt overlays. VM action handlers prove canonical `ERROR` delivery for
  uppercase/lowercase SYNTAX and lowercase error, exact joined and blank-context
  messages, and `INVALID_SIGNAL_CODE` for an unknown name. Aggregate CMake
  registration is queued.
- Performance/RXAS review evidence: generated metadata remains
  `raise(type=.string,code=.string,parm1=.string) -> .int`. A second source/RXAS
  review confirms bounded compatibility comparisons, no library calls or
  output, at most one message concatenation, and one dynamic `signal`; this is
  an exceptional path rather than a normal hot path.
- Completion summary: B/T/P/D/V are complete; no Level C contract applies.
  `git diff --check`, the empty compiler diff, and clean aggregate test manifests
  pass. Row 5 is now active.

### 5. `signal` — done

- Source/public surface: `lib/rxfnsb/rexx/signal.crexx`, 271 lines; two public
  interfaces (`signal`, `signalaction`), four public implementation/transport
  classes (`standard_signal`, `runtime_signal_raw`, `runtime_signal`,
  `standard_signalaction`), 32 public factories/methods, and three internal
  lookup helpers.
- Dependencies and known consumers: `upper`; the compiler signal exit's
  generated handler wrapper; VM `sigcalla` and `sigbrv` transport; trace and
  debugger surfaces. `runtime_signal_raw`'s five register-mapped slots and
  `runtime_signal`'s raw attachment layout are ABI-coupled to
  `rxsignal_populate_raw_interrupt()`/`rxsignal_populate_runtime_signal()` and
  must retain their order and register views.
- Current Level B signature/error/algorithm findings: public scalar arguments
  and results are typed. Unknown names map to code 13
  (`INVALID_SIGNAL_CODE`), while raising an unknown dynamic name is rejected by
  the VM. The code table now uses static SELECT dispatch. `standard_signal`
  supplies the provider score for the documented `.signal(...)` interface
  factory, while `runtime_signal` has no match hook and remains directly
  constructible by the compiler exit/runtime adapter. The focused harness uses
  the public interface factory for message-only conditions. Source metadata access performs a
  closest-preceding linear scan only when a handler asks for file/line/column or
  source.
- Level C contract and current implementation status: none in the Level C BIF
  ledger (`C = —`); Classic SIGNAL syntax/lowering is a separate compiler-exit
  concern and remains out of scope.
- Existing Level B tests: compiler-exit tests cover raising, handler actions,
  bad handler diagnostics, and block scope; VM RXAS tests cover raw dispatch and
  handler modes. There is no selector-local harness covering the object API,
  complete name/code table, action factories, raw wrapper, or source metadata.
- Existing Level C tests: not applicable.
- Existing documentation: VM and signal/trace working guides describe the raw
  ABI and handler model, but there is no stable Level B selector page and the
  source lacks public RexxDoc blocks.
- Approved implementation notes or decisions: preserve the VM-coupled layouts;
  use the existing C-style static string SELECT for the code table if generated
  RXAS proves the intended dispatch; do not change compiler lowering.
- Focused validation commands/results: `ts_signal` passed in isolated noopt and
  opt overlays with the selector's explicit `upper` dependency. It covers the
  public factory provider, every named VM code plus alias/unknown behavior, all
  action factories, raw slot views, runtime wrapping, payload access, and
  file/line/column/source metadata. Aggregate CMake registration is queued.
- Performance/RXAS review evidence: before change, optimized selector RXAS was
  2,078 lines versus 1,464 noopt and contained no `.jtable`. After the static
  SELECT change it is 1,949 versus 1,391 lines and each optimized inlined
  `code()` path contains one packed `.jtable`/`jumpr` dispatch. A second review
  confirms ordinary accessors remain constant-time and metadata scanning stays
  lazy on the exceptional diagnostic path.
- Completion summary: B/T/P/D/V are complete; no Level C contract applies.
  The VM-coupled raw/wrapper attribute order and register views are unchanged.
  `git diff --check`, the empty compiler diff, and clean aggregate manifests
  pass. Row 6 is now active.

### 6. `symbol` — done

- Source/public surface: `lib/rxfnsb/rexx/symbol.crexx`, 97 lines and one
  `rxfnsb` export, `symbol(inputsymbol=.string) -> .string`.
- Dependencies and known consumers: Level B `lower`, `length`, `pos`, `strip`,
  and `substr`, plus caller-address/source metadata. The Level C contract needs
  `RexxBifCallContext`, `RexxValue`, and the caller `RexxVariablePool`.
- Current Level B signature/error/algorithm findings: the public argument and
  result are typed, and invalid names correctly return `BAD` rather than signal.
  The implementation lowercases twice, strips text that validation later
  rejects, recalculates length in the loop, rebuilds metadata search strings,
  and tests keywords without a leading delimiter (`dress` can match
  `address`). Metadata lookup is a backwards scan bounded by the caller's
  current procedure.
- Level C contract and current implementation status: `SYMBOL(name)` is in the
  repository Level C BIF ledger but has no direct RexxValue implementation.
  ANSI X3J18 requires full Classic symbol recognition, no general reserved-word
  blacklist, and `VAR`/`LIT` classification through the active variable pool,
  including derived compound names. Compiler BIF-name recognition is not the
  implementation and remains unchanged.
- Existing Level B tests: `ts_symbol` covers eight basic VAR/LIT/BAD cases but
  has no pass marker, keyword-boundary regression, empty/dotted cases, or
  focused documentation assertions.
- Existing Level C tests: none.
- Existing documentation: the Level C planning reference has a one-row contract
  and the compiler compliance notes describe Classic symbol/pool rules; there
  are no separate stable Level B and Level C selector pages or source RexxDoc.
- Approved implementation notes or decisions: keep Level B's compiled-symbol
  and keyword semantics distinct; add a standalone Level C module and direct
  harness; add only the variable-pool query needed for non-mutating derived-name
  lookup; do not touch compiler lowering or the deprecated dispatcher.
- Level B result: the implementation now caches the input length, normalizes
  only once, precomputes its metadata needle, and uses exact space-delimited
  keyword matching. This fixes the prior false positive where `dress` matched
  the suffix of `address`; the compiled-symbol/source-metadata semantics remain
  distinct from Classic SYMBOL.
- Level C result: `RexxClassicBifSymbol.crexx` is a standalone direct
  `RexxValue` BIF. It performs Classic symbol recognition without the Level B
  keyword list and asks `RexxVariablePool.symbolHasValue()` for a non-mutating
  resolved scalar/stem/compound binding lookup. The compatibility dispatcher
  and compiler lowering are unchanged.
- Tests and documentation: the revised Level B harness covers visible
  variables, literals, exact and suffix keywords, dotted constants,
  underscores, invalid/empty input, and a pass marker. The direct Level C
  harness covers case folding, Classic contextual keywords, `!`/`?`/`_`,
  stems, substituted compound tails, dropped values, reserved period names,
  constants/exponents, invalid forms, and missing/extra/omitted argument
  errors. Separate stable contracts are in `lib/rxfnsb/rexx/symbol.md` and
  `lib/rxfnsc/symbol.md`; source RexxDoc was added.
- Focused validation commands/results: Level B `ts_symbol` passed in isolated
  noopt and opt overlays with only `raise`, `length`, `lower`, `pos`, and
  `substr`. Direct Level C `testRexxClassicBifSymbol` passed in noopt and opt
  overlays with its six explicit rxfnsc support modules. Generated harness
  RXAS calls `rexxclassicbif_symbol` directly and contains no compatibility
  dispatcher call.
- Performance/RXAS review evidence: the Level B path performs one input scan
  plus its existing backwards metadata lookup and generates 642 optimized
  versus 416 noopt RXAS lines. The first Level C implementation generated
  7,943 optimized lines because repeated Level B string helpers were inlined.
  Replacing those calls with direct `strlen`, `strchar`, and `strupper`
  operations reduced the optimized module to 918 lines, essentially identical
  to its 917-line noopt artifact, while retaining both passing harnesses. Pool
  lookup is hash-based and does not create dropped bindings.
- Completion summary: all B/C/T/P/D/V gates are complete. `git diff --check`,
  the empty compiler diff, clean aggregate test manifests, and the direct-call
  artifact check pass. Row 7 is now the sole active item.

### 7. `trace` — done

- Source/public surface: `lib/rxfnsb/rexx/trace.crexx`, 1,204 lines. It exports
  three classes, 33 namespace selectors, and 70 public class factories/methods;
  the selector is the shared Level B trace/debugger runtime rather than a
  same-name BIF wrapper.
- Dependencies and known consumers: the certified TRACE exit, RXDB/RXDB GUI,
  ADDRESS command hooks, VM breakpoint/signal delivery, and VM module,
  procedure, source-step, trace-event, instruction, and operand metadata. The
  four register-backed fields of `trace_interrupt_raw` are VM ABI coupled and
  must retain their order and register views. String/numeric helpers include
  `upper`, `lower`, `strip`, `left`, `right`, `substr`, `length`, `pos`, `c2x`,
  `d2x`, `x2d`, and `getenv`.
- Current Level B signature/error/algorithm findings: generated metadata shows
  typed integer module/address/count/register arguments and typed string
  names/modes/values, including typed optional defaults. Invalid dynamic modes
  currently pass an empty normalized mode into active trace state, and output
  open failure currently prints instead of signalling. Context construction
  eagerly performs exact-source, closest-source, instruction-decode, and
  procedure backwards scans before the caller knows which view it needs; this
  is the main performance-review target. Status-returning VM metadata/load
  queries must retain their documented status protocol.
- Level C contract and current implementation status: the repository ledger
  defines direct `TRACE([option])` with `oACEFILNOR`, returning the prior
  activation trace setting and applying the optional new option/interactive
  toggle. There is no direct RexxValue implementation or Classic trace state in
  `RexxVariablePool`; compiler TRACE lowering is explicitly outside this
  programme.
- Existing Level B tests: `ts_trace` covers basic controller mode changes,
  option abbreviations, current mode, loaded-module discovery, and a narrow
  source/ASM context path. The compiler-exit suite covers statement generation
  and end-to-end trace modes, but the selector-local harness does not cover
  namespace filters, command-report policy, invalid-mode signals, output
  errors, result-target state, or escaping boundaries.
- Existing Level C tests: none.
- Existing documentation: the Level B runtime is described across
  `CREXX_LIBS.md`, `CREXX_DEBUGGING.md`, `CREXX_TRACE_REQUIREMENTS.md`, and the
  TRACE statement reference, but it has no stable selector-local Markdown page
  or source RexxDoc coverage. The BIF reference still says TRACE() is reserved;
  the Level C planning table supplies the intended direct contract, but there
  is no separate `lib/rxfnsc` BIF page.
- Approved implementation notes or decisions: preserve the VM-coupled raw
  interrupt layout and all status protocols; optimize the event-context hot
  path without changing compiler source or lowering; add a standalone Level C
  state/BIF and direct harness; keep statement extensions out of the Classic
  BIF contract.
- Level B result: typed public metadata and VM status-returning operations were
  retained. Invalid direct `_trace_set` modes now raise `INVALID_ARGUMENTS`
  without changing active state; an output-open failure raises `NOTREADY`.
  Breakpoint contexts now resolve procedure metadata for filtering first and
  prepare source/instruction content only for accepted events. Context metadata
  is cached, exact source is reused as closest source, stored namespace filters
  are no longer repeatedly normalized, component matching avoids temporary
  path strings, and escaping scans/appends code points directly. The raw
  interrupt register layout is unchanged.
- Level C result: `RexxClassicBifTrace.crexx` is a standalone direct
  `TRACE([option])` BIF over `RexxValue`. Per-activation Classic setting and
  interactive state live in `RexxTraceState` through `RexxVariablePool`; child
  pools can copy then independently change the state. Shared CheckArgs now
  implements `oACEFILNOR`, including leading `?`, empty/default Normal, and
  standard `40.28` errors. Statement extensions and compiler lowering remain
  separate and unchanged.
- Tests and documentation: the expanded `ts_trace` covers all implemented mode
  families/aliases, breakpoint policy, format state, both signals, namespace
  component/inclusion rules, escaping including Unicode/control bytes, module
  and procedure status boundaries, and source/ASM/procedure metadata. The
  direct Level C harness covers queries, updates, repeated interactive toggles,
  empty/default behavior, Off reset, state inheritance/isolation, omitted
  arguments, excess arguments, and invalid options. Separate stable contracts
  are `lib/rxfnsb/rexx/trace.md` and `lib/rxfnsc/trace.md`; the public BIF
  reference now records the direct-library/deferred-lowering boundary.
- Focused validation commands/results: `ts_trace` passes in isolated noopt and
  opt overlays with only its explicit Level B dependencies. Direct
  `testRexxClassicBifTrace` passes in noopt and opt rxfnsc overlays. Its RXAS
  calls `rexxclassicbif_trace` directly and contains no compatibility
  dispatcher call.
- Performance/RXAS review evidence: the pre-review optimized aggregate member
  was 56,941 lines. The first lazy/filter/escaping pass produced 48,661 lines;
  preventing repeated expansion of the mode normalizer and using canonical
  fast paths reduced the final optimized overlay to 22,318 lines. Noopt is
  9,102 lines for this 1,200-line, 100-plus-callable runtime. Namespace matching
  and accepted-event preparation are single-pass/cached; rejected events do not
  scan source or decode instructions. No separate timing benchmark was needed
  for the exceptional trace/debug path.
- Completion summary: all B/C/T/P/D/V gates are complete. `git diff --check`,
  the empty compiler diff, clean aggregate test manifests, and direct-call
  checks pass. Aggregate Level C registration remains queued. Row 8 is now the
  sole active item.

### 8. `value` — parked after independently testable work

- Source/public surface: `lib/rxfnsb/rexx/value.crexx`, one
  export, `value(inputstring=.string) -> .string`. The current Level B helper is
  read-only despite the broader Classic BIF name.
- Dependencies and known consumers: Level B `lower`, `upper`, `length`, and
  `pos`; caller-address and source metadata; `metalinkpreg` parent-frame reads;
  and `classlib/Rexx.crexx`. RexxScript's host `value()` method is a separate
  runtime-pool API. The direct Level C contract depends on `RexxValue`, the
  caller `RexxVariablePool`, compound-tail resolution, and eventually external
  pool configuration for the optional third argument.
- Level B result: the public input and result remain explicitly `.string`.
  Empty input returns empty and missing/cleared names return the uppercase
  input. Normalization, the exact metadata needle, and fallback are computed
  once. The backwards scan now stops at `.meta_func`, processes all records at
  that boundary address, and calls `metalinkpreg` only for a register proven to
  belong to the immediate parent frame. Integer, float, and string copies are
  unlinked before return. No error condition is part of this read-only contract.
- Level C result: `RexxClassicBifValue.crexx` is a standalone direct
  `RexxValue` BIF. The internal `rSYM oANY oANY` form resolves scalars, reserved
  variables, stems, substituted compound tails, empty components, and stem
  defaults through `RexxVariablePool`; it returns the prior value and assigns
  argument 2 when present. Shared CheckArgs now implements `SYM` with standard
  `40.26`, reusing the same Classic recognizer as `SYMBOL`. `RexxStem` now owns
  an optional default value. The configuration-backed external pool interface
  does not exist, so supplied argument 3 is validated as `rANY oANY oANY` and
  reports `40.37`; successful external get/set remains parked.
- Tests: `ts_value` is now assertion-only with a pass marker and covers typed
  registers, constants, exact `a`/`aa` names, case, blank/empty values, empty
  and missing names, later visibility, and a procedure-boundary regression.
  The direct Level C harness covers lookup/assignment old values, creation,
  empty values, `.RC`, derived and empty compound components, stem defaults,
  `40.3`/`40.4`/`40.5`/`40.26`, and the parked external `40.37` path. It never
  calls `rexxclassicbif_call`. Moving the Classic recognizer into shared call
  support was regression-tested with the direct SYMBOL harness; the runtime
  pool harness also passes.
- Documentation: Level B source RexxDoc and
  `lib/rxfnsb/rexx/value.md` document the read-only frame contract. The distinct
  direct BIF contract and parked dependency are in `lib/rxfnsc/value.md`; the
  public BIF reference now explains both surfaces and its examples are covered.
- Approved implementation notes or decisions: preserve Level B's read-only
  caller-metadata contract and make matching/boundaries exact; implement and
  test the internal Level C pool form directly; inspect repository host-pool
  interfaces before deciding whether to complete or explicitly park the
  external third-argument portion. Do not change compiler lowering.
- Focused validation commands/results: isolated Level B opt/noopt overlays both
  report `PASS: value`. Isolated direct Level C opt/noopt overlays both report
  `PASS: Level C VALUE BIF`; direct SYMBOL regression overlays report
  `PASS: Level C SYMBOL BIF`; runtime-pool overlays report `ok`. The compiler
  tree and aggregate test manifests remain untouched, and `git diff --check`
  passes.
- Performance/RXAS review evidence: direct VM string normalization replaces
  Level B helper calls, the metadata needle is allocated once, successful
  lookup returns immediately, and the scan has a safe hard procedure boundary.
  The stale pre-row aggregate emitted 427 RXAS lines; the reviewed source emits
  409 noopt and 401 optimized lines. The optimized Level C harness calls
  `rexxclassicbif_value` directly; the standalone BIF is 253 noopt/262 optimized
  RXAS lines and contains no name dispatcher. No focused timing benchmark is
  justified for metadata introspection.
- Completion summary: B/T/P/D/V and the internal Level C implementation are
  complete. The row is parked solely on a future configuration-named external
  pool service; that exact item is in the parked queue. Aggregate registration
  remains in the deferred integration queue. Row 9 is now the sole active item.

### 9. `version` — done

- Source/public surface: `lib/rxfnsb/rexx/version.crexx`, 34 lines and one
  zero-argument export, `version() -> .string`.
- Dependencies and known consumers: one direct `rxvers` VM instruction. The
  benchmark runner records the result as provenance; no library source imports
  are required.
- Level B result: the zero-argument `.string` signature is unchanged. The
  temporary result is now explicitly `.string`; `rxvers` remains the sole
  operation and replaces it with four space-separated fields. The instruction
  has no translated VM signal; allocation failure is fatal rather than a
  recoverable library error.
- Level C contract and current implementation status: no `VERSION` entry exists
  in the repository Level C recognition ledger, so C is not applicable.
- Tests: `ts_version` now asserts exactly four fields, the complete platform
  set, `32`/`64`, the stable `crexx-` prefix, an eight-digit `yyyymmdd` date,
  and repeated-call stability. It no longer embeds the current beta version.
- Documentation: source RexxDoc and `lib/rxfnsb/rexx/version.md` document every
  field, version metadata variability, instruction behavior, and failure
  boundary. The existing public BIF section now gives the exact date form and
  links the selector page; its result shape is covered by the harness.
- Approved implementation notes or decisions: retain the direct VM instruction
  and exact public signature; make the test validate the stable shape rather
  than one development version; add source RexxDoc and a stable Level B page.
- Focused validation commands/results: isolated optimized and unoptimized
  source/test overlays both report `PASS: version`; `git diff --check`, the
  empty compiler diff, and clean aggregate test manifests pass.
- Performance/RXAS review evidence: the implementation is 25 RXAS lines in
  both modes and contains exactly one `rxvers` operation plus return. The
  optimized harness inlines that instruction directly. There is no alternate
  algorithm, scan, conversion, or avoidable allocation to benchmark.
- Completion summary: all applicable B/T/P/D/V gates are complete; C is not in
  the repository ledger. Test registration is queued. Row 10 is now the sole
  active item.

### 10. `abbrev` — done

- Source/public surface: `lib/rxfnsb/rexx/abbrev.crexx`, 51 lines and one
  export, currently `abbrev(string=.string, astr=.string, len=0) -> .string`.
- Dependencies and known consumers: direct `strlen`/`strchar` VM operations;
  `wordpos` calls the Level B helper and date parsing also depends on the
  abbreviation contract.
- Level B result: the logical result is now `.int`, and generated signature
  metadata confirms `string=.string,candidate=.string,?minimum=.int`. Negative
  minimum raises `INVALID_ARGUMENTS`. Source and candidate lengths are computed
  once and checked before any character read; codepoint integers are compared
  directly without the old `itos` conversions. Omitted minimum remains zero,
  which is result-equivalent to the Classic default because the full candidate
  must match.
- Level C result: `RexxClassicBifAbbrev.crexx` is a standalone direct
  `RexxValue` BIF with `rANY rANY oWHOLE>=0`, Classic omitted-minimum behavior,
  early length guards, and direct codepoint comparison. The old controller body
  is explicitly deprecated but retained/exported until final source wiring;
  compiler lowering remains unchanged.
- Tests: the Level B harness now has a pass marker and covers prefix/mismatch,
  short-source safety, exact minimum boundaries, case, empty candidate,
  Unicode, qualified calls, and the negative signal. The focused direct Level C
  harness covers the same value boundaries plus `40.3`, `40.4`, `40.5`,
  `40.12`, and `40.13`; it calls the qualified standalone function and contains
  no compatibility dispatcher reference.
- Documentation: source RexxDoc and `lib/rxfnsb/rexx/abbrev.md` define the typed
  Level B contract and signal. `lib/rxfnsc/abbrev.md` separately defines the
  direct BIF and all errors. The existing book section now corrects its argument
  wording, explains the two omitted-minimum representations, links both pages,
  and its examples are covered.
- Approved implementation notes or decisions: return `.int`, type the minimum
  as `.int`, reject negative Level B lengths with `INVALID_ARGUMENTS`, compare
  codepoints directly after both length guards, create the standalone Level C
  BIF, retain only an unexported deprecated compatibility body in the controller,
  and defer combined CMake/RexxScript wiring.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: abbrev`; isolated direct Level C overlays both
  report `PASS: Level C ABBREV BIF`. Because the corrected `.int` ABI differs
  from the stale aggregate's `.string` metadata, the focused test compiler used
  a temporary selector-only RXAS import index while the aggregate was safely
  moved aside and restored; no aggregate rebuild was performed. `git diff
  --check`, the empty compiler diff, and clean aggregate test manifests pass.
- Performance/RXAS review evidence: Level B drops both per-codepoint `itos`
  operations, guards out-of-range reads, and emits 136 lines in both modes
  versus 141 in the stale aggregate. The direct BIF emits 246 noopt/252 opt
  lines, scans only after both guards, and contains no substring allocation or
  dispatcher call. No timing benchmark is justified for the linear prefix scan.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate and
  RexxScript wiring is queued for the single final integration pass. Row 11 is
  now the sole active item.

### 11. `center` — done

- Source/public surface: `lib/rxfnsb/rexx/center.crexx`, 58 lines and one
  export, `center(expose string=.string, centlen=.int, pad=" ") -> .string`.
  Row 12's separate `centre` source is currently an exact copy and will receive
  its own review next.
- Dependencies and known consumers: Level B `substr` and `copies`, plus direct
  `strlen`, `strchar`, `appendchar`, and integer division instructions. Formatting
  documentation and `fsay` examples use the helper.
- Current Level B signature/error/algorithm findings: width is correctly
  `.int`, but the read-only input is unnecessarily `expose`; padding locals are
  mostly inferred. The implementation silently truncates multi-character pads,
  reads an empty pad at index zero, accepts negative widths, and uses a mutation
  sequence merely to extract the first pad codepoint. Centering/truncation bias
  otherwise matches the documented examples.
- Level C contract and current implementation status: `CENTER(string, length
  [,pad])` is recognized but has no direct implementation. ANSI CheckArgs is
  `rANY rWHOLE>=0 oPAD`; `CENTRE` is a separate recognized alias handled in row
  12. Compiler lowering is out of scope.
- Existing Level B tests: the combined `ts_center` covers fourteen asserted
  CENTER/CENTRE padding and truncation cases, then prints six noisy examples.
  It lacks a pass marker, empty/negative/multi-pad signals, zero width, Unicode,
  and distinct evidence for the two selector rows.
- Existing Level C tests: none.
- Existing documentation: duplicated CENTER and CENTRE book sections cover
  ordinary semantics/examples but do not state Level B types/signals and there
  are no separate selector-local Level B or direct Level C pages/RexxDoc.
- Approved implementation notes or decisions: review/implement only CENTER in
  this row; remove `expose`, retain `.int` width, require exactly one pad
  codepoint and non-negative width via `INVALID_ARGUMENTS`, preserve the current
  left/right bias, add a focused CENTER-only Level B harness and standalone
  Level C BIF. Row 12 may then delegate CENTRE to the reviewed CENTER core.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: center`; isolated direct Level C overlays both
  report `PASS: Level C CENTER BIF`. The Level B harness covers ordinary,
  edge, Unicode, non-mutation, and signal cases. The direct harness covers the
  same result surface plus errors 40.3, 40.4, 40.5, 40.12, 40.13, and 40.23.
  Neither focused path rebuilt an aggregate target.
- Performance/RXAS review evidence: the reviewed Level B implementation emits
  211 RXAS lines in both modes versus 410 in the stale aggregate. It computes
  lengths once, uses direct VM cursor/subslice operations, creates one reusable
  half-padding string, and avoids `substr`/`copies` calls. The standalone Level
  C BIF emitted 307 noopt/301 opt lines before the subsequent CENTRE alias was
  added; the combined module now emits 328/322, with CENTER's algorithm
  unchanged. Its harness calls `rexxclassicbif_center` directly and has no
  dispatcher call.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate wiring is
  queued for the final integration pass. Row 12 is now the sole active item.

### 12. `centre` — done

- Source/public surface: `lib/rxfnsb/rexx/centre.crexx`, 59 lines and one
  export, `centre(expose string=.string, centlen=.int, pad=" ") -> .string`.
  It is a separate copy of the former CENTER implementation.
- Dependencies and known consumers: Level B `substr` and `copies`, plus direct
  string instructions. `.Rexx.centre` delegates to it, the classlib functional
  test exercises that method, and `tests/demo/unicode.crexx` calls it directly.
  Row 11's reviewed CENTER implementation is now an available co-dependency.
- Current Level B signature/error/algorithm findings: width is correctly
  `.int`, but the read-only source is unnecessarily `expose`. Like the former
  CENTER copy, it accepts negative width, silently truncates a multi-character
  pad, reads an empty pad unsafely, allocates through `substr`/`copies`, and has
  mostly inferred locals. The stale aggregate emits 410 RXAS lines.
- Level C contract and current implementation/lowering status:
  `CENTRE(string, length [,pad])` is recognized and has the same
  `rANY rWHOLE>=0 oPAD` contract as CENTER. The repository Level C catalog
  explicitly requires a direct alias rather than duplicated logic. There is no
  standalone entry today; compiler lowering is out of scope.
- Existing Level B tests: the former combined `ts_center` asserted only one
  CENTRE result and printed three more without checking them. Current focused
  `ts_center` deliberately contains no CENTRE call. Broad classlib coverage has
  ordinary widths but no direct signals, explicit empty/multi-pad, or focused
  optimized/unoptimized evidence.
- Existing Level C tests: none.
- Existing Level B docs: the book duplicates the general CENTER prose and four
  mixed-name examples. There is no selector-local page or RexxDoc contract.
- Existing Level C docs/spec clauses: the compiler Level C catalog defines
  CENTRE as CENTER's alternative spelling and direct alias, including the
  one-character pad rule; no standalone Level C page or RexxDoc exists.
- Approved implementation notes or decisions: retain a direct Level B
  implementation using the reviewed CENTER algorithm so the foundation helper
  pays no alias-call overhead; align its types, signals, Unicode behavior, and
  docs with CENTER. Add a CENTRE-only Level B harness. Implement the standalone
  Level C entry as the catalog-required direct alias to
  `rexxclassicbif_center`, with a CENTRE-named direct harness proving results
  and propagated standard errors. Defer aggregate wiring.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: centre`. Isolated direct Level C overlays both
  report `PASS: Level C CENTRE BIF`; the CENTER direct harness was rerun in both
  modes after adding the shared-module alias and still passes. The CENTRE
  harness covers normal, empty, zero-width, Unicode, count/presence, whole,
  negative, and pad cases without rebuilding an aggregate target.
- Performance/RXAS review evidence: Level B uses the reviewed direct algorithm
  and emits 211 RXAS lines in both modes versus 410 in the stale aggregate. The
  shared direct Level C module emits 328 noopt/322 opt lines in total; the
  CENTRE alias itself is only 37/40 lines including metadata and makes one
  direct call to `rexxclassicbif_center`. The harness calls
  `rexxclassicbif_centre` directly and contains no compatibility-dispatcher
  call. Keeping both direct entries in one module avoids the compiler's
  cross-module reference-forwarding mismatch without changing the compiler.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate test wiring
  is queued for the final integration pass. Row 13 is now the sole active item.

### 13. `changestr` — done

- Source/public surface: `lib/rxfnsb/rexx/changestr.crexx`, 45 lines and one
  export, `changestr(expose needle=.string, expose haystack=.string, expose
  nneedle=.string) -> .string`.
- Dependencies and known consumers: Level B `pos` and `substr`, concatenation,
  and direct string lengths. It is widely used by the preprocessor, RexxDoc,
  signal exit, LLM/address demos, process tests, qremoveall, veclib, benchmark
  CSV quoting, and `.Rexx.changestr`.
- Current Level B signature/error/algorithm findings: all three read-only
  inputs are unnecessarily `expose`; their `.string` types are correct and
  there are no value-domain errors. The loop repeatedly searches and rebuilds
  the entire already-modified result through up to two `substr` calls and two
  concatenations per match, producing avoidable quadratic copying. It computes
  replacement length solely to advance across the rebuilt result. The stale
  aggregate emits 819 RXAS lines.
- Level C contract and current implementation/lowering status:
  `CHANGESTR(needle, haystack, replacement)` is recognized with CheckArgs
  `rANY rANY rANY` but has no standalone direct implementation. Compiler
  lowering is out of scope.
- Existing Level B tests: three assertions cover ordinary replacement, two
  matches, and absent needle, followed by one noisy unasserted example. They do
  not cover deletion, expansion, overlapping candidates, empty needle/haystack,
  Unicode, read-only arguments, replacement containing the needle, or a
  repeated workload; there is no pass marker.
- Existing Level C tests: none.
- Existing Level B docs: only a one-line source comment and the classlib method
  RexxDoc exist; the BIF book has a catalog row but no CHANGESTR section or
  selector-local page.
- Existing Level C docs/spec clauses: the repository Level C catalog requires
  replacement of non-overlapping occurrences and calls out empty-needle
  compatibility as unresolved. ANSI draft section 9.3.4 searches only the
  original haystack; via POS section 9.3.16, a null needle is not found, so the
  haystack is returned unchanged. There is no standalone Level C page/RexxDoc.
- Approved implementation notes or decisions: remove `expose`, keep the three
  `.string` arguments, return immediately for empty needle, search the original
  haystack only, and append unmatched slices/replacements into one result using
  direct VM string operations. Preserve case-sensitive, left-to-right,
  non-overlapping semantics even when the replacement contains the needle. Add
  focused Level B and standalone direct Level C tests/docs; defer aggregate
  wiring.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: changestr`; isolated direct Level C overlays both
  report `PASS: Level C CHANGESTR BIF`. The Level B harness covers every book
  and selector-local example plus deletion, expansion, non-overlap, empty
  inputs, Unicode, non-mutation, and 1,000 repeated matches. The direct harness
  covers the result surface and 40.3/40.4/40.5 for all required positions.
  Neither path rebuilt an aggregate target.
- Performance/RXAS review evidence: Level B falls from 819 stale-aggregate RXAS
  lines to 199 in both modes. Its loop has no function calls or concatenation
  expressions: one direct search per match appends an unmatched slice and the
  replacement into a single power-of-two-grown VM string buffer. The direct
  Level C BIF emits 292 noopt/283 opt lines and uses the same algorithm. Its
  harness calls `rexxclassicbif_changestr` directly and has no dispatcher call.
  The second review confirms that only the original haystack is searched, so
  replacement growth cannot alter matching or cause a loop.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate wiring is
  queued for the final integration pass. Row 14 is now the sole active item.

### 14. `compare` — done

- Source/public surface: `lib/rxfnsb/rexx/compare.crexx`, 55 lines and one
  export, `compare(string=.string, astr=.string, pad=" ") -> .int`.
- Dependencies and known consumers: Level B `substr`, direct character/length
  instructions, and padding allocations. `.Rexx.compare` delegates to it and
  has broad ordinary-behavior tests; no other direct Level B consumer was found.
  ObjectComparator/RexxValue comparison methods are different ordering APIs.
- Current Level B signature/error/algorithm findings: the two string types and
  `.int` result are correct, but optional `pad` is inferred and never validated.
  Empty pad can cause unsafe padding behavior and multi-codepoint pad is not
  rejected. The function allocates a fully padded copy of the shorter string
  through `substr`, then scans both strings; locals are largely inferred. The
  stale aggregate emits 612 RXAS lines.
- Level C contract and current implementation/lowering status:
  `COMPARE(left, right [,pad])` is recognized with CheckArgs
  `rANY rANY oPAD` and character, not byte, positions. It has no standalone
  direct implementation; compiler lowering is out of scope.
- Existing Level B tests: eleven assertions cover equality, an internal
  mismatch, blank/default/explicit padding, nonblank padding, both empty and
  ordinary strings, and a last-character mismatch. The file has stale commented
  diagnostics, prints `Look for OK` without an OK line, lacks a pass marker,
  Unicode, invalid-pad signals, source non-mutation, and focused evidence for
  both longer-side directions.
- Existing Level C tests: none.
- Existing Level B docs: only a source label and `.Rexx.compare` RexxDoc exist;
  the BIF book has neither a catalog row nor a COMPARE section, and there is no
  selector-local page.
- Existing Level C docs/spec clauses: the repository catalog and ANSI draft
  section 9.3.5 specify first differing 1-based character position after
  padding the shorter side, or zero. The optional pad must be one character.
  There is no standalone direct Level C page/RexxDoc.
- Approved implementation notes or decisions: retain the typed `.string`,
  `.string`, optional one-codepoint pad, and `.int` result; signal
  `INVALID_ARGUMENTS` for an invalid Level B pad. Compare the common prefix and
  then the longer tail directly against the cached pad codepoint, avoiding all
  padded-string allocation and helper calls. Add focused Unicode, both-side,
  edge, signal/non-mutation tests and a standalone direct Level C BIF/harness;
  defer aggregate wiring.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: compare`; isolated direct Level C overlays both
  report `PASS: Level C COMPARE BIF`. Tests cover every documented example,
  both longer-side directions, empty/default/nonblank/Unicode padding, Unicode
  positions, non-mutation, Level B signals, and Level C 40.3/40.4/40.5/40.23.
  Neither path rebuilt an aggregate target.
- Performance/RXAS review evidence: Level B falls from 612 stale-aggregate RXAS
  lines to 266 in both modes. The generated loop has only direct length and
  codepoint reads: no call, `substr`, or padded string allocation. It scans the
  common prefix once and only the unmatched longer tail thereafter. The direct
  BIF emits 369 noopt/360 opt lines with the same allocation-free algorithm;
  its harness calls `rexxclassicbif_compare` directly with no dispatcher call.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate wiring is
  queued for the final integration pass. Row 15 is now the sole active item.

### 15. `copies` — done

- Source/public surface: `lib/rxfnsb/rexx/copies.crexx`, 36 lines and one
  export, `copies(cstring=.string, count=.int) -> .string`.
- Dependencies and known consumers: only concatenation in the implementation,
  but the public helper is a heavily used foundation dependency across string
  BIFs, formatting, translation, parsers/preprocessor, demos, plugins, tests,
  and `.Rexx.copies`.
- Current Level B signature/error/algorithm findings: source/count/result types
  are correct. A negative count silently returns null rather than signalling.
  The loop creates a new concatenation result once per copy, causing quadratic
  prefix copying for large counts. Locals are inferred. The stale aggregate is
  only 56 RXAS lines, but that compact loop hides the runtime allocation cost.
- Level C contract and current implementation/lowering status:
  `COPIES(string, count)` uses `rANY rWHOLE>=0`. A controller-owned direct body
  currently validates and delegates to Level B `copies`; it is public and used
  by RexxScript and the broad direct harness. Row 15 must extract a standalone
  direct RexxValue entry while retaining the controller body as deprecated
  compiler compatibility until the later lowering initiative.
- Existing Level B tests: seven assertions cover ordinary counts, zero, empty
  source, and a count of ten. They lack a pass marker, count one, Unicode,
  negative signal, non-mutation, non-power-of-two doubling coverage, and a
  substantial result.
- Existing Level C tests: the broad controller-module harness checks only
  `COPIES("abc",3)`. There is no standalone direct harness or argument-error
  coverage; RexxScript has one combined ordinary expression.
- Existing Level B docs: the book has a short semantics paragraph and three
  examples but no typed/error/performance distinction or selector-local page;
  source and classlib RexxDoc are minimal.
- Existing Level C docs/spec clauses: the repository catalog and ANSI draft
  section 9.3.6 specify `rANY rWHOLE>=0`, including zero. Normal string limits
  govern resource exhaustion. There is no standalone Level C page/RexxDoc.
- Approved implementation notes or decisions: retain `.string`, `.int`, and
  `.string`; signal `INVALID_ARGUMENTS` for negative Level B count. Build the
  result with binary decomposition: append the current repeated chunk for set
  bits and double the chunk, reducing append operations from count to O(log
  count) while the VM grows buffers geometrically. Extract a standalone direct
  RexxValue implementation with all standard count errors and focused tests;
  annotate/queue the legacy controller export for final rewiring rather than
  changing compiler lowering now.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: copies`; isolated standalone Level C overlays
  both report `PASS: Level C COPIES BIF`. The tests cover every documented
  example, zero/one/non-power-of-two/Unicode/empty/4,096-copy values, Level B
  negative signalling, and Level C 40.3/40.4/40.5/40.12/40.13. Neither path
  rebuilt an aggregate target.
- Performance/RXAS review evidence: Level B grows from 56 stale-aggregate RXAS
  lines to 146 in both modes because the compact linear loop is replaced by
  binary decomposition. Runtime work is materially lower: a 4,096 count uses
  twelve chunk doublings plus one result append instead of 4,096 growing-prefix
  concatenations. Generated code uses `imod`, `idiv`, and direct `append` only,
  with no function call/concatenation expression. The direct BIF emits 249
  noopt/243 opt lines and the harness calls the standalone qualified entry with
  no dispatcher call.
- Completion summary: all B/C/T/P/D/V gates are complete. The controller body
  is marked deprecated and all aggregate/RexxScript rewiring is queued for the
  final integration pass. Row 16 is now the sole active item.

### 16. `countstr` — done

- Source/public surface: `lib/rxfnsb/rexx/countstr.crexx`, 49 lines and one
  export, `countstr(expose needle=.string, expose haystack=.string) -> .int`.
- Dependencies and known consumers: direct VM lengths/search only. Public use is
  narrow but foundational: `.Rexx.countstr`, its classlib tests, and a process
  plugin test use it.
- Current Level B signature/error/algorithm findings: input/result types are
  correct but both read-only strings are unnecessarily `expose`; integer locals
  are inferred. The algorithm already caches lengths and advances by needle
  length after each direct `strpos`, correctly producing non-overlapping counts.
  It can combine its empty and oversized-needle guards. The stale aggregate is
  106 RXAS lines.
- Level C contract and current implementation/lowering status:
  `COUNTSTR(needle, haystack)` is recognized with `rANY rANY`, including null
  needle returning zero through POS semantics. It has no standalone direct
  implementation; compiler lowering is out of scope.
- Existing Level B tests: thirteen assertions cover repeated/non-overlapping,
  empty inputs, absent/oversized needles, numeric literals converted to text,
  and ordinary sentences. The test is verbose/repetitive and has no pass
  marker, Unicode, boundary-only match, substantial repeated workload, or
  explicit argument non-mutation.
- Existing Level C tests: none.
- Existing Level B docs: source has a one-line comment and the classlib has a
  short method wrapper; the BIF book has neither a catalog row nor a COUNTSTR
  section, and no selector-local page exists.
- Existing Level C docs/spec clauses: the repository catalog and ANSI draft
  section 9.3.7 define non-overlapping counts using POS; POS section 9.3.16 makes
  the null-needle result zero. There is no standalone Level C page/RexxDoc.
- Approved implementation notes or decisions: remove `expose`, make all locals
  explicit integers, retain the direct search/needle-length advance, and return
  early for null or longer-than-haystack needles. Simplify/enhance the focused
  Level B harness and add a standalone direct RexxValue implementation/harness
  with presence errors, Unicode, non-overlap, and substantial repeated input.
  Defer aggregate wiring.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: countstr`; isolated direct Level C overlays both
  report `PASS: Level C COUNTSTR BIF`. Tests cover all documented examples,
  empty/absent/oversized/boundary/non-overlap/Unicode behavior, non-mutation,
  1,000 matches, and Level C 40.3/40.4/40.5. Neither path rebuilt an aggregate
  target.
- Performance/RXAS review evidence: Level B emits 118 lines in both modes
  versus 106 in the stale aggregate; the small increase comes from explicit
  typing and the combined null/oversized guard, not added loop work. Generated
  code still performs exactly two cached lengths and one direct `strpos` per
  match, with no call or substring allocation. The direct BIF emits 208
  noopt/199 opt lines with the same algorithm; its harness calls the standalone
  entry directly and has no dispatcher call.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate wiring is
  queued for the final integration pass. Row 17 is now the sole active item.

### 17. `delstr` — done

- Source/public surface: `lib/rxfnsb/rexx/delstr.crexx`, 46 lines and one
  export, `delstr(string=.string, position=.int, dellen=0) -> .string`.
- Dependencies and known consumers: Level B `length`/`substr` and
  concatenation. `.Rexx.delstr` delegates to it; direct public use is otherwise
  limited to its focused tests in the searched tree.
- Current Level B signature/error/algorithm findings: source/start/result types
  are correct and length infers integer from zero, but the implementation does
  not query optional-argument presence. It therefore treats explicit length
  zero as omitted and deletes through the end, contrary to the standard. It
  silently clamps start below one to one (a commented line records the missing
  error), and negative length also falls into omitted behavior. Multiple
  `length`/`substr` helper calls and concatenation produce 1,066 stale-aggregate
  RXAS lines.
- Level C contract and current implementation/lowering status:
  `DELSTR(string, start [,length])` uses
  `rANY rWHOLE>0 oWHOLE>=0`: omitted length deletes through the end, explicit
  zero changes nothing, and start is 1-based. There is no standalone direct
  implementation; compiler lowering is out of scope.
- Existing Level B tests: seventeen repetitive assertions cover ordinary,
  omitted, oversized start/length, empty text, and start zero. The start-zero
  assertion encodes the nonstandard clamping behavior. Explicit length zero,
  negative signals, Unicode, non-mutation, and a pass marker are missing; four
  stale diagnostics remain commented out.
- Existing Level C tests: none.
- Existing Level B docs: the book has a general section and three examples but
  uses ambiguous parameter names and does not distinguish omitted from explicit
  zero or the Level B signal/type surface. No selector-local page/RexxDoc exists.
- Existing Level C docs/spec clauses: the repository catalog and ANSI DELSTR
  definition specify positive start, optional non-negative length, 1-based
  character indexes, and unchanged output when start is beyond the string.
  There is no standalone Level C page/RexxDoc.
- Approved implementation notes or decisions: retain typed string/start/result,
  use `?delete_length` to distinguish omission from explicit zero, and signal
  `INVALID_ARGUMENTS` for nonpositive start or negative supplied length. Cache
  character length once and construct at most a prefix and suffix through
  direct cursor/subslice/append operations. Replace the noisy test with focused
  standard semantics, signals, Unicode, and non-mutation; add standalone direct
  Level C tests/docs and defer aggregate wiring.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: delstr`; isolated direct Level C overlays both
  report `PASS: Level C DELSTR BIF`. Coverage includes omitted versus explicit
  zero length, middle/prefix/oversized deletion, empty and Unicode strings,
  non-mutation, and the Level B signals plus Level C `40.14`/`40.13` argument
  errors. Neither path rebuilt an aggregate target.
- Performance/RXAS review evidence: Level B emits 234 lines in both modes,
  down from the stale aggregate's 1,066 lines. The hot path caches length once
  and constructs at most one prefix and one suffix using direct `setstrpos`,
  `substring`, and `append` instructions; it makes no Level B helper call. The
  direct BIF emits 352 noopt/349 opt lines, uses the same bounded construction,
  and its harness calls `rexxclassicbif_delstr` directly with no controller.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate wiring is
  queued for the final integration pass. Row 18 is now the sole active item.

### 18. `insert` — parked

- Source/public surface: `lib/rxfnsb/rexx/insert.crexx`, one exported
  `insert` procedure returning `.string`; its source uses untyped optional
  `position`, `len`, and `pad` defaults.
- Dependencies and known consumers: currently calls Level B `length` and
  `substr` repeatedly and concatenates the prefix, insertion, and suffix.
  `.Rexx.insert` delegates to it, but that adapter always supplies legacy `-1`
  defaults and reverses the documented receiver/new roles; this is the exact
  parked integration edge. The focused harness and language-reference examples
  are its other direct consumers.
- Current Level B signature/error/algorithm findings: the implementation uses
  `-1` sentinels instead of optional-argument presence, silently treats every
  nonpositive position as zero, treats negative length as omitted, and replaces
  an empty pad with blank. These behaviours hide invalid arguments. It may
  format the insertion and extend the target through multiple Level B helper
  calls before constructing three concatenated pieces.
- Level C contract and current implementation/lowering status: repository
  catalog marks `INSERT` as required. ANSI section 9.3.11 specifies
  `rANY rANY oWHOLE>=0 oWHOLE>=0 oPAD`, default position zero, default length
  equal to the new-string length, and classic after-character insertion. There
  was no standalone direct implementation; compiler lowering is out of scope.
- Existing Level B tests: twenty numbered/repetitive checks cover common,
  omitted, padded, truncated, extended-target, zero-length, and empty-pad
  cases, but include no signal, Unicode, non-mutation, direct performance, or
  pass-marker coverage.
- Existing Level C tests: none.
- Existing Level B docs: the language-reference section has five examples but
  no selector-local page and does not describe the typed Level B surface or
  signal conditions.
- Existing Level C docs/spec clauses: the compiler catalog has the correct
  checklist and concise semantics but there was no stable selector-local Level
  C page or RexxDoc implementation block.
- Approved implementation notes or decisions: type both optional numeric Level
  B arguments as integers, preserve length presence, signal negative numeric or
  non-single-character pad input, and build the result with cached character
  lengths plus direct VM substring/append/pad operations. Add the standalone
  RexxValue context BIF and focused B/C tests/docs; defer aggregate wiring and
  park the class adapter rather than rebuilding classlib during row processing.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: insert`; isolated direct Level C overlays both
  report `PASS: Level C INSERT BIF`. Coverage includes every documented
  example, omitted option holes, explicit zero, target extension, truncation,
  Unicode position/pad handling, non-mutation, substantial padding, Level B
  signals, and Level C `40.3`/`40.4`/`40.5`/`40.12`/`40.13`/`40.23` errors.
  Neither path rebuilt an aggregate target.
- Performance/RXAS review evidence: Level B emits 368 lines in both modes,
  down from 1,094 stale-aggregate lines. It performs three cached lengths
  (including pad validation), at most two direct substring operations, bounded
  appends, and O(1) VM `padstr` construction; it makes no Level B helper call.
  The direct BIF emits 472 noopt/475 opt lines with the same algorithm, and its
  harness calls `rexxclassicbif_insert` directly with no controller.
- Completion summary: all selector B/C/T/P/D/V gates are complete and aggregate
  wiring is queued. The row is parked only for the `.Rexx.insert` class adapter
  correction and its classlib-level test at integration. Row 19 is now active.

### 19. `length` — done

- Source/public surface: `lib/rxfnsb/rexx/length.crexx`, one exported
  `length(string1=.string) -> .int` procedure.
- Dependencies and known consumers: this is a foundation primitive used
  throughout Level B and classlib. Its body has no library dependency and the
  `.Rexx.length` method delegates directly to it.
- Current Level B signature/error/algorithm findings: argument and result types
  are correct and the implementation is already one direct `strlen`, but the
  input is unnecessarily `expose`d because of an obsolete compiler workaround.
  The stale aggregate is only 31 RXAS lines. Valid `.string` input has no
  source-level error case.
- Level C contract and current implementation/lowering status: `LENGTH(string)`
  is `rANY`. A legacy lowering helper in `RexxClassicBifs` directly accepts one
  `.RexxValue`, while the compatibility controller performs argument checks.
  There is no standalone context-based implementation with standard errors;
  compiler/lowering changes remain out of scope.
- Existing Level B tests: eight repetitive assertions cover empty, short,
  ASCII, quoted Unicode, and an unquoted Unicode symbol, but have no pass marker
  or explicit non-mutation check.
- Existing Level C tests: the broad compatibility harness has one dispatcher
  check, one direct legacy-helper check, and a missing-argument check; there is
  no standalone direct context harness.
- Existing Level B docs: the book has two examples but no selector-local page
  and does not distinguish character length from encoded byte length or state
  the typed surface.
- Existing Level C docs/spec clauses: the compiler catalog records `rANY`,
  character/configuration length, and possible standard `23.1` invalid-data
  handling; no selector-local Level C page exists.
- Approved implementation notes or decisions: remove the obsolete `expose`
  workaround while preserving the typed surface and direct `strlen`; replace
  the repetitive harness with codepoint/non-mutation coverage and a pass
  marker. Add a standalone context-based RexxValue BIF with `rANY` CheckArgs,
  direct tests, and separate B/C docs. Keep and deprecate the existing
  value-only lowering helper; do not change compiler lowering.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: length`; isolated direct Level C overlays both
  report `PASS: Level C LENGTH BIF`. Tests cover empty, ASCII, multibyte
  Unicode, a combining codepoint, non-mutation, and Level C
  `40.3`/`40.4`/`40.5` errors. The direct harness uses the standalone qualified
  namespace and never invokes the controller. Neither path rebuilt an
  aggregate target.
- Performance/RXAS review evidence: Level B emits 37 lines in both modes versus
  31 stale lines; the six metadata/source-step lines accompany RexxDoc and the
  non-exposed argument, while executable work remains exactly one direct
  `strlen` and no call/allocation. The direct context BIF emits 105 noopt/114
  opt lines and adds only context validation/extraction plus result materializing
  around the same single `strlen`.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate wiring and
  RexxScript/broad-test switching are queued; the deprecated helper preserves
  current compiler artifacts. Row 20 is now the sole active item.

### 20. `lower` — done

- Source/public surface: `lib/rxfnsb/rexx/lower.crexx`, one exported
  `lower(string=.string) -> .string` procedure.
- Dependencies and known consumers: no library dependency; `.Rexx.lower`,
  `trace`, `parse`, and `symbol` call it. A compatibility Level C helper also
  exists, but the component ledger explicitly does not classify LOWER as a
  required Level C BIF.
- Current Level B signature/error/algorithm findings: the type and one direct
  `strlower` algorithm are appropriate, but the source unnecessarily exposes
  its argument, uses a stale UPPER comment, and leaves the result untyped. The
  stale aggregate is 31 RXAS lines. Valid `.string` input has no error branch.
- Level C contract and current implementation/lowering status: not applicable
  for this programme; `RexxClassicBifs` retains LOWER only as a dispatcher
  compatibility/practicality helper, so no second standalone implementation is
  required by the repository ledger.
- Existing Level B tests: two assertions cover an ASCII sentence and one
  accented capital, with no empty/already-lower/mixed-Unicode/non-mutation
  cases or pass marker.
- Existing Level C tests: broad compatibility coverage only; out of scope for
  this non-Level-C row.
- Existing Level B docs: the book advertises unsupported optional start/length
  arguments and has no selector-local Level B page.
- Existing Level C docs/spec clauses: the compiler catalog explicitly labels
  LOWER a cREXX/RexxScript convenience outside the required standard set.
- Approved implementation notes or decisions: remove the unnecessary argument
  exposure, correct the stale comment, keep the one-instruction algorithm, and
  replace the narrow harness with empty/mixed/Unicode/non-mutation coverage and
  a pass marker. Correct the book to the actual whole-string Level B surface
  and add selector-local docs. Do not create a Level C module outside the
  repository ledger.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: lower`. Coverage includes ASCII, mixed text,
  accented Unicode mappings, already-lower and empty strings, and argument
  non-mutation. No aggregate target was rebuilt.
- Performance/RXAS review evidence: Level B emits 37 lines in both modes versus
  31 stale lines. As with LENGTH, the increase is RexxDoc/source metadata;
  executable work remains exactly one direct `strlower`, with no call, loop, or
  intermediate helper allocation.
- Completion summary: all applicable B/T/P/D/V gates are complete and Level B
  wiring is queued. C is not applicable. Row 21 is now the sole active item.

### 21. `overlay` — parked

- Source/public surface: `lib/rxfnsb/rexx/overlay.crexx`, one exported
  `overlay(insstr=.string, string=.string, position=.int, len=0, pad="")`
  procedure returning `.string`.
- Dependencies and known consumers: currently calls Level B `substr` several
  times. `fmtmask`, `runmask`, `wordrep`, the focused harness, and
  `.Rexx.overlay` consume it. The class adapter always supplies its zero length
  default, so it cannot preserve omitted versus explicit-zero semantics and is
  a likely integration co-dependency.
- Current Level B signature/error/algorithm findings: start is incorrectly
  mandatory instead of defaulting to one; supplied-length presence is lost,
  zero and negative lengths are treated as omitted, empty pad is silently made
  blank, and multi-character pad is truncated. Empty target/new inputs are
  special-cased by mutating local argument values. Repeated `strlen`/`substr`
  helper calls and concatenation produce 1,372 stale RXAS lines.
- Level C contract and current implementation/lowering status:
  `OVERLAY(new, target [,start [,length [,pad]]])` uses
  `rANY rANY oWHOLE>0 oWHOLE>=0 oPAD`; start defaults to one and omitted length
  defaults to the new-string length. There is no standalone direct
  implementation; compiler lowering remains out of scope.
- Existing Level B tests: seventeen noisy/commented checks cover many ordinary,
  padding, truncation, beyond-target, and empty cases, but the default-start
  case is commented out and explicit zero, invalid signals, Unicode,
  non-mutation, and a pass marker are missing.
- Existing Level C tests: none.
- Existing Level B docs: the book has five examples but uses ambiguous names,
  does not distinguish omitted length from zero, and has no selector-local
  typed/error page.
- Existing Level C docs/spec clauses: the compiler catalog records the correct
  signature/checklist and notes its similarity to INSERT; no standalone Level C
  page or RexxDoc implementation exists.
- Approved implementation notes or decisions: preserve optional start/length
  presence, type the numeric arguments, signal non-positive start, negative
  supplied length, or invalid pad, and use the reviewed INSERT-style direct
  construction with the tail beginning after the overlay width. Add standalone
  RexxValue BIF/tests/docs; defer aggregate wiring and park the class adapter.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: overlay`; isolated direct Level C overlays both
  report `PASS: Level C OVERLAY BIF`. Coverage includes the documented
  examples, default and omitted optional slots, explicit zero within/beyond the
  target, empty input, truncation, Unicode position/pad, non-mutation,
  substantial padding, Level B signals, and Level C
  `40.3`/`40.4`/`40.5`/`40.12`/`40.13`/`40.14`/`40.23` errors. Neither path
  rebuilt an aggregate target.
- Performance/RXAS review evidence: Level B emits 383 lines in both modes,
  down from 1,372 stale-aggregate lines. It caches three character lengths,
  performs at most two direct substrings, uses bounded appends and O(1)
  `padstr`, and makes no formatting-helper call. The direct BIF emits 491
  noopt/494 opt lines with the same algorithm; its harness calls the standalone
  entry directly and contains no controller call.
- Completion summary: all selector B/C/T/P/D/V gates are complete and wiring is
  queued. The row is parked only for `.Rexx.overlay` optional-length adaptation
  and its classlib test at integration. Row 22 is now active.

### 22. `pos` — done

- Source/public surface: `lib/rxfnsb/rexx/pos.crexx`, one exported
  `pos(needle=.string, haystack=.string, start=1) -> .int` procedure.
- Dependencies and known consumers: heavily used throughout Level B and
  classlib; `.Rexx.pos` delegates with a compatible start default. The body
  currently calls Level B `length` twice before one VM `strpos`.
- Current Level B signature/error/algorithm findings: string/result types and
  the direct search are appropriate, but start only infers integer from its
  default and non-positive values are not rejected. Two helper calls guard
  empty strings even though direct comparisons can do so without call overhead;
  the stale aggregate is 100 RXAS lines.
- Level C contract and current implementation/lowering status:
  `POS(needle, haystack [,start])` is `rANY rANY oWHOLE>0`; null needle returns
  zero. A direct helper currently lives in the common module and calls Level B
  POS. The `M` ledger status requires a standalone RexxValue context module;
  the common helper/controller remains deprecated compatibility until final
  integration/lowering work.
- Existing Level B tests: twenty repetitive checks cover documented, absent,
  empty, boundary, start, and Unicode cases, but have no invalid-start signal,
  non-mutation, pass marker, or substantial-search case.
- Existing Level C tests: the broad harness has one dispatcher check and one
  direct common-helper call; there is no standalone direct harness.
- Existing Level B docs: the book has four examples but makes start look
  mandatory and has no selector-local typed/signal/performance page.
- Existing Level C docs/spec clauses: the compiler catalog records the correct
  checklist and null-needle result; no selector-local Level C page exists.
- Approved implementation notes or decisions: type/preserve the positive start,
  signal invalid Level B starts, replace helper-length guards with direct null
  checks, and retain one direct `strpos`. Add the standalone RexxValue context
  implementation with the same algorithm and qualify it around the deprecated
  common helper name; defer all aggregate/runtime switching.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: pos`; isolated direct Level C overlays both
  report `PASS: Level C POS BIF`. Coverage includes documented, absent, empty,
  boundary, beyond-end, repeated, Unicode/default-start and Unicode-later-start,
  non-mutation, a 1,001-character direct search, Level B signals, and Level C
  `40.3`/`40.4`/`40.5`/`40.12`/`40.14` errors. Neither path rebuilt an aggregate
  target.
- Performance/RXAS review evidence: Level B emits 81 lines in both modes,
  down from 100 stale lines. The executable search path performs direct null
  checks and exactly one `strpos`, with no helper call or substring allocation.
  The standalone BIF emits 184 noopt/190 opt lines and adds only standard
  context handling around the same one-instruction search; its harness contains
  no controller call.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate module/test
  wiring and common-helper migration are queued. Row 23 is now active.

### 23. `lastpos` — parked

- Source/public surface: `lib/rxfnsb/rexx/lastpos.crexx`, one exported
  `lastpos(needle=.string, haystack=.string, upto=0) -> .int` procedure.
- Dependencies and known consumers: `datatype`, `.Rexx.lastpos`, and the focused
  harness use it. Its body has no library call but performs three direct lengths
  and repeated forward `strpos` searches because the VM has no reverse-search
  instruction.
- Current Level B signature/error/algorithm findings: the zero sentinel loses
  optional-argument presence and makes explicit zero mean omitted instead of an
  error. Locals are mostly inferred rather than explicitly integer. The forward
  scan correctly permits overlapping candidates and rejects matches ending
  after `upto`; the stale aggregate is 155 RXAS lines.
- Level C contract and current implementation/lowering status:
  `LASTPOS(needle, haystack [,start])` uses `rANY rANY oWHOLE>0`; omitted start
  means the haystack end and null needle returns zero. No standalone direct
  implementation exists; compiler lowering is out of scope.
- Existing Level B tests: twenty-five repetitive checks cover ordinary,
  absent, explicit limit, multicharacter, boundary, empty haystack, and Unicode
  cases, but empty-needle cases are commented out and invalid-start signals,
  overlapping matches, non-mutation, substantial input, and a pass marker are
  missing.
- Existing Level C tests: none.
- Existing Level B docs: no selector-local page; the book section is being
  inventoried for start semantics and typed/error details.
- Existing Level C docs/spec clauses: the compiler catalog records the correct
  checklist and leftward-search summary; no selector-local Level C page or
  RexxDoc implementation exists.
- Approved implementation notes or decisions: replace the zero sentinel with
  optional presence, signal invalid supplied ends, type all scan state, cache
  the two lengths once, and retain the overlap-correct monotonic `strpos` scan
  because no reverse VM search exists. Add standalone RexxValue BIF/tests and
  separate docs; park the class adapter and defer aggregate wiring.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: lastpos`; isolated direct Level C overlays both
  report `PASS: Level C LASTPOS BIF`. Coverage includes ordinary, absent,
  exact/beyond limits, multicharacter-end semantics, empty input, overlapping
  matches, Unicode, non-mutation, 1,000-character overlapping input, Level B
  signals, and Level C `40.3`/`40.4`/`40.5`/`40.12`/`40.14` errors. Neither path
  rebuilt an aggregate target.
- Performance/RXAS review evidence: Level B emits 195 lines in both modes
  versus 155 stale lines. The increase is explicit presence/error/type and
  RexxDoc metadata; runtime retains two cached `strlen` operations and one
  advancing `strpos` per match, with no helper call or substring. The direct
  BIF emits 300 noopt/306 opt lines with the same scan and no controller call.
- Completion summary: all selector B/C/T/P/D/V gates are complete and wiring is
  queued. The row is parked only for `.Rexx.lastpos` optional-argument
  adaptation at integration. Row 24 is now active.

### 24. `left` — done

- Source/public surface: `lib/rxfnsb/rexx/left.crexx`, one exported
  `left(lstring=.string, leftlen=0, pad=' ') -> .string` procedure plus an
  unnecessary `_rxsysb` import.
- Dependencies and known consumers: widely used throughout Level B and
  classlib; `.Rexx.left` delegates with the required integer width. The body
  imports the signal helper, uses `strlen`, `substcut`, `strchar`, and `padstr`.
- Current Level B signature/error/algorithm findings: width is incorrectly
  optional instead of a required `.int`, negative width silently returns null,
  pad validation occurs only on the padding path and accepts null, and the pad
  codepoint is incorrectly initialized as a string. Truncation mutates the
  local source via `substcut`; padding uses expression concatenation. The stale
  aggregate is 163 RXAS lines.
- Level C contract and current implementation/lowering status:
  `LEFT(string, length [,pad])` is `rANY rWHOLE>=0 oPAD`. A direct helper in the
  common module currently uses Level B helpers; `M` requires a standalone
  RexxValue context module while retaining the common controller body until
  final migration/lowering work.
- Existing Level B tests: nine repetitive assertions cover documented,
  zero/equal/short/long, and one-character pad cases, but omit negative/empty/
  multi-pad signals, Unicode, non-mutation, substantial padding, and a pass
  marker.
- Existing Level C tests: the broad harness covers one success, negative width,
  and multi-pad error; no standalone direct harness exists.
- Existing Level B docs: the book has three examples but ambiguous names and no
  selector-local typed/signal/performance page.
- Existing Level C docs/spec clauses: the compiler catalog records the correct
  checklist and character semantics; no selector-local Level C page exists.
- Approved implementation notes or decisions: require typed width, validate it
  and the one-codepoint pad before all result paths, remove `_rxsysb`, type the
  pad codepoint correctly, and use direct substring/pad/append operations
  without mutating the local source. Add the standalone context BIF with a
  qualified direct harness and separate docs; deprecate but retain the common
  controller body.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: left`; isolated direct Level C overlays both
  report `PASS: Level C LEFT BIF`. Coverage includes all documented shapes,
  zero/equal widths, Unicode truncation/pad, non-mutation, 1,000-character
  padding, Level B signals, and Level C
  `40.3`/`40.4`/`40.5`/`40.12`/`40.13`/`40.23` errors. Neither path rebuilt an
  aggregate target.
- Performance/RXAS review evidence: Level B emits 178 lines in both modes
  versus 163 stale lines. The extra lines are strict validation/type/docs; the
  runtime path is two cached lengths plus either one direct substring or one
  O(1) `padstr` and bounded append, with no helper call or `substcut`. The direct
  BIF emits 274 noopt/280 opt lines with the same core and no controller call.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate module/test
  wiring and common-helper migration are queued. Row 25 is now active.

### 25. `right` — done

- Source/public surface: `lib/rxfnsb/rexx/right.crexx`, one exported
  `right(rstring=.string, rlen=.int, pad=' ') -> .string` procedure plus an
  unnecessary `_rxsysb` import.
- Dependencies and known consumers: widely used throughout Level B and
  classlib; `.Rexx.right` delegates compatibly. The body uses direct length/pad
  operations but calls Level B `substr` for truncation.
- Current Level B signature/error/algorithm findings: width type is correct but
  negative values reach an invalid substring path rather than a signal. Pad is
  checked only after zero/equal-width returns, null pad is accepted, and the pad
  codepoint is incorrectly typed as string. Padding uses expression
  concatenation. The stale aggregate is 375 RXAS lines because of the helper
  call path.
- Level C contract and current implementation/lowering status:
  `RIGHT(string, length [,pad])` is `rANY rWHOLE>=0 oPAD`. A common-module
  helper currently uses Level B helpers; `M` requires a standalone RexxValue
  context module while its controller body remains until final migration.
- Existing Level B tests: fourteen repetitive assertions cover ordinary,
  empty, zero/equal, padding, truncation, and long ASCII, but omit invalid
  signals, Unicode, non-mutation, substantial padding, and a pass marker.
- Existing Level C tests: one broad success check only; no standalone direct
  harness.
- Existing Level B docs: the book has three examples but ambiguous names and no
  selector-local typed/error/performance page.
- Existing Level C docs/spec clauses: the compiler catalog records the correct
  checklist and character semantics; no selector-local Level C page exists.
- Approved implementation notes or decisions: mirror the completed LEFT design
  with right-side slicing and left-side padding.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: right`; isolated direct Level C overlays both
  report `PASS: Level C RIGHT BIF`. Coverage includes documented, empty,
  zero/equal/short/long, Unicode, non-mutation, 1,000-character padding, Level B
  signals, and Level C `40.3`/`40.4`/`40.5`/`40.12`/`40.13`/`40.23` errors.
  Neither path rebuilt an aggregate target.
- Performance/RXAS review evidence: Level B emits 182 lines in both modes
  versus 375 stale aggregate lines. The runtime path caches character lengths
  and uses either one direct cursor/substring operation or one `padstr` plus
  bounded appends, with no helper call. The direct Level C BIF emits 278 noopt/
  284 opt lines with the same core and no controller call.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate module/test
  wiring and common-helper migration are queued. Row 26 is now active.

### 26. `reverse` — done

- Source/public surface: `lib/rxfnsb/rexx/reverse.crexx`, one exported
  `reverse(str=.string) -> .string` procedure.
- Dependencies and known consumers: `.Rexx.reverse` delegates compatibly;
  `checkBSN` and the focused Level B test also consume it. There is no native
  reverse-string VM instruction.
- Current Level B signature/error/algorithm findings: the signature is already
  typed and has no domain error. The implementation calls Level B `length` and
  then calls `substr` once per character while repeatedly concatenating the
  growing result. Its stale aggregate is 302 RXAS lines and the algorithm
  creates avoidable helper-call and repeated-copy overhead.
- Level C contract and current implementation/lowering status:
  `REVERSE(string)` is `rANY`. No standalone direct RexxValue BIF or common
  compatibility-controller implementation was found; ledger marker `R`
  requires a standalone context module and direct harness.
- Existing Level B tests: three checks cover an ASCII sentence, involution, and
  empty text, but omit Unicode/codepoint behavior, single-character input,
  source non-mutation, substantial input, and a pass marker.
- Existing Level C tests: no direct or broad REVERSE test was found.
- Existing Level B docs: no selector-local page or REVERSE entry in the BIF
  function-reference book was found.
- Existing Level C docs/spec clauses: the compiler catalog records `rANY`; no
  selector-local Level C page exists.
- Approved implementation notes or decisions: traverse cached Unicode
  codepoints from end to start using direct `strchar`/`appendchar` VM operations,
  avoiding per-character Level B calls and growing-string concatenation. Add a
  standalone Level C context BIF with the same core and separate documentation.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: reverse`; isolated direct Level C overlays both
  report `PASS: Level C REVERSE BIF`. Coverage includes ASCII, involution,
  empty/one-codepoint, Unicode, source non-mutation, a 1,000-codepoint input,
  and Level C `40.3`/`40.4`/`40.5` errors. The initial substantial test's
  accidental COPIES dependency was removed so both harnesses remain
  selector-local. Neither path rebuilt an aggregate target.
- Performance/RXAS review evidence: Level B emits 79 lines in both modes versus
  302 stale aggregate lines. Its single reverse pass performs one cached
  `strlen`, then one `strchar` and one `appendchar` per codepoint; emitted RXAS
  contains no Level B helper call, substring, or concatenation. The direct
  Level C BIF emits 148 noopt/157 opt lines with the same core and no name
  controller call.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate module/test
  wiring is queued. Row 27 is now active.

### 27. `space` — done

- Source/public surface: `lib/rxfnsb/rexx/space.crexx`, one exported
  `space(string=.string, spacenr=1, pad=" ") -> .string` procedure. The integer
  literal currently makes `spacenr` optional `.int` in emitted metadata, but the
  public name does not match the contract and the source argument is
  unnecessarily exposed.
- Dependencies and known consumers: `_address`, `abs`, `b2x`, RexxScript, and
  `.Rexx.space` consume it. The class adapter supplies compatible integer/pad
  defaults. The current implementation calls the Level B `words` and `word`
  helpers repeatedly.
- Current Level B signature/error/algorithm findings: negative counts are not
  signalled; empty/multi-codepoint pads are silently truncated or reach an
  invalid `strchar`. The function rescans the source from its beginning for
  each word and repeatedly concatenates both the separator and growing result,
  making multiword input unnecessarily quadratic. The stale aggregate is 201
  RXAS lines.
- Level C contract and current implementation/lowering status:
  `SPACE(string [,count [,pad]])` is `rANY oWHOLE>=0 oPAD`. The common module
  has a compatibility body that uses Level B `words`, `word`, and `copies` plus
  repeated concatenation. Marker `M` requires a standalone direct RexxValue BIF
  while the common body remains deprecated until final migration; RexxScript
  currently calls that common export directly.
- Existing Level B tests: sixteen repetitive success assertions cover the main
  documented ASCII shapes, zero count, omitted count with a supplied pad, and
  hyphen data. They omit invalid signals, empty/all-whitespace input, Unicode
  whitespace/pad, source non-mutation, substantial input, and a pass marker.
- Existing Level C tests: the broad harness covers two success shapes only; no
  standalone direct harness or error coverage exists.
- Existing Level B docs: the book has examples but shows mandatory `n`/`pad`
  names and has no selector-local typed/signal/performance page.
- Existing Level C docs/spec clauses: the repository catalog and ANSI draft
  agree on `rANY oWHOLE>=0 oPAD`; no selector-local Level C page exists.
- Approved implementation notes or decisions: validate count and a one-codepoint
  pad before every result path, then make one Unicode-whitespace scan with
  direct `fndnblnk`/`fndblnk`, cursor/substring, and append operations. Build
  the separator lazily with one `padstr`, only when a second word requires it.
  Add the standalone Level C implementation, focused direct harness, separate
  B/C docs, and deprecate the retained common body.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: space`; isolated direct Level C overlays both
  report `PASS: Level C SPACE BIF`. Coverage includes documented/default and
  optional-hole calls, zero count, empty/all-whitespace, Unicode whitespace and
  pad, pad characters retained inside words, source non-mutation, lazy
  100,000-count single-word handling, 250 words, Level B signals, and Level C
  `40.3`/`40.4`/`40.5`/`40.12`/`40.13`/`40.23` errors. Neither path rebuilt an
  aggregate target.
- Performance/RXAS review evidence: Level B emits 264 lines in both modes
  versus 201 stale aggregate lines; the additional code is strict validation
  and the complete local scanner. Runtime changes from `WORDS` plus a fresh
  `WORD` prefix scan per word and repeated concatenation to one monotonic
  Unicode-whitespace scan, direct word-slice appends, and at most one lazy
  `padstr`. Emitted Level B RXAS has no helper call or concat instruction. The
  direct Level C BIF emits 356 noopt/362 opt lines with the same linear core and
  only context/RexxValue support calls, not the name controller or Level B
  helpers.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate module/test
  wiring and common-helper migration are queued. Row 28 is now active.

### 28. `strip` — parked

- Source/public surface: `lib/rxfnsb/rexx/strip.crexx`, one exported
  `strip(instr=.string, option="B", schar="UTF8WSP") -> .string` procedure. The
  sentinel is a nonstandard public spelling used to distinguish the default
  Unicode-whitespace set from a supplied one-character pad.
- Dependencies and known consumers: heavily used across Level B and
  RexxScript; `.Rexx.strip` always forwards its matching `UTF8WSP` sentinel.
  The function calls Level B `substr` on several custom-character paths.
- Current Level B signature/error/algorithm findings: all public values emit as
  strings, but invalid/empty options are accepted or ignored and supplied
  empty/multi-codepoint characters are truncated or reach an invalid
  `strchar`, rather than signalling. Numeric-looking custom characters are
  converted through `itos`. The body mutates its local source/cursor, mixes two
  algorithms, calls `substr`, and emits 834 stale RXAS lines.
- Level C contract and current implementation/lowering status:
  `STRIP(string [,option [,char]])` is `rANY oLTB oPAD`; omitted char means the
  configuration's complete blank set while a supplied char is exactly one
  codepoint. The common compatibility body converts omission to an explicit
  ASCII blank and delegates to Level B `strip`, so it loses that distinction.
  Marker `M` requires a standalone RexxValue BIF; RexxScript currently calls the
  common export.
- Existing Level B tests: twelve success assertions cover basic L/T/B, custom
  ASCII characters, and empty input. They use numeric `0` where Level B should
  pass a string and omit option/char errors, default Unicode blanks versus an
  explicit blank, Unicode custom characters, all-trimmed input, non-mutation,
  substantial input, and a pass marker.
- Existing Level C tests: the broad harness has one custom-character success;
  no standalone direct harness or error coverage exists.
- Existing Level B docs: the book has basic examples but mandatory-looking
  names, numeric custom-character examples, no distinction between omitted
  configuration blanks and explicit blank, and no selector-local page.
- Existing Level C docs/spec clauses: the repository catalog and ANSI draft
  agree on `rANY oLTB oPAD` and the omitted configuration blank set; no
  selector-local Level C page exists.
- Approved implementation notes or decisions: use argument presence instead of
  the public `UTF8WSP` sentinel, validate the option's first codepoint and a
  supplied one-codepoint char before all result paths, and calculate one
  start/end slice. Use direct Unicode blank scans for the omitted-char case and
  direct codepoint scans for a supplied char, with no Level B helper. Add the
  standalone Level C BIF/tests/docs and deprecate the retained common body.
  Park only the `.Rexx.strip` adapter until the final classlib checkpoint can
  preserve char omission and remove its sentinel safely.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: strip`; isolated direct Level C overlays both
  report `PASS: Level C STRIP BIF`. Coverage includes L/T/B and first-character
  options, lowercase, option/char optional holes, custom ASCII and Unicode,
  omitted Unicode whitespace versus explicit blank, empty/all-trimmed,
  non-mutation, 2,006-codepoint input, Level B signals, and Level C
  `40.3`/`40.4`/`40.5`/`40.21`/`40.28`/`40.23` errors. Neither path rebuilt an
  aggregate target.
- Performance/RXAS review evidence: Level B emits 429 lines in both modes versus
  834 stale aggregate lines. The two old mutating/helper algorithms are replaced
  by cached boundaries, monotonic blank/codepoint edge scans, and at most one
  direct substring. Emitted Level B RXAS has no helper call, `substcut`, or
  concatenation. The direct Level C BIF emits 498 noopt/507 opt lines with the
  same core and only context/RexxValue support calls, not the name controller or
  Level B helpers.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate module/test
  wiring and common-helper migration are queued. The row is parked only because
  `.Rexx.strip` still forwards the removed `UTF8WSP` sentinel; that adapter/test
  is queued for the single final classlib checkpoint. Row 29 is now active.

### 29. `substr` — parked

- Source/public surface: `lib/rxfnsb/rexx/substr.crexx`, one exported
  `substr(string1=.string, start=.int, len=-256, pad=" ") -> .string`
  procedure. The negative sentinel is used to distinguish omitted length.
- Dependencies and known consumers: this is a foundational helper with many
  Level B, classlib, and RexxScript consumers. `.Rexx.substr` always forwards
  the same `-256` sentinel and pad instead of preserving optional presence. The
  current module imports `_rxsysb` only to raise errors.
- Current Level B signature/error/algorithm findings: start is correctly typed,
  and inferred metadata makes optional length an `.int`, but the public sentinel
  collides with the required non-negative domain. Invalid start uses a helper
  raise; negative supplied length is incorrectly returned as blank; empty pad is
  silently changed to blank; multi-pad uses helper raise. Pad validation occurs
  only on padding paths. Fast paths mutate the local source with `substcut`; the
  general path otherwise uses direct cursor/substring/pad operations. The stale
  aggregate is 286 RXAS lines.
- Level C contract and current implementation/lowering status:
  `SUBSTR(string, start [,length [,pad]])` is
  `rANY rWHOLE>0 oWHOLE>=0 oPAD`. The common compatibility body uses Level B
  `length`, `substr`, and `copies` plus concatenation. Marker `L` requires a
  standalone direct RexxValue BIF; RexxScript currently calls the common export.
- Existing Level B tests: eighteen repetitive successes cover most ASCII slice/
  padding shapes and some Unicode, but one explicitly expects the nonstandard
  empty-pad fallback. They omit invalid start/length signals, multi-pad, zero
  length, optional length with supplied pad, source non-mutation, substantial
  input, and a pass marker.
- Existing Level C tests: the broad harness covers two successes and some
  shared context mechanics; no standalone direct harness or complete error
  coverage exists.
- Existing Level B docs: the book documents the standard shapes but has a
  mandatory-looking heading, no typed/signal/non-mutation detail, and no
  selector-local page.
- Existing Level C docs/spec clauses: the repository catalog and ANSI draft
  agree on `rANY rWHOLE>0 oWHOLE>=0 oPAD`; no selector-local Level C page
  exists.
- Approved implementation notes or decisions: replace the sentinel with
  argument presence, validate start, supplied length, and one-codepoint pad
  before all result paths, and use cached lengths plus at most one direct
  substring and one direct pad append without mutating the source or calling a
  helper. Add the standalone Level C BIF/tests/docs and deprecate the retained
  common body. Park only `.Rexx.substr` until the final classlib checkpoint can
  preserve length/pad presence and remove its sentinel.
- Focused validation commands/results: isolated Level B optimized/unoptimized
  overlays both report `PASS: substr`; isolated direct Level C overlays both
  report `PASS: Level C SUBSTR BIF`. Coverage includes omitted/supplied/zero
  length, optional length with supplied pad, equal and beyond-end starts,
  default/custom/Unicode padding, Unicode slicing, source non-mutation, a
  1,000-codepoint slice/pad case, Level B signals, and Level C
  `40.3`/`40.4`/`40.5`/`40.12`/`40.14`/`40.13`/`40.23` errors. Neither path
  rebuilt an aggregate target.
- Performance/RXAS review evidence: Level B emits 294 lines in both modes versus
  286 stale aggregate lines; the small increase is presence-aware strict
  validation. The runtime path caches length and performs at most one direct
  substring plus one direct `padstr` append. Emitted RXAS has no helper call,
  `substcut`, or concatenation. The direct Level C BIF emits 416 noopt/419 opt
  lines with the same core and only context/RexxValue support calls, not the
  name controller or Level B helpers.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate module/test
  wiring and common-helper migration are queued. The row is parked only because
  `.Rexx.substr` still forwards the removed `-256` sentinel; that adapter/test is
  queued for the single final classlib checkpoint. Row 30 is now active.

### 30. `substro` — done

- Source/public surface: `lib/rxfnsb/rexx/substro.crexx`, one exported
  `substro(string1=.string, start=.int, len=length(string1)+1-start,
  pad=" ") -> .string` procedure. It is a cREXX-specific alternate SUBSTR
  implementation rather than a standard BIF name.
- Dependencies and known consumers: it imports `_rxsysb` only for helper-raised
  errors. No source consumer, focused test, class adapter, or Level C use was
  found; it remains explicitly listed in the bootstrap library target/catalog.
- Current Level B signature/error/algorithm findings: start and inferred length
  are integers, but omission is hidden in a default expression, length zero is
  incorrectly rejected, and failures call `raise`. Copying and padding run one
  interpreted `concchar`/`appendchar` loop per codepoint instead of using the VM
  slice/pad primitives. The stale aggregate is 345 RXAS lines.
- Level C contract and current implementation/lowering status: not applicable;
  the component ledger has no Level C BIF mapping.
- Existing Level B tests: none; `tsubstr` exercises only the separate `substr`
  export.
- Existing Level C tests: not applicable.
- Existing Level B docs: no function-reference entry or selector-local page.
- Existing Level C docs/spec clauses: not applicable.
- Approved implementation notes or decisions: preserve the bootstrap export as
  a cREXX-specific function with the same typed, presence-aware, signal-based
  contract as corrected Level B `substr`. Keep a standalone direct VM core so
  the selector does not add a helper call; add focused B tests and documentation.
- Focused validation commands/results: isolated optimized/unoptimized Level B
  overlays both report `PASS: substro`. Coverage includes omitted/supplied/zero
  length, optional length with supplied pad, beyond-end, default/custom/Unicode
  padding, Unicode slicing, source non-mutation, a 1,000-codepoint slice/pad
  case, and signal errors for invalid start/length/pad. No aggregate target was
  rebuilt; Level C is not applicable.
- Performance/RXAS review evidence: Level B emits 294 lines in both modes versus
  345 stale aggregate lines. Per-codepoint `concchar`/`appendchar` loops and the
  helper raise import are replaced by cached lengths, at most one direct
  substring, and one direct `padstr` append. Emitted RXAS has no helper call,
  `substcut`, per-codepoint copy/pad loop, or concatenation.
- Completion summary: all applicable B/T/P/D/V gates are complete. Focused test
  registration is queued. Row 31 is now active.

### 31. `translate` — parked

- Source/public surface: `lib/rxfnsb/rexx/translate.crexx`, one exported
  `translate(source, outputTable, inputTable, pad) -> .string` procedure whose
  three optional arguments are currently represented by string defaults and
  magic `"?????"` omission sentinels.
- Dependencies and known consumers: the implementation calls the later Level B
  selectors `upper`, `copies`, and `left`; `translate` is widely used by Level
  B modules, RexxScript, classlib, compiler exits, and tests. `.Rexx.translate`
  also always forwards the legacy omission sentinels. Correct omitted-input-
  table semantics depend on the configuration character range.
- Current Level B signature/error/algorithm findings: optional presence is
  lost, pad is not typed or validated, and no failure is signalled. Both-table
  omission is incorrectly made dependent on the pad value. When the input table
  alone is omitted, the code substitutes one literal blank rather than the
  configuration range. It may allocate padded copies of both tables, invokes
  three unreviewed Level B helpers, and then performs one `transchar` plus one
  append for every source character.
- Level C contract and current implementation/lowering status: `TRANSLATE` is
  recognized but has no standalone or common-controller implementation. The
  repository checklist is `rANY oANY oANY oPAD` and explicitly requires
  configuration uppercase/range services; direct harness coverage is absent.
- Existing Level B tests: a large repetitive harness covers uppercase, explicit
  table mapping, duplicate input characters, Unicode uppercase, a 100,001-entry
  table, and optional holes. Several assertions codify the nonstandard blank-
  only fallback, the both-table-omitted custom-pad case contradicts the
  repository specification, invalid pad coverage is absent, and there is no
  pass marker.
- Existing Level C tests: none.
- Existing Level B docs: the language-reference section describes table
  padding/truncation but presents a mandatory-looking three-argument signature;
  no selector-local Level B page exists.
- Existing Level C docs/spec clauses: the repository Level C reference and ANSI
  draft agree that no supplied tables means configuration uppercase, omitted
  input table means `Config_Xrange`, omitted output table means the null table,
  output is padded to input length, and pad defaults to blank. No selector-local
  Level C page exists.
- Approved implementation notes or decisions: do not make this foundation
  selector depend on the current deprecated `xrange`, which prints a warning and
  delegates to the unreviewed `sequence`, and do not retain the nonstandard
  literal-blank approximation. Resume after row 54 establishes the repository's
  configuration-range/XRANGE service. Then replace sentinels with argument
  presence, add the standalone RexxValue implementation, correct the class
  adapter, simplify both test surfaces, and document the two contracts.
- Focused validation commands/results: not run because no independent correct
  implementation exists until the configuration-range contract is available.
- Performance/RXAS review evidence: the inventory identifies avoidable helper
  calls and table extension, but the second review is deferred with the
  implementation.
- Completion summary: parked with no implementation gate claimed. Row 54's
  configuration-range/XRANGE decision is the exact dependency; row 32 is now
  active.

### 32. `upper` — done

- Source/public surface: `lib/rxfnsb/rexx/upper.crexx`, one exported
  `upper(string=.string) -> .string` procedure.
- Dependencies and known consumers: no imported helper; one `strupper` VM
  instruction supplies the result. This foundation helper has many Level B,
  classlib, compiler-exit, and runtime-support consumers. `.Rexx.upper` forwards
  exactly the receiver and needs no optional-argument repair.
- Current Level B signature/error/algorithm findings: the public argument and
  result are already correctly typed and the algorithm is the desired single
  VM instruction. The exposed argument is not mutated and avoids the defensive
  link/copy sequence emitted for an ordinary typed string argument. Valid
  `.string` input has no runtime error branch.
- Level C contract and current implementation/lowering status: no applicable
  required Level C BIF contract in the programme ledger. A same-named common
  runtime helper exists solely for cREXX/RexxScript compatibility and remains
  outside this standalone-split programme, as already established for `lower`.
- Existing Level B tests: `ts_upper` has only one ASCII sentence and one
  accented character and no pass marker. `ts_upper_lower` duplicates the ASCII
  assertion while jointly smoke-testing the two selectors.
- Existing Level C tests: the broad compatibility-controller harness has one
  UPPER success; no standalone Level C harness applies.
- Existing Level B docs: the book incorrectly documents unsupported start and
  length extension arguments and examples. There is no selector-local page or
  RexxDoc block.
- Existing Level C docs/spec clauses: compiler documentation explicitly says
  UPPER is a practicality helper rather than a strict Classic catalog BIF; no
  new Level C documentation applies.
- Approved implementation notes or decisions: retain the zero-copy exposed
  input and one-instruction core, add source RexxDoc and a selector-local Level
  B page, correct the book to the actual one-argument Level B contract, and make
  `ts_upper` the complete focused selector harness without disturbing the
  combined smoke test.
- Focused validation commands/results: isolated optimized and unoptimized Level
  B overlays both report `PASS: upper`. Coverage includes ASCII and Unicode
  case mapping, mixed/nonletter text, already-uppercase and empty input, source
  non-mutation, and every documented example. No aggregate target was rebuilt.
- Performance/RXAS review evidence: the stale aggregate and existing isolated
  optimized/unoptimized module each contain 31 RXAS lines and one `strupper`.
  A trial ordinary typed argument emitted 37 lines because it added a defensive
  link/copy sequence; retaining the non-mutated exposed input preserves the
  31-line zero-copy path. There are no helper calls or additional scans.
- Completion summary: all applicable B/T/P/D/V gates are complete. The enhanced
  existing test is queued for the final aggregate sweep; Level C is not
  applicable. Row 33 is now active.

### 33. `verify` — done

- Source/public surface: `lib/rxfnsb/rexx/verify.crexx`, one exported
  `verify(string, reference, option='N', start=1) -> .int` procedure. The two
  optional arguments currently infer their types from unspaced literal defaults.
- Dependencies and known consumers: no imported helper. The current core uses
  only VM instructions plus interpreted loops. `.Rexx.verify` forwards its
  receiver/reference and matching defaults without an omission sentinel;
  Level B tests and ordinary library consumers call the public selector.
- Current Level B signature/error/algorithm findings: string/reference and
  result types are correct, but invalid options leave `imatch` uninitialized and
  non-positive starts are silently clamped to one instead of signalling. Each
  source character runs a nested interpreted reference-table scan, converts
  both integer codepoints to strings, and compares those temporary values. The
  stale aggregate is 287 RXAS lines.
- Level C contract and current implementation/lowering status: `VERIFY` is a
  recognized `rANY rANY oMN oWHOLE>0` BIF implemented only in the common
  controller. That body repeatedly calls Level B `length`, `substr`, and `pos`;
  no standalone direct module/harness exists.
- Existing Level B tests: thirteen repetitive assertions cover the book
  examples, empty strings, modes, and starts, but omit invalid option/start
  signals, Unicode table membership, long option names, non-mutation,
  substantial input, and a pass marker.
- Existing Level C tests: the broad controller harness has two success cases
  only; it does not directly call a standalone function or cover errors.
- Existing Level B docs: the book describes the semantics accurately but uses a
  mandatory-looking signature and has no typed/error/performance detail or
  selector-local page.
- Existing Level C docs/spec clauses: the repository catalog defines
  `rANY rANY oMN oWHOLE>0`, first-character case-insensitive options, positive
  start, and the standard empty/beyond-end results. No selector-local Level C
  page exists.
- Approved implementation notes or decisions: type the Level B start as an
  integer, validate a nonempty M/N option and positive start with
  `INVALID_ARGUMENTS` signals, cache lengths, and replace the interpreted inner
  loop/string conversions with `poschar`. Treat only positions below the cached
  reference character length as matches, which also guards the VM primitive's
  end-position behavior for Unicode/NUL input. Add the standalone RexxValue
  implementation/tests/docs and mark the common body deprecated; no compiler
  or aggregate wiring change occurs during the row.
- Focused validation commands/results: isolated Level B and standalone direct
  Level C optimized/unoptimized overlays all pass (`PASS: verify` and
  `PASS: Level C VERIFY BIF`). Coverage includes every documented example,
  M/N and long/lowercase options, optional holes, empty/beyond-end cases,
  Unicode and NUL membership, non-mutation, a 1,001-character scan, Level B
  option/start signals, and Level C `40.3`/`40.4`/`40.5`/`40.21`/`40.28`/
  `40.12`/`40.14` errors. No aggregate target was rebuilt.
- Performance/RXAS review evidence: stale Level B aggregate is 287 lines; the
  rewritten optimized/unoptimized module is 253 lines. The source and reference
  use non-mutated zero-copy bindings. The hot loop contains one `strchar` and
  one `poschar`, with no nested interpreted scan, helper call, substring,
  append, or `itos`; cached reference length bounds the VM result for Unicode/
  NUL correctness. The standalone Level C BIF is 360 noopt/366 opt lines and
  has the same direct core plus only context/RexxValue support calls.
- Completion summary: all B/C/T/P/D/V gates are complete. Aggregate test/module
  wiring and common-helper migration are queued. Row 34 is now active.

### 34. `_ftrunc` — done

- Source/public surface: `lib/rxfnsb/rexx/_ftrunc.crexx`, one internal exported
  `_rxsysb._ftrunc(number=.float) -> .string` procedure that returns the text
  following the decimal point in the VM-formatted float.
- Dependencies and known consumers: no imported helpers; `format.crexx` is the
  only source consumer. The adjacent `_itrunc` is a separate next-row selector.
- Current Level B signature/error/algorithm findings: the float input and string
  result are correctly typed and invalid coercion is handled by the VM's typed
  call signal. The implementation converts the float in place, scans every
  formatted character for a point, converts each codepoint to text, then scans
  the suffix again to append it one character at a time. It uses `dp=0` as both
  not-found state and position and emits 155 stale aggregate RXAS lines.
- Level C contract and current implementation/lowering status: not applicable;
  this private helper is not in the Level C recognition ledger.
- Existing Level B tests: no focused helper test. `ts_format` indirectly covers
  only a few decimal values and is mostly diagnostic output/TODO commentary.
- Existing Level C tests: not applicable.
- Existing Level B docs: none; the public BIF book should not expose this private
  implementation helper.
- Existing Level C docs/spec clauses: not applicable.
- Approved implementation notes or decisions: preserve the typed private
  contract and VM numeric formatting, but replace both interpreted loops and all
  `itos`/append work with one direct `strpos`, cached formatted length, and at
  most one `substring`. Add focused Level B tests/docs without changing
  `format` or `_itrunc` in this row.
- Focused validation commands/results: isolated optimized/unoptimized Level B
  overlays both report `PASS: _ftrunc`. Coverage includes positive/negative
  fractions, values below one, whole/zero values, absence of synthetic trailing
  zeros, caller non-mutation, documented examples, and dynamic invalid-float
  coercion through `CONVERSION_ERROR`. No aggregate target was rebuilt.
- Performance/RXAS review evidence: stale aggregate is 155 lines with two
  interpreted per-character loops; the rewritten optimized/unoptimized module
  is 95 lines. After `ftos`, it executes one `strpos`, one cached `strlen`, and
  at most one `setstrpos`/`substring`. Emitted code has no helper call,
  `strchar`, `itos`, per-character loop, append, or concatenation.
- Completion summary: all applicable B/T/P/D/V gates are complete. New focused
  test registration is queued; Level C is not applicable. Row 35 is now active.

### 35. `_itrunc` — done

- Source/public surface: `lib/rxfnsb/rexx/_itrunc.crexx`, one internal exported
  `_rxsysb._itrunc(number=.float) -> .string` procedure that returns the
  formatted text before the decimal point.
- Dependencies and known consumers: no imported helpers; `format.crexx` is the
  only source consumer. `_ftrunc` is now independently complete.
- Current Level B signature/error/algorithm findings: the float input/string
  result are correctly typed and dynamic invalid coercion uses the VM signal.
  The implementation converts the float in place, then scans and appends every
  formatted codepoint until the decimal point, converting each integer
  codepoint to a temporary string. It emits 110 stale aggregate RXAS lines.
- Level C contract and current implementation/lowering status: not applicable;
  this private helper is not in the Level C recognition ledger.
- Existing Level B tests: no focused helper test. `ts_format` only indirectly
  exercises a few values and is primarily diagnostic/TODO output.
- Existing Level C tests: not applicable.
- Existing Level B docs: none; the public BIF book should not expose this private
  implementation helper.
- Existing Level C docs/spec clauses: not applicable.
- Approved implementation notes or decisions: preserve VM numeric formatting
  and sign behavior, but replace the interpreted scan/copy/`itos` path with one
  direct `strpos` and at most one prefix `substring`; copy the existing string
  payload when there is no decimal point so it is not formatted twice. Add
  focused Level B tests/docs without editing `format` in this row.
- Focused validation commands/results: isolated optimized/unoptimized Level B
  overlays both report `PASS: _itrunc`. Coverage includes positive/negative
  fractions, values below one, signed/unsigned whole values, zero, caller
  non-mutation, documented examples, and dynamic invalid-float coercion through
  `CONVERSION_ERROR`. No aggregate target was rebuilt.
- Performance/RXAS review evidence: stale aggregate is 110 lines with an
  interpreted per-character append loop; the rewritten optimized/unoptimized
  module is 86 lines. It executes one `ftos` and one `strpos`, then either one
  `scopy` of the already formatted payload or one `substring`. The first trial
  used a typed return on the whole-number path and emitted a redundant second
  `ftos`; the explicit payload copy removes it. There are no helper calls,
  per-character operations, append loops, or concatenations.
- Completion summary: all applicable B/T/P/D/V gates are complete. New focused
  test registration is queued; Level C is not applicable. Row 36 is now active.

### 36. `abs` — VM gate complete; class adapter parked

- Source/public surface: `lib/rxfnsb/rexx/abs.crexx`, one exported
  `abs(number=.string) -> .string` procedure that currently implements text
  cleanup rather than numeric absolute value.
- Dependencies and known consumers: calls the reviewed Level B `space`, `strip`,
  `left`, and `substr` helpers. Known consumers include `datatype`, the `.Rexx`
  method adapter, tests, and ordinary BIF calls. The class adapter passes its
  stored string directly and its tests include legacy blank-separated signs.
- Current Level B signature/error/algorithm findings: the argument and result
  are strings instead of numeric values. The implementation removes all blanks
  anywhere in the input, strips either sign without validating the remaining
  text, performs two repeated `left` calls, and can return nonnumeric text as a
  successful result. It emits 774 stale aggregate RXAS lines through four
  helper calls.
- Level C contract and current implementation/lowering status: recognized
  `ABS(number)` with `rNUM`; the common controller already uses RexxValue
  decimal conversion but there is no standalone module/harness. Classic numeric
  text includes the documented blank-separated leading-sign examples, which
  the current shared NUM checker rejects before conversion.
- Existing Level B tests: nine repetitive assertions largely exercise the old
  whitespace/sign string rewriting. They omit typed decimal precision,
  conversion signals, non-mutation, a pass marker, and clear Level B versus
  Classic text expectations.
- Existing Level C tests: the broad controller harness has no direct focused ABS
  coverage or standard error cases.
- Existing Level B docs: the book describes the Classic normalization rule and
  examples, but does not distinguish the native typed Level B contract. No
  selector-local Level B page exists.
- Existing Level C docs/spec clauses: repository and ANSI material agree on
  `rNUM`, caller numeric normalization, absolute decimal value, and standard
  argument errors. No selector-local Level C page exists.
- Approved implementation notes or decisions: make Level B a direct
  `.decimal -> .decimal` function with typed conversion signals and no string
  helper calls. Extract the standalone RexxValue decimal BIF and mark the common
  body deprecated. Normalize only whitespace between a leading sign and its
  numeric text in shared Classic NUM checking (not arbitrary embedded digit
  whitespace), so documented Level C input is accepted. Park `.Rexx.abs` until
  the final classlib checkpoint supplies Classic string normalization rather
  than weakening the native Level B signature.
- Resolved VM dependency: dynamic invalid `.string -> .decimal` argument
  binding now maps decNumber conversion syntax to catchable
  `CONVERSION_ERROR`. The optimized and unoptimized Level B harnesses include
  this runtime case.
- Focused validation commands/results: isolated valid Level B optimized/
  unoptimized overlays both report `PASS: abs`; because the new signature
  intentionally conflicts with the stale aggregate metadata, the focused
  harness compile temporarily hides and immediately restores only
  `library.rxbin` rather than rebuilding it. Coverage includes positive,
  negative, zero, standard numeric-string promotion, exponent normalization,
  and non-mutation. Standalone direct Level C overlays both report
  `PASS: Level C ABS BIF`, covering ordinary/signed/exponent/blank-separated
  forms, non-mutation, and `40.3`/`40.4`/`40.5`/`40.11` errors including
  rejection of embedded digit blanks. The dynamic invalid Level B decimal case
  now catches `CONVERSION_ERROR` in both modes.
- Performance/RXAS review evidence: stale Level B aggregate is 774 lines with
  four helper calls and repeated slicing; the native Level B core is 39 noopt/
  33 opt lines with one decimal comparison and at most one subtraction, no
  helper/string call, scan, or allocation. The standalone Level C BIF is 127
  noopt/128 opt lines and adds only CheckArgs/context/RexxValue conversion plus
  the same decimal core.
- Completion summary: B/C/T/P/D/V are complete for the standalone selector.
  Only the distinct Classic-normalizing `.Rexx.abs` adapter remains parked.

### 37. `format` — VM gate complete; class adapter test parked

- Source/public surface: `lib/rxfnsb/rexx/format.crexx`, one exported
  `format(number=.string, before=0, after=0, expp=0, expt=-1) -> .string`
  procedure. Optional presence is collapsed into numeric sentinels.
- Dependencies and known consumers: imports `_rxsysb` and calls `_itrunc`,
  `_ftrunc`, `strip`, `left`, `substr`, and `right`; consumers include mask/
  reporting helpers and `.Rexx.format`. The class adapter also forwards five
  concrete defaults and therefore loses Classic omission/null distinctions.
- Current Level B signature/error/algorithm findings: the number and four width
  controls are strings/inferred integers rather than a native decimal plus
  presence-aware integer options. The implementation coerces through float,
  always inserts a decimal point on the non-exponent path, does not implement
  required rounding, conflates omitted `after=0` with explicit zero, uses a
  fixed `%1.14E` exponent representation, and returns English error strings
  instead of signals. It emits 2,388 stale aggregate RXAS lines.
- Level C contract and current implementation/lowering status: recognized but
  unimplemented. Repository contract is
  `rNUM oWHOLE>=0 oWHOLE>=0 oWHOLE>=0 oWHOLE>=0`, driven by caller numeric
  form/precision, with `40.38` when integer or exponent width cannot fit. There
  is no standalone/common implementation or direct harness.
- Existing Level B tests: only the first two assertions can fail; the remainder
  prints diagnostics and contradictory expected-value commentary. Most of the
  much broader class-method matrix is commented out. Rounding, omissions,
  widths, exponent thresholds, signals, numeric context, and pass-marker
  coverage are absent.
- Existing Level C tests: none.
- Existing Level B docs: the book contains a broad Classic/extension narrative
  including a sixth `exform` argument that is not in the repository's five-
  argument contract. It does not separate typed Level B behavior or link a
  selector-local page.
- Existing Level C docs/spec clauses: the repository catalog explicitly says
  this should become a shared numeric formatter rather than copied into callers;
  no selector-local page exists.
- Approved implementation notes or decisions: row 40 established direct,
  zero-overhead inheritance of caller DIGITS/FORM, and row 42 established exact
  decimal coefficient/exponent decomposition plus a long-mantissa textual path.
  Resume now: build presence-aware Level B and standalone RexxValue entry points
  over the appropriate decimal/textual cores, replace returned error strings
  with signals/context errors, simplify the tests into normative assertions,
  correct or park the class adapter as required, and reconcile the book to the
  repository's five-argument contract.
- Implemented Level B surface: `format(.decimal, optional .int, optional .int,
  optional .int, optional .int) -> .string`, with explicit presence for all
  controls, inherited DIGITS/FORM, exact decimal digit rounding, lower/upper
  exponent parsing, and `INVALID_ARGUMENTS` signals for negative or non-fitting
  controls. Float conversion, `_itrunc`/`_ftrunc`, returned error strings, and
  general string-BIF calls were removed from the optimized path.
- Implemented Level C surface: standalone
  `rexxclassicbifformat.rexxclassicbif_format(context_ref)` over RexxValue and
  `rNUM oWHOLE>=0 oWHOLE>=0 oWHOLE>=0 oWHOLE>=0`. It inherits caller numeric
  settings, preserves omissions, reports standard count/type errors, and uses
  `40.38` with argument 2 or 4 for fit failures. No common-controller body or
  compiler/lowering change was introduced.
- Test/doc result: `ts_format` is now an assertion-only Level B harness covering
  rounding, widths, omission, exponent suppression/defaulting, zero-exponent
  fields, scientific/engineering form, growth on rounding, non-mutation, and
  signals. The direct Level C harness covers the same contract plus
  `40.3`/`40.4`/`40.5`/`40.11`/`40.12`/`40.13`/`40.38`. Separate stable B/C
  pages were added, and the book was corrected from the unsupported six-
  argument extension to the repository's five-argument contract.
- Focused validation commands/results: isolated optimized and unoptimized Level
  B overlays both report `PASS: format`; direct Level C overlays both report
  `PASS: Level C FORMAT BIF`. The focused aggregate metadata was hidden and
  restored only while compiling the intentionally changed Level B signature;
  `library.rxbin` remains present. No aggregate build or CTest sweep ran.
- Performance/RXAS review evidence: the stale implementation emitted 2,388
  lines, converted through float, and repeatedly called general helpers. The
  final Level B implementation emits 1,800 noopt / 1,768 opt lines, has no
  `rxfnsb` helper calls on the optimized path, and keeps arithmetic in exact
  decimal/text operations. Direct Level C is 1,967 noopt / 1,935 opt lines.
- Completion summary: B/C/T/P/D/V are complete for the standalone selector,
  including catchable dynamic-decimal conversion in both modes. The corrected
  omission-preserving `.Rexx.format` source remains parked only for its focused
  class-adapter test.

### 43. `b2x` — done

- Source/public surface: `lib/rxfnsb/rexx/b2x.crexx` exports
  `b2x(bin=.string) -> .string`; `.Rexx.b2x` forwards its receiver text without
  changing the contract.
- Dependencies and current algorithm: imports `_rxsysb`, removes blanks through
  `space`, validates through `verify`, pads through `copies`, converts the full
  value through integer-based `reradix`, then calls `translate` and `right`.
  The integer intermediate is a size/precision dependency for an operation that
  is naturally a linear nibble conversion.
- Level B error/contract findings: invalid binary text returns the same empty
  string as valid empty input instead of raising a signal. It also accepts
  blanks in arbitrary locations after `space`, while the repository/ANSI `BIN`
  contract rejects leading/trailing blanks and requires a multiple of four
  binary digits to the right of each interior blank.
- Level C status: `B2X` is listed in `raw-levelc-bifs.md` with the exact
  `CheckArgs 'rBIN'` contract. There is no standalone implementation, common
  controller body, direct test, or BIN rule in the current shared CheckArgs
  helper.
- Test findings: B2X has four assertions embedded at the end of the X2B harness;
  they cover empty input, one conversion, and invalid input returning empty, and
  the `"10 10"` expectation contradicts standard blank placement. There is no
  direct Level C harness. The `.Rexx.b2x` forwarding adapter is simple but not
  covered by a focused selector test.
- Documentation findings: the book has the five main conversion examples but
  says at least one digit even though repository tests and `DATATYPE(...,'B')`
  treat the empty binary string as valid. No separate stable Level B or Level C
  page exists, and invalid/blank-placement behavior is undocumented.
- Performance baseline: isolated original overlays emit 189 noopt / 233 opt
  RXAS lines and make five general helper calls (`space`, `verify`, `reradix`,
  `translate`, and `right`).
- Planned focused change: replace the integer/reradix path with a typed linear
  nibble converter and `INVALID_ARGUMENTS` signals, add the shared direct BIN
  CheckArgs rule plus standalone RexxValue B2X, split B2X out of the X2B test,
  prove the class adapter, and add separate B/C documentation. Only the B2X
  overlays and changed shared CheckArgs support will be compiled.
- Implemented result: Level B now validates the standard `BIN` blank rules,
  signals `INVALID_ARGUMENTS`, and emits hexadecimal one nibble at a time with
  no whole-value numeric conversion or general helper call. The simple
  `.Rexx.b2x` adapter remains contract-compatible and is covered directly.
- Level C result: shared CheckArgs now implements `BIN` and standard `40.24`;
  standalone `rexxclassicbifb2x.rexxclassicbif_b2x(context_ref)` performs the
  direct RexxValue conversion. No common-controller or compiler change was
  added.
- Test/doc result: dedicated B/C harnesses cover partial/complete nibbles,
  retained zero nibbles, valid/consecutive group blanks, empty input, argument
  non-mutation, a 256-bit input, all invalid blank locations/characters,
  argument count/omission, `40.24`, and the class adapter. B2X assertions were
  removed from the X2B harness. Separate B/C pages and the book now document
  empty input, grouping, errors, and the unbounded linear implementation.
- Focused validation: optimized/unoptimized Level B overlays both report
  `PASS: b2x`; direct Level C overlays, compiled with only the changed shared
  CheckArgs support, both report `PASS: Level C B2X BIF`. No aggregate build or
  CTest sweep ran.
- Second performance/style review: original output was 189 noopt / 233 opt
  lines with five helper calls and a precision-dependent integer intermediate.
  The complete implementation is 330 noopt / 303 opt lines, has zero general
  helper calls, two bounded linear scans, constant per-nibble arithmetic, and
  no input-size/decimal-precision dependency. Direct Level C is 312 noopt / 310
  opt lines. The extra validation code is required contract work, not an
  algorithmic regression.
- Completion summary: every B/C/test/performance/doc/validation gate is complete;
  row 43 is done and row 44 is active.

### 44. `b2d` — parked after native-core completion

- Source/public surface: `lib/rxfnsb/rexx/b2d.crexx` exports the non-Level-C
  helper `b2d(bin=.string) -> .int`; `.Rexx.b2d` forwards only its receiver and
  therefore exposes the same one-argument unsigned conversion.
- Current implementation: delegates the whole string to the decimal/integer
  `reradix` accumulator without validating binary text or standard blank
  placement. It has no selector test and returns whatever overflow/invalid
  behavior the unrelated helper produces.
- Contract/type finding: `.string -> .int` is the current code and class
  contract and matches Level B's signed 64-bit native integer surface. The old
  class-reference prose describes an optional signed-width argument that
  neither callable accepts; this is documentation drift rather than a working
  API to preserve. B2D is absent from the repository Level C BIF ledger, so no
  RexxValue implementation or Level C page is required.
- Error/limit finding: valid unsigned results are limited to `INT64_MAX` by the
  declared return type. Invalid binary text needs `INVALID_ARGUMENTS`; a value
  with more than 63 significant bits needs `OVERFLOW_UNDERFLOW`. Arbitrarily
  many leading zeroes can still be accepted without overflow.
- Test/doc finding: there is no Level B test. The Markdown class reference has
  useful unsigned examples but also non-runnable optional signed-width examples;
  no selector-local Level B page documents grouping, signals, or the 64-bit
  boundary.
- Performance baseline: original optimized and unoptimized overlays are both 37
  RXAS lines but make one precision-dependent whole-value `reradix` call. The
  focused replacement will be a direct bounded linear scan; RXAS size alone is
  not the useful comparison for this selector.
- Planned focused change: retain the actual one-argument typed surface, add
  standard BIN validation and explicit overflow signaling, replace `reradix`
  with direct checked accumulation, add a dedicated Level B/class-adapter
  harness, and reconcile the Markdown docs. No Level C/common/compiler work and
  no aggregate rebuild are in scope for this row.
- Implemented result: B2D now validates the same standard binary grouping as
  B2X, returns zero for empty/all-zero input, accumulates only significant bits,
  signals `INVALID_ARGUMENTS` for bad text and `OVERFLOW_UNDERFLOW` above 63
  significant bits, and makes no whole-value radix-helper call. The class
  source return was corrected from `.rexx` to `.int` to match the native
  function and the existing C2D/X2D observation pattern.
- Test/doc result: new `ts_b2d` covers the documented unsigned examples, grouped
  blanks, empty input, 128 leading zeroes, exact `INT64_MAX`, non-mutation,
  invalid input, and 64-bit overflow. A stable selector page documents the
  actual one-argument Level B surface and signals. The Markdown class reference
  was stripped of the non-runnable optional signed-width extension and linked
  to the stable contract. B2D is not in the Level C ledger, so no Level C code,
  test, or page was created.
- Focused validation: optimized and unoptimized overlays both report
  `PASS: b2d`. Original output was 37 lines with a `reradix` call; the checked
  direct implementation is 309 noopt / 281 opt lines with no helper calls, two
  bounded linear scans, no decimal-precision dependency, and an explicit
  signed-64-bit boundary.
- Completion summary: T/P/D and the complete native core are independently
  proven. B/V remain open only because the current aggregate classlib metadata
  still describes `.Rexx.b2d` as returning `.rexx`; row 44 is parked for the
  single final classlib rebuild/adapter assertion, and row 45 is active.

### 38. `max` — done

- Source/public surface: `lib/rxfnsb/rexx/max.crexx`, one exported variadic
  `max(first=.float, ...=.float) -> .float` procedure.
- Dependencies and known consumers: no imported helper; it is used in regex
  index/length calculations and ordinary library calls. There is no `.Rexx.max`
  adapter despite a stale object-style book example.
- Current Level B signature/error/algorithm findings: at least one argument is
  structurally required and the single linear comparison loop is sound, but
  float input/result lose decimal precision and representation. Dynamic invalid
  decimal conversion shares row 36's VM signal blocker. The stale aggregate is
  63 RXAS lines.
- Level C contract and current implementation/lowering status: `MAX(number, ...)`
  is a recognized `rNUM...` BIF implemented only in the common controller. It
  validates every present argument as numeric, compares decimals, and preserves
  the first normalized argument text on ties. No standalone module/harness
  exists; fixed-list CheckArgs does not expose a variadic validation entry.
- Existing Level B tests: one combined MIN/MAX smoke harness has only three
  positive integer MAX cases. It omits one argument, negatives, decimals,
  precision beyond binary float, ties, non-mutation, conversion behavior, and a
  pass marker.
- Existing Level C tests: the broad controller harness has one three-argument
  success and no direct/error/tie coverage.
- Existing Level B docs: the book describes Classic numeric comparison and
  first-argument tie representation, but does not distinguish native typed
  Level B behavior and includes an unavailable object-method example. No
  selector-local page exists.
- Existing Level C docs/spec clauses: repository contract is variadic `rNUM...`
  with at least one argument, caller-context decimal comparison, and first
  selection on ties; no selector-local page exists.
- Approved implementation notes or decisions: change Level B to a direct
  `.decimal` variadic core and retain one linear scan. Add a shared exported
  variadic CheckArgs support function, standalone RexxValue MAX/tests/docs, and
  deprecate the common controller body. Level C preserves selected normalized
  input text; typed Level B returns a decimal. Park only dynamic invalid Level B
  decimal conversion on the already identified VM signal dependency.
- Focused validation commands/results: valid Level B optimized/unoptimized
  overlays both report `PASS: max`; the harness compilation temporarily hides
  and immediately restores the stale aggregate `library.rxbin` metadata rather
  than rebuilding it. Coverage includes one/many arguments, positive/negative
  values, numeric-string promotion, ties, non-mutation, and exact ordering above
  binary float precision. Standalone direct Level C overlays both report
  `PASS: Level C MAX BIF`, covering the documented representation-preserving
  ties, exponent forms, blank-separated sign normalization, precision,
  non-mutation, no-argument/omitted errors, and `40.11` invalid text. Dynamic
  invalid Level B decimal conversion is the already reproduced VM blocker.
- Performance/RXAS review evidence: stale Level B aggregate is 63 lines with a
  correct linear loop but binary-float comparisons; the rewritten noopt/opt
  module remains 63 lines and the hot loop is one decimal `dgt` plus selection,
  with no helper call or allocation. The standalone Level C BIF is 204 noopt/
  211 opt lines, validates/converts each argument once, scans linearly, and
  performs no name dispatch or substring work.
- Completion summary: all B/C/T/P/D/V gates are complete. Dynamic invalid
  decimal text raises and is caught as `CONVERSION_ERROR` in both Level B
  modes.

### 39. `min` — done

- Source/public surface: `lib/rxfnsb/rexx/min.crexx`, one exported variadic
  `min(first=.float, ...=.float) -> .float` procedure.
- Dependencies and known consumers: no imported helper; known uses include a
  file-I/O record bound and ordinary/RexxScript calls. There is no `.Rexx.min`
  adapter despite a stale object-style book example.
- Current Level B signature/error/algorithm findings: at least one argument is
  structurally required and the linear comparison loop is sound, but float
  input/result lose decimal precision. Dynamic invalid decimal conversion has
  the same known VM signal blocker as rows 36/38. Stale aggregate is 63 lines.
- Level C contract and current implementation/lowering status: `MIN(number, ...)`
  is recognized `rNUM...`, implemented only in the common controller, and has
  no standalone module/harness. The row-38 shared variadic CheckArgs support is
  now available.
- Existing Level B tests: the combined MIN/MAX smoke harness has three positive
  integer MIN cases only; it omits one argument, negatives, decimals, precision,
  ties, non-mutation, conversion behavior, and a pass marker.
- Existing Level C tests: the broad controller harness has one three-argument
  success and no direct/error/tie coverage.
- Existing Level B docs: the book describes Classic first-argument tie
  representation but does not distinguish native typed Level B behavior and
  includes an unavailable object-method example. No selector-local page exists.
- Existing Level C docs/spec clauses: variadic `rNUM...`, at least one argument,
  caller-context decimal comparison, and first selection on ties; no local page.
- Approved implementation notes or decisions: mirror the reviewed MAX design:
  native `.decimal` variadic Level B core, standalone RexxValue MIN using the
  shared variadic validator, first normalized Level C text on ties, deprecate
  the common body, and park only the shared VM dynamic invalid-decimal signal.
- Focused validation commands/results: valid Level B optimized/unoptimized
  overlays both report `PASS: min`, again compiling with the stale aggregate
  metadata temporarily hidden/restored instead of rebuilt. Coverage includes
  one/many arguments, signs, decimals, promotion, ties, non-mutation, and exact
  ordering beyond float precision. Standalone Level C overlays both report
  `PASS: Level C MIN BIF`, including representation-preserving ties, exponent
  forms, blank-separated signs, precision, no-argument/omitted errors, and
  invalid `40.11` cases. Dynamic invalid Level B conversion is the known VM
  blocker.
- Performance/RXAS review evidence: stale Level B aggregate is 63 lines with a
  correct linear loop but binary-float comparisons; rewritten noopt/opt remains
  63 lines and replaces the comparison with decimal `dlt`, retaining the one-
  pass/no-allocation core. Standalone Level C is 204 noopt/211 opt lines with
  one validation/conversion pass and one comparison pass, no dispatcher or
  substring work.
- Completion summary: all B/C/T/P/D/V gates are complete. Dynamic invalid
  decimal text raises and is caught as `CONVERSION_ERROR` in both Level B
  modes.

### 40. `numeric` — done

- Source/public surface: `lib/rxfnsb/rexx/numeric.crexx` exports five zero-
  argument accessors: `digits() -> .int`, `fuzz() -> .int`,
  `form() -> .string`, `numcase() -> .string`, and
  `standard() -> .string`.
- Dependencies and known consumers: the module imports no helper and each
  accessor reads one inherited VM numeric-context field. The three standard
  accessors are general caller-context services; `FORMAT` specifically depends
  on inherited DIGITS/FORM behavior. `numcase` and `standard` are Level B-only
  cREXX extensions.
- Current Level B signature/error/algorithm findings: all five signatures are
  already correctly typed and accept no arguments. Each function is constant
  time: one `getnum*` instruction, with a two-branch code-to-name mapping for
  string-valued accessors. There is no value-dependent error path. The existing
  source emitted 146 RXAS lines in the prior selector inventory.
- Level C contract and current implementation/lowering status: the repository
  Level C catalog requires only `DIGITS()`, `FORM()`, and `FUZZ()`. They return
  the immediate caller's current numeric settings. No standalone direct module
  or direct harness exists; current generated calls still use existing
  artifacts and are outside this task's lowering scope.
- Existing Level B tests: `ts_numeric` exercises all five accessors under two
  numeric contexts, but returns at the first failed context, repeats calls in
  diagnostics, and has no pass marker.
- Existing Level C tests: none for the standalone RexxValue entries.
- Existing Level B docs: `docs/books/crexx_language_reference/numeric.md`
  documents the NUMERIC instruction and retrieval functions, but there is no
  selector-local typed Level B page or code-adjacent RexxDoc for the five
  accessors.
- Existing Level C docs/spec clauses: the book's BIF table lists DIGITS, FORM,
  and FUZZ but their reference sections are empty comments. The direct
  zero-argument RexxValue API and standard excess-argument error are not
  documented separately.
- Approved implementation notes or decisions: retain the already minimal Level
  B algorithms, add code-adjacent contracts and a complete aggregating harness,
  and implement the three required Level C entries in one standalone selector
  module. Each direct procedure itself declares the relevant setting inherited,
  so it receives the immediate caller's context without adding state or
  per-call capture overhead to the shared BIF context. Use blank CheckArgs for
  the zero-argument contract and return native RexxValue integer/string views.
- Focused validation commands/results: selector-local Level B overlays report
  `PASS: numeric` in optimized and unoptimized modes. Standalone Level C
  overlays report `PASS: Level C NUMERIC BIFs` in both modes, calling each
  qualified direct entry under two inherited caller contexts and checking
  `RXC-LC-40.4` for an extra argument. Existing `library.rxbin` was reused and
  remained present; no aggregate target or fixture-aware CTest was run.
- Performance/RXAS review evidence: the reviewed Level B module remains 146
  lines in both modes. Each accessor contains exactly one matching `getnum*`
  instruction, no helper call, and only the required two comparisons for a
  string code-to-name mapping. The combined standalone Level C module is 248
  lines in both modes; each entry adds only context/error validation and
  RexxValue construction around the same single context read. It contains no
  name-controller call and does not burden unrelated BIF contexts.
- Completion summary: all gates complete. The five typed Level B accessors and
  the three distinct direct Level C BIFs are documented and independently
  proven in both modes. Aggregate wiring is queued; row 41 is active.

### 41. `sign` — VM gate complete; class adapter parked

- Source/public surface: `lib/rxfnsb/rexx/sign.crexx` exports
  `sign(number=.float) -> .int`. It has no imports. `.Rexx.sign` forwards its
  receiver string directly to this helper.
- Dependencies and known consumers: ordinary library calls and the class
  adapter are the only located consumers. SIGN shares the same numeric
  conversion boundary as ABS/MAX/MIN but needs only two comparisons after
  conversion.
- Current Level B signature/error/algorithm findings: `.float` can lose a
  nonzero decimal to binary underflow or collapse huge values during conversion;
  the correct foundation type is `.decimal`. The two-comparison constant-time
  algorithm is otherwise optimal. Dynamic invalid decimal binding has the
  already reproduced VM conversion-signal blocker. The stale aggregate module
  is 38 RXAS lines.
- Level C contract and current implementation/lowering status: SIGN is cataloged
  as `rNUM`, returning `-1`, `0`, or `1` after Classic numeric normalization.
  The deprecated common controller body already compares a RexxValue decimal,
  but no standalone direct module/harness exists.
- Existing Level B tests: `ts_sign` contains many repetitive binary-float and
  expression-conversion cases, two disabled cases, a massive repeated literal,
  duplicated test numbering, no pass marker, and no precise decimal-underflow
  boundary or non-mutation assertion. It cannot catch invalid dynamic decimal
  conversion while the shared VM blocker remains.
- Existing Level C tests: the broad common-module harness has one negative
  success case and no direct, count/presence, invalid-number, normalization,
  zero/exponent, precision, or non-mutation coverage.
- Existing Level B docs: the book documents the Classic string BIF only. There
  is no selector-local typed page or source RexxDoc, and the `.Rexx.sign` docs do
  not describe conversion behavior.
- Existing Level C docs/spec clauses: the repository contract requires `rNUM`
  and normalized numeric comparison. There is no standalone Level C page.
- Approved implementation notes or decisions: change the Level B core to
  `.decimal -> .int`, retain the two comparisons, simplify its harness to valid
  typed behavior including precision/underflow, and add distinct documentation.
  Add a standalone RexxValue SIGN using shared `rNUM` validation, standard
  errors, and decimal comparison; deprecate the common body. As with ABS, park
  only the Level B invalid dynamic conversion and `.Rexx.sign` adapter on their
  known shared integration dependencies.
- Focused validation commands/results: valid Level B optimized/unoptimized
  overlays both report `PASS: sign`, including nonzero decimal values below and
  above binary-float range. Harness compilation temporarily hid and immediately
  restored only stale aggregate metadata; `library.rxbin` is present. Direct
  Level C overlays both report `PASS: Level C SIGN BIF`, covering normalization,
  exponent extremes, signed zero, non-mutation, and `40.3`/`40.4`/`40.5`/`40.11`.
- Performance/RXAS review evidence: optimized Level B remains 38 lines (48
  unoptimized) and retains exactly two comparisons, now native `dgt`/`dlt`
  instead of binary-float operations, with no call or allocation. Standalone
  Level C is 146 noopt/145 opt lines: one validation/conversion followed by the
  same two comparisons and native integer RexxValue factories, with no name
  dispatch or numeric result-string construction.
- Completion summary: B/C/T/P/D/V are complete for the standalone selector,
  including catchable dynamic invalid-decimal input. Only the distinct
  Classic-normalizing `.Rexx.sign` adapter remains parked.

### 42. `trunc` — VM gate complete; class adapter parked

- Source/public surface: `lib/rxfnsb/rexx/trunc.crexx` exports
  `trunc(innum=.string, fraction=0) -> .string`. It has no imports but calls
  several other `rxfnsb` functions through the aggregate namespace.
- Dependencies and known consumers: `fmtmask`, `runmask`, and `.Rexx.trunc`
  consume it. The class adapter preserves the integer fraction but passes its
  receiver string into the numeric boundary. Decimal coefficient/exponent
  extraction is also directly relevant to the parked FORMAT design.
- Current Level B signature/error/algorithm findings: the numeric input is
  incorrectly `.string` and the fraction is inferred rather than `.int`.
  Exponents are converted through float and then integer (`stof`, power,
  multiply, `itos`), losing decimal precision and overflowing large values.
  The remainder makes repeated `pos`/`substr`/`left` calls and never rejects a
  negative fraction. The stale aggregate is 1,213 RXAS lines. Correct typed
  behavior is `.decimal`, non-negative `.int`, and `.string` result with no
  rounding.
- Level C contract and current implementation/lowering status: repository
  contract is `TRUNC(number [,digits])`, checklist `rNUM oWHOLE>=0`, preserving
  Classic fixed text including leading `0.` construction. No common or
  standalone implementation or harness exists.
- Existing Level B tests: `ts_trunc` is 341 lines of repetitive hand-written
  comparisons, commented examples/duplicates, repeated calls in diagnostics,
  no pass marker, no typed error assertion, and no robust exponent/precision/
  non-mutation coverage.
- Existing Level C tests: none.
- Existing Level B docs: the book describes the Classic string BIF only. There
  is no selector-local typed page or source RexxDoc; `.Rexx.trunc` documents the
  fraction but not numeric conversion.
- Existing Level C docs/spec clauses: the catalog requires Classic numeric
  normalization, non-negative whole digits, fixed non-exponent output, no
  rounding, and zero padding. No standalone page exists.
- Approved implementation notes or decisions: implement the Level B core as
  `.decimal, .int -> .string`, reject negative fractions with
  `INVALID_ARGUMENTS`, and use native decimal coefficient/exponent extraction
  plus direct substring/padding assembly rather than float arithmetic or helper
  calls. Add a standalone RexxValue implementation that parses the already
  validated Classic numeric text without converting it through binary float or
  a fixed-precision decimal cache, preserving its mantissa. This row establishes
  the decimal-decomposition evidence needed by FORMAT but does not copy FORMAT's
  rounding/layout policy. Park only the known invalid dynamic decimal binding
  and `.Rexx.trunc` adapter integration dependency if they recur.
- Focused validation commands/results: valid Level B optimized/unoptimized
  overlays both report `PASS: trunc`, covering truncation, exact zero padding,
  positive/negative exponents, signed zero, precision beyond binary float,
  non-mutation, and catchable `INVALID_ARGUMENTS` for a negative fraction.
  Direct Level C overlays both report `PASS: Level C TRUNC BIF`, including a
  25-digit mantissa, huge-exponent zero, optional omission, and standard
  `40.3`/`40.4`/`40.5`/`40.11`/`40.12`/`40.13` errors. Stale aggregate metadata
  was hidden/restored only while compiling the Level B harness.
- Performance/RXAS review evidence: rewritten Level B is 527 noopt/522 opt lines
  versus 1,213 stale aggregate lines. It uses one `dextr`, one required
  coefficient materialization, direct scans/slices/padding, and no helper call,
  float conversion, exponent power/multiply, or integer reformat. Standalone
  Level C is 864 lines in both modes, scans input leading zeros once and writes
  output linearly; it contains no float instructions, decimal-cache conversion,
  typed TRUNC call, or name dispatch.
- Completion summary: B/C/T/P/D/V are complete for the standalone selector,
  including catchable dynamic invalid-decimal input. Only the distinct
  Classic-normalizing `.Rexx.trunc` adapter remains parked.

### 88. `getenv` — done

- Source/public surface: `lib/rxfnsb/rexx/getenv.crexx` exports only
  `getenv(env_name=.string) -> .string`; the argument and result were already
  correctly typed and read-only. There is no Level C BIF or class adapter.
- Dependency/error result: the implementation delegates name interpretation
  and lookup to the VM `getenv` instruction. Per its instruction contract,
  missing and empty-valued variables both return empty and no VM signal is
  raised; allocation failure is fatal. No selector-local validation was added
  that would conflict with platform name rules.
- Test/doc result: the new focused harness covers an injected value containing
  a blank, repeatability, missing and empty names, and name non-mutation. New
  RexxDoc and stable Level B Markdown document the typed surface, the
  missing-versus-empty boundary, platform delegation, and absence of Level C.
- Focused validation and second review: optimized and unoptimized overlays both
  report `PASS: getenv`. The selector remains 37 RXAS lines in both modes,
  identical to baseline, with exactly one native `getenv` operation, no helper
  call, no intermediate copy, and no extra branch. `git diff --check` passes and
  compiler/interpreter diffs remain empty.
- Completion summary: every applicable B/T/P/D/V gate is complete. Deferred
  CMake registration is queued; row 89 `linesize` is sole active.

### 89. `linesize` — done

- Source/public surface: `linesize() -> .int` and `.Rexx.linesize()` expose the
  portable Level B compatibility constant. Both signatures are already typed;
  there are no inputs, error cases, dependencies, or Level C BIF.
- Implementation result: the portable value remains `999999999`, consistent
  with the source's z/VM compatibility intent. The redundant local assignment
  was removed so the selector returns the literal directly; platform builds
  remain free to replace this module for a smaller native limit.
- Test/doc result: the existing focused harness is simplified and now has exact
  value, repeatability, clear diagnostics, and a PASS marker. RexxDoc and stable
  Level B Markdown distinguish the portable constant from a host discovery API
  and record the `.Rexx` forwarding surface.
- Focused validation and second review: optimized and unoptimized overlays both
  report `PASS: linesize`. RXAS falls from 24 noopt / 19 opt lines to 17 in both
  modes, consisting of the metadata and one immediate return with no allocation,
  branch, or helper call. `git diff --check` passes and compiler/interpreter
  diffs remain empty.
- Completion summary: all applicable B/T/P/D/V gates are complete; the enhanced
  existing CMake test is retained for the final build. Row 90 `filter` is sole
  active.

### 90. `filter` — done

- Source/public surface: `filter(input_value=.string, filter=.string) ->
  .string` is a Level B-only Unicode character-set removal helper. Both inputs
  and the result were already correctly typed; there is no class or Level C
  surface.
- Correctness/error result: every source code point present in the removal set
  is dropped, duplicates are idempotent, and order is preserved. Empty source/
  set behavior and input non-mutation are explicit. The VM reports invalid UTF-8
  as `UNICODE_ERROR`; allocation failure remains fatal.
- Test/doc result: the former two-line print demo is now an asserting harness
  covering ASCII sets, duplicate entries, Unicode code points, empty/all
  boundaries, saved result, and non-mutation, with a PASS marker. RexxDoc and a
  stable Level B page document the exact typed contract and VM error boundary.
- Focused validation and second review: optimized and unoptimized overlays both
  report `PASS: filter`. Output remains 47 RXAS lines in both modes, with one
  result initialization and one native `dropchar`; there are no Level B helper
  calls or normalized/intermediate copies. Retaining the VM's direct scan avoids
  Level B collection/dispatch overhead. `git diff --check` passes and compiler/
  interpreter diffs remain empty.
- Completion summary: all applicable B/T/P/D/V gates are complete; existing
  CMake wiring is retained for the final sweep. Row 91 `sequence` is sole active.

### 91. `sequence` — done

- Source/public surface: Level B `sequence(from=.string, tos=.string) ->
  .string` and `.Rexx.sequence(tos)` define an inclusive, non-wrapping Unicode
  range. There is no Level C BIF; byte-oriented wrapping XRANGE remains a
  separate API.
- Correctness/error result: endpoints must each contain exactly one character
  and must be ascending. Invalid or descending inputs now signal
  `INVALID_ARGUMENTS` instead of printing to stdout and returning `BAD`.
  Unicode surrogate code points are skipped, so even a range crossing that gap
  produces a valid scalar-value string through U+10FFFF.
- Algorithm result: the two endpoint code points are read directly; the result
  is appended once in ascending order. The old C2D calls, subtraction/control
  copy, stdout path, and accidental dependence on conversion semantics are gone.
- Test/doc result: the focused harness now asserts ASCII, Greek, equal
  supplementary/highest-scalar endpoints, surrogate-gap behavior,
  non-mutation, and all endpoint signals with a PASS marker. RexxDoc, a stable
  Level B page, the language book, and class RexxDoc now agree.
- Focused validation and second review: both overlays report `PASS: sequence`.
  Output is 159 lines in both modes versus 107 noopt / 182 opt at baseline; the
  explicit validation grows noopt, while opt shrinks and both now contain zero
  helper calls, one bounded linear append loop, no intermediate collection, and
  no diagnostic I/O. `git diff --check` passes and compiler/interpreter diffs
  remain empty.
- Completion summary: all applicable B/T/P/D/V gates are complete. Existing
  test plus final class example execution are queued; row 92 `find` is sole
  active.

### 92. `find` — done

- Source/public surface: `find(needle=.string, haystack=.string
  [,start=.int]) -> .int` is the Level B-only VM/TSO argument-order alias for
  `wordpos`; there is no class method or Level C BIF. Generated metadata confirms
  the defaulted `start=1` is an optional `.int`.
- Correctness/error result: exact case-sensitive phrase matching, Unicode blank
  equivalence, empty input, and non-positive-start `INVALID_ARGUMENTS` are
  inherited from the already reviewed row 82 algorithm. The wrapper preserves
  FIND's needle-first/haystack-second order and does not mutate either input.
- Test/doc result: 190 lines of commented-out WORDPOS experiments were replaced
  by a compact asserting FIND harness for order, phrases, start, case, prefix
  rejection, Unicode blanks, empty input, non-mutation, and propagated signal.
  RexxDoc and a stable Level B page document the alias boundary and no Level C.
- Focused validation and second review: both overlays report `PASS: find`
  against freshly compiled `wordpos`. FIND remains 51 RXAS lines in both modes:
  one fixed delegation and no independent scan/copy/allocation. Duplicating the
  492-line word matcher here would add maintenance and instruction-image cost
  while the one call is constant beside the required search. `git diff --check`
  passes and compiler/interpreter diffs remain empty.
- Completion summary: all applicable B/T/P/D/V gates are complete; focused test
  registration is retained for final integration. Row 93 `index` is sole active.

### 93. `index` — done

- Source/public surface: `index(haystack=.string, needle=.string
  [,start=.int]) -> .int` is the Level B-only VM/TSO haystack-first substring
  search. There is no class method or Level C BIF. The defaulted start emits
  optional `.int` metadata.
- Correctness/error result: the unnecessary `arg expose` alias on the haystack
  was removed. INDEX now preserves exact/case-sensitive Unicode character
  positions, returns zero for empty inputs/misses, signals `INVALID_ARGUMENTS`
  for non-positive starts, and leaves both arguments untouched.
- Algorithm result: rather than paying a POS function call, INDEX performs the
  same small validation and invokes native `strpos` directly. It holds only the
  required start/result register and allocates no substring or copied input.
- Test/doc result: the one-case legacy test is now a compact harness for public
  order, overlapping starts, case, Unicode positions, empties, non-mutation,
  and the signal. RexxDoc and a stable Level B page document the exact contract.
- Focused validation and second review: optimized/unoptimized overlays both
  report `PASS: index`. Static output grows from the 51-line delegated wrapper
  to 81 lines in both modes for explicit validation, while runtime removes its
  helper call and executes one `strpos` with zero text copies. `git diff --check`
  passes and compiler/interpreter diffs remain empty.
- Completion summary: all applicable B/T/P/D/V gates are complete; test wiring
  is retained for final integration. Row 94 `_datei` is sole active.

### 94. `_datei` — parked

- Public surface: `_rxsysb._datei(idate=.string, format=.string
  [,isep=.string]) -> .int` is the private input-format half of the extended
  Level B DATE implementation; there is no Level C BIF under this helper name.
- Co-dependency/correctness finding: its accepted abbreviations, separators,
  two-digit-year policy, and JDN epoch are shared directly with `_dateo`, `_jdn`,
  and `date`. Invalid formats call `raise` with `40.28` then return `-1`, while a
  malformed split prints to stdout and still continues. Month/day validity is
  not checked before `_jdn` normalizes it.
- Performance/test/doc evidence: the current aggregate is 11,931 RXAS lines,
  repeatedly invoking ABBREV, SUBSTR, WORD, WORDPOS, RIGHT, WORDS, UPPER, and
  concatenation across format branches. The only direct use is a print-oriented
  DATE demo; no assertion freezes parsed fields, invalid dates, errors, or
  round trips, and no stable helper Markdown exists.
- Integration follow-up: the corrected exact WORDPOS implementation exposed
  that NORMAL and QUALIFIED month abbreviations had depended on WORDPOS's old
  accidental prefix behavior. A typed DATE-local month mapper now preserves
  that established compatibility without weakening WORDPOS. Fixed NORMAL and
  QUALIFIED full/abbreviated cases pass through `ts_date2` in both modes.
- Completion summary: the broader selector remains parked on the shared
  extended DATE format, validation/error, and round-trip harness checkpoint;
  the narrow compatibility repair does not settle those contracts. Row 95
  `_dateo` is sole active.

### 95. `_dateo` — parked

- Public surface: `_rxsysb._dateo(jdn=.int, format=.string
  [,osep=.string]) -> .string` is the private output-format half of extended
  Level B DATE. There is no Level C BIF with this helper name.
- Co-dependency/correctness finding: format abbreviation and separator behavior
  must round-trip row 94. BASE, UNIX, JDN, DAYS, CENTURY, and EPOCH currently
  return integers through a string contract; unknown formats silently fall back
  to `dd mm yyyy`; and USA/XUSA currently emit the same four-digit year despite
  separate format names. None of those ambiguities can be corrected without
  freezing the shared DATE surface.
- Performance/test/doc evidence: the aggregate is 14,367 RXAS lines, dominated
  by repeated abbreviation tests, RIGHT/WORD/SUBSTR formatting, and repeated
  `_jdn(1,1,year)` calls. `ts_date` only prints direct helper results; it has no
  expected values, invalid-format cases, type assertions, or inverse checks.
  No separate stable helper page exists.
- Completion summary: parked without implementation/build on the same format,
  return-type, validation, and round-trip contract as rows 94/96/97. Row 96
  `_jdn` is sole active.

### 96. `_jdn` — parked

- Public surface: `_rxsysb._jdn(day=.int, month=.int, year=.int) -> .int` is a
  typed internal Gregorian-to-Julian-day helper used only by the DATE cluster;
  there is no Level C BIF under this name.
- Correctness/error dependency: the standard integer formula is correct for
  valid proleptic-Gregorian fields, but the helper accepts invalid months/days
  and normalizes them arithmetically. Adding `INVALID_ARGUMENTS` here would leak
  through the current public DATE path, while Classic DATE requires conversion
  error `40.19`; that translation belongs in the shared cluster checkpoint.
  Year-zero/negative-year scope is likewise not documented.
- Performance/test/doc evidence: the current aggregate is only 97 RXAS lines,
  containing fixed integer arithmetic with no helper call or allocation, so no
  valid-path algorithm replacement is indicated. There is no focused leap/
  century/boundary/invalid/round-trip harness and no stable helper page.
- Completion summary: parked without code/build specifically on calendar-domain
  validation and DATE error translation, not performance. Row 97 `date` is sole
  active.

### 97. `date` — parked

- Public contracts: Level B `date([oformat=.string [,idate=.string
  [,iformat=.string [,osep=.string [,isep=.string]]]]]) -> .string` is an
  extended five-argument formatter. Classic Level C is separately specified as
  `DATE([option [,date [,inoption]]])` with checklist `oBDEMNOSUW oANY
  oBDENOSU`; these surfaces must not be merged.
- Dependency/correctness finding: Level B delegates all conversion to rows
  94-96, truncates separators silently, uppercases the date payload, and has no
  complete invalid-date/format signal translation. Current-date evaluation
  reads wall time directly. Level C instead requires frozen clause time,
  `Time2Date`/`Leap`, and `40.19`; no clause-time service, standalone RexxValue
  DATE, or common-controller DATE body exists.
- Performance/test/doc evidence: the wrapper aggregate is 1,854 RXAS lines in
  addition to the 11,931/14,367-line input/output helpers. `ts_date2` has useful
  fixed conversion assertions but mixes them with time-dependent printing,
  duplicate coercion comparisons, disabled cases, repeated calls in failures,
  and no invalid conversion coverage. `ts_date` is almost entirely a print demo.
  The language book documents only the Level B extension and there are no
  separate stable B/C pages.
- Integration follow-up: `ts_date2` now also asserts a full NORMAL month name
  and an abbreviated QUALIFIED month name. The month-prefix regression and all
  eight previously stale RXAS/AST expectations are closed; the final unchanged
  full Debug rerun passes 1,794/1,794 tests.
- Completion summary: the selector remains parked on the shared calendar core,
  strict Level B error contract, frozen-clause-time service, direct Classic
  implementation, and separate harnesses/docs. Compiler lowering remains out
  of scope. Row 98 `random` is sole active.

### 98. `random` — parked

- Public contracts: Level B currently exposes three optional defaulted integer
  slots `random([min [,max [,seed]]]) -> .int`; Classic Level C separately
  defines `RANDOM([max])` or `RANDOM([min [,max [,seed]]])` with
  `oWHOLE>=0` arguments and a maximum range width of 100000.
- Contract/error finding: the current Level B one-argument call means
  `min..999`, unlike Classic's `0..max`; `-1` sentinels lose the distinction
  between omission and explicit invalid values; all three arguments are
  unnecessarily exposed; and errors call the placeholder `raise`, print, then
  return magic negative values rather than signalling. Changing these choices
  is a public Level B contract decision.
- Algorithm/service finding: the VM `irand` instruction owns process-global C
  `rand()` state. Applying `% (max-min+1)` is biased and cannot reach much of a
  range wider than the platform `RAND_MAX`; native range arithmetic can also
  overflow at the full nonnegative `.int` boundary. The Level C specification
  explicitly lists random seed/next-value as a missing configuration service.
- Test/doc finding: `ts_random` is only a sequence of printed values, includes
  an intentionally invalid final call, and asserts neither deterministic
  reseeding, bounds, omission, errors, state isolation, nor distribution
  reachability. The language page describes only current Level B defaults and
  no separate stable B/C pages or standalone direct harness exist.
- Completion summary: parked before implementation/build on the Level B
  optional-call decision and scoped unbiased RNG service. That checkpoint can
  then produce distinct typed B and direct RexxValue C implementations with
  standard `40.31`/`40.32`/`40.33`, without compiler changes. Row 99 `reradix`
  is sole active.

### 99. `reradix` — done

- Public surface: `_rxsysb.reradix(subject=.string, FromRadix=.int,
  ToRadix=.int) -> .string` plus `.Rexx.reradix(fromRadix,toRadix)` convert an
  unsigned value between ASCII radices 2 through 16. There is no Level C BIF.
- Correctness/error result: the new implementation is exact beyond native
  integer size, accepts lowercase input, returns uppercase, preserves the
  established binary/hex width rules, normalizes other leading zeros, and does
  not mutate input. Empty text, invalid radices/characters/digits now signal
  `INVALID_ARGUMENTS`; the stdout/`?` overflow path is removed.
- Algorithm result: binary-to-hex groups at most four bits and hex-to-binary
  emits four bits per digit, both linearly. Other conversions use little-endian
  target digits with multiply/add intermediates below 256. There is no decimal
  or scientific conversion, native overflow, repeated front-prepend, general
  string selector, or helper call.
- Test/doc result: the repetitive native-sized test is replaced with an
  asserting harness for lower/uppercase, binary/hex width, zero, arbitrary
  radices, a 128-bit boundary, round trip, non-mutation, and every error family.
  RexxDoc and stable Level B Markdown cover the class forwarding surface and no
  Level C.
- Focused validation and second review: both overlays report `PASS: reradix`.
  RXAS changes from 277 noopt / 722 opt to 614/606: explicit exact arrays and
  validation grow noopt, while opt shrinks, all 8-10 former helper calls vanish,
  and common binary/hex runtime becomes linear. `git diff --check` passes and
  compiler/interpreter diffs remain empty.
- Completion summary: all applicable B/T/P/D/V gates are complete; focused and
  class-forwarding tests are queued for final integration. Row 100 `time` is
  sole active.

### 100. `time` — parked after Level B completion

- Public contracts: Level B `time([option=.string]) -> .string` exposes local/
  UTC clocks, elapsed/reset state, timezone data, process ticks, and `US` as an
  extension. Classic Level C is the separate
  `TIME([option [,time [,inoption]]])` contract with checklist
  `oCEHLMNORS oANY oCHLMNS`, conversions, frozen clause time, and `40.19`/`40.29`.
- Level B correctness result: unsupported options now signal
  `INVALID_ARGUMENTS`; `L` zero-pads six fractional digits on the correct side;
  `C` uses 01-12 with correct noon/midnight suffixes; UTC is normalized across
  local-day boundaries; elapsed seconds divide microseconds correctly, account
  for midnight rollover, and `R` returns the elapsed value before reset.
- Level B algorithm result: option folding uses one native copy; ZD/T/TS/ZN
  return immediately after one opcode, and benchmark-hot `US` returns directly
  after `mtime` without decomposition. Numeric decomposition is delayed until
  required and clock text is appended directly, removing RIGHT/LEFT/UPPER and
  all other general selector calls. Only E/R call the required RXAS `_elapsed`.
- Test/doc result: the print/large-loop demo is now an asserting format/range/
  elapsed/reset/option harness with a PASS marker. Stable Level B Markdown and
  RexxDoc document the extension; separate Level C Markdown records its precise
  pending direct contract. The ZN assertion avoids the known VM `xtime "N"`
  character-count cache gap while still verifies nonempty returned text.
- Focused validation and second review: both overlays report `PASS: time`.
  RXAS moves from 506 noopt / 612 opt with 12/11 calls to 735/724 with only the
  two elapsed-state call sites; added static code is direct validation and
  formatting, while `US` and other raw clock paths are substantially shorter
  and allocation-free. `git diff --check` passes and compiler/interpreter diffs
  remain empty.
- Completion summary: typed Level B B/P/D and focused behavior are complete.
  C/T/V remain parked on shared DATE/TIME local adjustment, `Time2Date`, frozen
  clause time, and elapsed state services; no compiler lowering change was made.
  Row 101 `fnv` is sole active.

### 101. `fnv` — parked

- Public surface: `fnv(input_value=.string) -> .int` is Level B-only and is
  consumed by treemap bucket macros. Its argument/result are typed and read-only;
  there is no Level C BIF or class method.
- Contract blocker: despite the public name, the authoritative VM `rxhash`
  opcode implements a nonnegative reverse-order SDBM-style recurrence, not
  FNV-1/FNV-1a. Other repository components use conventional 32/64-bit FNV-1a,
  so silently changing this selector would alter bucket compatibility and needs
  an explicit public algorithm decision.
- VM correctness blocker: `rxhash` obtains a Unicode character count and then
  uses it as a byte-loop bound. A direct probe showed `"a🙂b"` is therefore
  hashed from only a prefix of its UTF-8 payload. The supplied length register
  is ignored, so Level B cannot repair this by passing a byte count; fixing the
  opcode is an interpreter change outside this library-only programme.
- Performance/test/doc evidence: current output is 48 lines in both modes and
  redundantly executes `strlen` before an opcode that derives length again.
  The test merely prints four unlabelled values and has no empty/Unicode/vector/
  non-mutation assertions; no stable page identifies the actual algorithm.
- Completion summary: parked before source/test/doc changes on the public hash
  algorithm and VM Unicode-byte-length repair. Do not optimize away the visible
  pre-scan and then freeze defective vectors. Row 102 `arraypop` is sole active.

### 102. `arraypop` — done

- Public surface: `arraypop(expose array=.string[] [,default=.string]) ->
  .string` is a Level B-only mutator; no Level C BIF or class method exists.
  Exposure is intentional because removal is the contract.
- Correctness/result: a nonempty array returns and removes exactly its last
  value, including an empty string. Empty input returns the explicit or omitted
  default without mutation. The high-water mark remains the one source of size.
- Test/doc result: a focused harness now covers Unicode/empty elements,
  repeated mutation, retained values/high-water state, empty input, and both
  default forms. RexxDoc and stable Level B Markdown document mutation and no
  Level C.
- Focused validation and second review: optimized/unoptimized overlays both
  report `PASS: arraypop`. Output remains 61 RXAS lines in both modes, with one
  high-water read and one native `delattrs1` only on the nonempty path; there is
  no scan, shift, helper call, or Level B allocation. `git diff --check` passes
  and compiler/interpreter diffs remain empty.
- Completion summary: all applicable B/T/P/D/V gates are complete; focused
  registration is queued. Row 103 `arrayshift` is sole active.

### 103. `arrayshift` — done

- Public surface: `arrayshift(expose array=.string[] [,default=.string]) ->
  .string` is a Level B-only front-removal mutator; there is no Level C or class
  surface. The exposed array is required by the operation.
- Correctness/result: first values, including empty strings, are returned and
  removed; subsequent indices and high-water state shift down exactly once.
  Empty input returns the explicit/omitted default without mutation.
- Test/doc result: a focused harness covers Unicode/empty values, moved indices,
  repeated mutation, defaults, and high-water state. RexxDoc and stable Level B
  Markdown document the shift and no Level C.
- Focused validation and second review: both overlays report
  `PASS: arrayshift`. Output remains 52 lines in both modes, with one first-value
  read and one native `delattrs1`; the required O(n) move stays inside the VM
  instead of a Level B loop, with no helper call. `git diff --check` passes and
  compiler/interpreter diffs remain empty.
- Completion summary: all applicable B/T/P/D/V gates are complete; registration
  is queued. Row 104 `arrayreverse` is sole active.

### 104. `arrayreverse` — done

- Public surface: `arrayreverse(expose array=.string[]) -> .int` reverses a
  Level B array in place and returns its unchanged high-water mark. There is no
  Level C BIF or class method; exposure is required for mutation.
- Correctness/result: empty, single, odd, even, empty-string, and Unicode values
  reverse in place; applying the operation twice restores the array.
- Algorithm result: the existing two-index half-swap is the optimal O(n), O(1)
  algorithm. The high-water mark is now cached once for both the right cursor
  and return, eliminating a second attribute read without allocating a copy.
- Test/doc result: a focused harness covers all size/value shapes, return state,
  and double reversal. RexxDoc and stable Level B Markdown document mutation,
  complexity, and absence of Level C.
- Focused validation and second review: both overlays report
  `PASS: arrayreverse`. Output changes from 94 to 98 RXAS lines in each mode for
  the cached local, while high-water reads fall from two to one; the loop still
  has exactly floor(n/2) swaps and zero helper calls. `git diff --check` passes
  and compiler/interpreter diffs remain empty.
- Completion summary: all applicable B/T/P/D/V gates are complete; registration
  is queued. Row 105 `arrayjoin` is sole active.

### 105. `arrayjoin` — done

- Public surface: `arrayjoin(array=.string[] [,separator=.string]) -> .string`
  is a read-only Level B helper with no Level C or class surface.
- Correctness/result: separators occur only between adjacent elements, including
  empty-string elements; empty and single arrays, omitted/multicodepoint
  separators, Unicode content, and input state are all preserved correctly.
- Algorithm result: the count is cached once and every separator/element is
  appended to one growing result. This removes the prior per-element expression
  concatenation result while retaining O(total output text) behavior.
- Test/doc result: a focused harness covers all boundary/value/separator shapes
  and non-mutation. RexxDoc and stable Level B Markdown document placement,
  complexity, and no Level C.
- Focused validation and second review: both overlays report `PASS: arrayjoin`.
  Output grows from 82 to 98 RXAS lines in both modes for explicit append/cached
  state, but the runtime loop now has two in-place append sites, no temporary
  concat result, one high-water read, and zero helper calls. `git diff --check`
  passes and compiler/interpreter diffs remain empty.
- Completion summary: all applicable B/T/P/D/V gates are complete; registration
  is queued. Row 106 `arraysort` is sole active.

### 106. `arraysort` — done

- Public surface: `arraysort(expose array=.string[] [,offset=.int
  [,order=.string [,debug=.int]]]) -> .int` sorts a one-based Level B string
  array in place by a case-sensitive substring key and returns the unchanged
  high-water mark. There is no Level C BIF or class method.
- Correctness/error result: offset is a positive one-based character position;
  order accepts only case-insensitive `ASC`/`DESC`; debug accepts only 0/1 and
  is now off by default. Invalid values signal `INVALID_ARGUMENTS` rather than
  clamping, selecting an implicit direction, or printing placeholder errors.
- Algorithm result: every Unicode substring key is extracted once, then values
  and keys move together through an in-place binary heap. This replaces the
  repeated-SUBSTR Shell-sort comparisons with O(n log n) comparisons and O(n)
  key storage; the sort core has no general selector calls.
- Test/doc result: focused valid coverage exercises empty/single arrays, both
  orders, prefix-related numeric text, substring keys, Unicode, return state,
  and mutation. The error harness verifies offset/order/debug signals
  sequentially in one VM and checks caller-array preservation after every
  catch. RexxDoc and stable Level B Markdown document the exact behavior,
  complexity, and lack of a Level C surface.
- Focused validation and second review: optimized and unoptimized overlays both
  report `PASS: arraysort`; both repeated-signal error tests pass. RXAS
  changes from 377 noopt / 1,231 opt lines to 765/757: the original noopt body
  has five helper calls and optimized output contains twelve inlined substring
  sites, while the replacement has zero calls and one key-extraction substring
  site in each mode. `git diff --check` passes and compiler/interpreter diffs
  remain empty.
- Completion summary: all applicable B/T/P/D/V gates are complete. The shared
  caught-signal call-window repair is covered by the one-VM repeated-signal
  harness and the focused 63-test integration gate.

### 107. `arraydump` — parked before implementation

- Public surface: `arraydump(array=.string[] [,from=.int [,to=.int
  [,flags=.string [,header=.string [,prefix=.string]]]]]) -> .int` prints a
  header and selected formatted elements, returning the number of elements.
  It is Level B-only, but the compiler pprint exit emits calls to it for the
  source-level `DUMP ARRAY` convenience form.
- Contract blockers: invalid starts are silently clamped, nonpositive/oversize
  ends silently become the high-water mark, and unknown flags are ignored.
  The comment promises doubled embedded single quotes but the helper only
  switches quote style when the first character is a quote and does not escape
  either delimiter. Empty-range prefixing, header inclusion, index zero-padding,
  and `N` rendering also disagree with duplicated row 108 behavior.
- Test/doc finding: there is no functional harness or selector-local Markdown.
  The books provide only a signature/one-line summary and do not freeze the
  flag, range, escaping, header, or signal contracts. No Level C BIF exists.
- Performance baseline: isolated output is 758 noopt / 1,940 opt RXAS lines.
  The noopt path has 21 calls and 35 concatenations; optimized output still has
  17 calls and 46 concatenations. It repeatedly invokes general UPPER/POS/
  RIGHT/LENGTH/STRIP/SUBSTR/C2D/D2X selectors and rebuilds growing strings.
- Completion summary: parked before edits on one shared `arraydump`/
  `arrayformat` output-and-error contract, because changing compiler-exit-visible
  text selector-locally would be a public-contract decision. Resume by adding a
  shared vector harness first, then implement the agreed formatter without any
  compiler/lowering change. Row 108 `arrayformat` is sole active.

### 108. `arrayformat` — parked before implementation

- Public surface: `arrayformat(array=.string[] [,from=.int [,to=.int
  [,flags=.string [,header=.string [,prefix=.string]]]]]) -> .string[]` is a
  Level B-only formatter with no current consumer, Level C BIF, or class method.
- Contract blockers: it duplicates row 107's range, flag, escaping, and
  non-printable ambiguities. In addition, a function documented as returning a
  formatted array prints its header to stdout, does not include that header in
  the result, treats `C` only as a choice between two side effects, and pads
  indices with spaces where `arraydump` pads them with zeroes.
- Test/doc finding: there is no harness or selector-local Markdown. The books
  provide only a signature and one-line return summary, which is insufficient
  to choose between a pure result array and mixed returned/printed output.
- Performance baseline: isolated output is 765 noopt / 1,947 opt RXAS lines.
  Like row 107 it has 21 noopt / 17 opt calls, 35/46 concatenations, and no
  in-place append; it also retains header `say` instructions despite returning
  an array.
- Completion summary: parked with row 107 on the shared formatter contract.
  One agreed vector table must drive both selectors, after which this function
  can be made pure or explicitly documented as side-effecting without changing
  compiler lowering. `git diff --check` passes and compiler/interpreter diffs
  remain empty. Row 109 `qpos` is sole active.

### 109. `qpos` — parked before implementation

- Public surface: `qpos(needle=.string, text=.string [,start=.int]) -> .int`
  returns the first 1-based match outside single- or double-quoted regions.
  It is Level B-only and is a foundation dependency of qsplit/extraction/
  comment helpers, fsayfmt, and the classlib Qfind implementation.
- Contract blocker: source documentation explicitly says quote doubling and
  escaping are unsupported, while the registered test expects doubled double
  quotes to keep a delimiter inside the quoted region. Starts below one are
  silently clamped and unmatched quotes consume the remainder; neither behavior
  is specified in stable documentation. Changing these semantics here would
  transitively change every structural q-family consumer.
- Current validation: the existing 15-case suite passes in optimized and
  unoptimized isolated overlays when linked only with its test-formatting RIGHT
  dependency. It covers useful ordinary/quoted/start/unmatched cases but has no
  assertions that distinguish doubled-quote grammar variants, Unicode positions,
  invalid starts, or source non-mutation.
- Performance baseline: isolated output is 381 noopt / 374 opt RXAS lines with
  zero calls, six native `strpos` sites, and one one-character substring site.
  The broad shape is already allocation-light, but the final one-pass state
  machine cannot be reviewed until its quote transitions are authoritative.
- Completion summary: parked on the shared quote grammar before source/test/doc
  edits. No Level C BIF exists. `git diff --check` passes and compiler/
  interpreter diffs remain empty. Row 110 `qsplit` is sole active.

### 110. `qsplit` — parked before implementation

- Public surface: `qsplit(text=.string, separator=.string) -> .string[]` splits
  on matches outside quotes. It is Level B-only and directly depends on qpos
  plus general SUBSTR/STRIP/LENGTH helpers.
- Contract blockers: its source claims doubled-quote escaping although qpos's
  source denies it. It strips each field before a separator but preserves the
  final field verbatim, and an empty separator silently produces one unchanged
  field. Neither whitespace nor empty-separator behavior is documented or
  tested, so correcting one side would make an unapproved public choice.
- Test/doc finding: no functional harness or selector-local Markdown exists;
  the language book only says that quoted separators are ignored. It does not
  define empty/trailing fields, whitespace, unbalanced/doubled quotes, Unicode,
  errors, or input preservation. No Level C BIF exists.
- Performance baseline: isolated output is 171 noopt / 600 opt RXAS lines.
  Noopt has five calls (qpos, two SUBSTR, STRIP, LENGTH); optimized output still
  calls qpos and STRIP. A final implementation should scan/slice once from the
  frozen grammar instead of repeatedly restarting qpos.
- Completion summary: parked on the shared quote/field grammar before edits.
  Row 111 `qsplitsafe` is sole active.

### 111. `qsplitsafe` — parked before implementation

- Public surface: `qsplitsafe(text=.string, separator=.string [,start=.int
  [,pairs=.string]]) -> .string[]` splits outside quotes and nested delimiter
  pairs. It is Level B-only and contains two internal pair-stack procedures.
- Contract blockers: quote doubling/escaping is unresolved; an odd-length
  `pairs` string silently disables nesting; mismatched closers are ignored;
  unclosed openers suppress later splits without error. Starts below one clamp,
  while a positive start initializes both scan and output origin there and thus
  discards the text prefix. Empty separators silently return the original text.
- Test/doc finding: there is no harness or selector-local Markdown. The book
  names default `()` nesting but defines none of the validation, error, quote,
  start, trailing-field, Unicode, or non-mutation cases. No Level C BIF exists.
- Performance baseline: isolated output is 625 noopt / 2,514 opt RXAS lines.
  Noopt executes twelve general/internal call sites and six slicing sites;
  optimization expands the helpers to 24 slicing sites. The per-character
  pair scan is also linear in the configured pair count, so the final scanner
  should use one frozen state-machine contract rather than this helper layering.
- Completion summary: parked on the shared quote/nesting/field grammar before
  edits. Row 112 `qextractall` is sole active.

### 112. `qextractall` — parked before implementation

- Public surface conflict: source metadata is
  `qextractall(open=.string, close=.string, text=.string [,mode=.string]) ->
  .string[]`, while its RexxDoc-like block, language-book signature, and example
  advertise a fourth `start` argument. It is Level B-only and feeds qremoveall.
- Correctness blockers: each result comes from qextractpair, but the next cursor
  is inferred as `first + length(result) - 1`. Exclusive results omit both
  delimiters and inclusive results include them, so this arithmetic cannot
  reliably advance beyond the consumed close and may rescan nested/adjacent
  content. Empty delimiters and incomplete pairs silently end the scan.
- Test/doc finding: no harness or stable selector page exists. The long source
  block claims start semantics that do not exist and does not define the
  implemented X/I/C/E modes, their returned delimiters, failure, quote grammar,
  adjacent pairs, or input preservation. No Level C BIF exists.
- Performance baseline: isolated output is 202 noopt / 214 opt RXAS lines.
  Noopt has five calls; optimized output still calls qpos and qextractpair once
  per segment. It also computes an unused opening-delimiter length.
- Completion summary: parked on the extraction ABI/mode/consumed-position
  contract and the underlying quote grammar before edits. Row 113
  `qextractpair` is sole active.

### 113. `qextractpair` — parked before implementation

- Public surface: `qextractpair(open=.string, close=.string, text=.string
  [,start=.int [,mode=.string]]) -> .string` returns one balanced region or
  empty text. It is Level B-only and is the structural core for qextractall and
  block-comment removal.
- Contract blockers: only the first mode letter is read; `E` maps to `X`, `C`
  and `I` are identical inclusive modes, and unknown modes silently become X.
  An opening delimiter beginning with `/*` selects a separate literal,
  non-nesting, non-quote-aware path, while all other delimiters use qpos and
  nesting. Empty/incomplete pairs return the same empty result as a valid empty
  exclusive body and no consumed end position is exposed to callers.
- Test/doc finding: no harness or selector-local page exists. The book signature
  is malformed and neither it nor source documentation defines X/I/C/E,
  full-word aliases, errors, comments, nesting, same delimiters, adjacency,
  Unicode, or the ambiguity of an empty result. No Level C BIF exists.
- Performance baseline: isolated output is 507 noopt / 1,677 opt RXAS lines.
  Noopt has fourteen general/qpos calls; optimized output retains six. Repeated
  qpos calls rescan quote state from each cursor, so the eventual implementation
  should return segment bounds from one authoritative scanner.
- Completion summary: parked with row 112 on the extraction mode/result-boundary
  ABI and shared quote grammar. Row 114 `qstripcomment` is sole active.

### 114. `qstripcomment` — parked before implementation

- Public surface: `qstripcomment(open=.string [,close=.string], text=.string)
  -> .string` removes line or block comments outside quotes. It is Level B-only
  and depends on qpos plus qextractpair for block regions.
- Contract blockers: line-comment mode searches only CRLF, so LF/CR text is
  truncated as if no line end exists. Empty open markers leave input unchanged;
  unterminated blocks cut the remainder; block nesting and exact line-ending
  preservation are unspecified. The source header calls the function QCOMMENT
  while exporting QSTRIPCOMMENT, and its quote-safe claim inherits both parked
  grammar dependencies.
- Test/doc finding: no harness or selector-local Markdown exists and the book
  only inventories the name. There are no assertions for quoted markers,
  repeated/adjacent comments, newline variants, unterminated blocks, empty
  markers, Unicode, input preservation, or signals. No Level C BIF exists.
- Performance baseline: isolated output is 275 noopt / 706 opt RXAS lines, with
  seven/three calls and two growing-output concatenation sites. The eventual
  implementation should append preserved spans from one scan rather than call
  qpos/qextractpair and repeatedly rebuild the result.
- Completion summary: parked on a comment/newline/unclosed-block policy plus the
  shared quote/extraction grammar. Row 115 `qremoveall` is sole active.

### 115. `qremoveall` — parked before implementation

- Public surface: `qremoveall(open=.string, close=.string, text=.string
  [,mode=.string]) -> .string` removes matched regions in inclusive or
  exclusive mode. It is Level B-only and directly consumes qextractall.
- Correctness blocker: qextractall returns segment values without positions;
  qremoveall then applies CHANGESTR globally for each value. Identical content
  outside the selected pair, including quoted text, is removed too. Exclusive
  mode similarly removes every occurrence of the inner content rather than the
  one bounded region, so delimiter preservation and duplicate/empty segments
  are not reliable.
- Test/doc finding: no harness or selector-local page exists. Source prose
  claims quote-safe nested removal and full-word modes but there is no coverage
  for repeated equal text, quoted duplicates, nesting, empty bodies, adjacent
  pairs, invalid modes/delimiters, Unicode, or non-mutation. No Level C exists.
- Performance baseline: isolated output is 116 lines in both modes with one
  qextractall and one CHANGESTR call site; runtime repeatedly scans and rebuilds
  the whole result once per extracted value, producing potentially quadratic
  work in addition to the correctness problem.
- Completion summary: parked on bound-returning extraction and the shared mode/
  quote grammar. The final implementation should append preserved spans by
  position and never perform global value replacement. Row 116 `qword` is sole
  active.

### 116. `qword` — parked before implementation

- Public surface: `qword(line=.string, wanted=.int) -> .string` returns a
  quote-aware word. It is Level B-only and is the value foundation for qwords,
  qwordlength, and qwordpos.
- Contract blockers/correctness: source says quoted strings are kept intact and
  returns the delimiters, while the repository example says the returned value
  is the inner text. Text attached immediately after a closing quote becomes a
  separate word without a blank. Doubled and unmatched quotes, blank/tab-only
  separation, and invalid indices are not documented; invalid indices silently
  return empty. The lookahead `strchar` executes before its end check and probes
  offset `length` when the closing quote is the last character.
- Test/doc finding: no functional harness or selector-local page exists. The
  book only inventories the signature and the single example contradicts the
  source return shape. There is no Level C BIF.
- Performance baseline: isolated output is 424 noopt / 847 opt RXAS lines.
  It has two FNDNBLNK, one FNDBLNK, three STRPOS, and two/four STRCHAR sites;
  noopt additionally calls WORD, SUBSTR twice, and STRIP, while opt retains
  WORD/STRIP. Repeated callers rescan from the beginning.
- Completion summary: parked before edits on one shared quote-aware word-span
  contract. Row 117 `qwordlength` is sole active.

### 117. `qwordlength` — parked after typed-wrapper completion

- Public surface result: the incorrect `.string` return and exposed read-only
  input are replaced by `qwordlength(text=.string, word_number=.int) -> .int`.
  Missing words return zero; nonpositive indices signal `INVALID_ARGUMENTS`.
  There is no Level C BIF or class method.
- Algorithm result: the wrapper calls qword exactly once and measures the
  returned span once with native STRLEN. It removes the old general LENGTH
  emptiness call plus repeated STRLEN and does not copy or expose the input.
- Test/doc result: a focused harness covers multiple unquoted positions,
  missing/empty input, non-mutation, and the invalid-index signal without
  freezing the parked quote-delimiter semantics. RexxDoc and stable Level B
  Markdown define the typed wrapper and explicitly route quote behavior to the
  shared qword contract.
- Focused validation and second review: optimized and unoptimized overlays both
  report `PASS: qwordlength`, linked only with isolated qword and its reviewed
  WORD fast-path dependency. The selector is 60 RXAS lines in both modes with
  one qword call and one STRLEN; stale aggregate output is 76 lines with one
  qword call and two STRLEN sites. The aggregate binary was temporarily hidden
  only to avoid its old `.string` metadata and was restored immediately.
- Completion summary: T/P/D and the independent typed wrapper are complete.
  B/V remain parked only because the length of a quoted span must follow the
  shared qword grammar; aggregate metadata/test wiring is queued. `git diff
  --check` passes and compiler/interpreter diffs remain empty. Row 118 `qwords`
  is sole active.

### 118. `qwords` — parked before implementation

- Public surface: `qwords(text=.string) -> .int` counts quote-aware words. It is
  Level B-only and currently delegates each candidate ordinal to qword.
- Correctness/contract blocker: counting stops when qword returns empty, so the
  result depends on whether quote delimiters are retained and how an empty
  quoted word is represented. The arbitrary 99,999,999 loop ceiling returns
  zero rather than the partial count. Blank, doubled/unmatched quote, attached
  text, and Unicode behavior are all inherited but unspecified.
- Test/doc finding: no harness, RexxDoc, or selector-local page exists; the book
  only inventories the signature. No Level C BIF exists.
- Performance baseline: output is 67 RXAS lines in both modes with one qword
  call site inside a loop. Because the ith call scans from the beginning to the
  ith word, counting is O(words multiplied by text length), rather than one
  linear scan.
- Completion summary: parked on the shared qword span scanner. Resume by using
  the same authoritative token transitions to count once, eliminating the
  artificial ceiling and repeated qword calls. Row 119 `qwordindex` is sole
  active.

### 119. `qwordindex` — parked before implementation

- Public surface: `qwordindex(text=.string, wanted=.int) -> .int` returns the
  1-based start of a quote-aware word. It is Level B-only and is consumed by
  qsubword and qwordpos.
- Correctness/contract blockers: the function duplicates qword's state machine,
  end lookahead, blank definition, attached-text behavior, and silent invalid
  index. Its two initial quote searches use different starts—one reuses a
  zero-based FNDNBLNK result as a one-based STRPOS start and the other relies on
  an implicit zero—so qword and qwordindex are not guaranteed to select the
  same spans.
- Test/doc finding: no harness, RexxDoc, or selector-local page exists; the book
  only inventories the signature. No Level C BIF exists.
- Performance baseline: isolated output is 368 noopt / 472 opt RXAS lines.
  Noopt has one WORDINDEX fast-path call plus two FNDNBLNK, one FNDBLNK, three
  STRPOS, and two STRCHAR sites; optimization inlines the plain path but still
  duplicates the quote scanner rather than sharing bounds with qword.
- Completion summary: parked on the shared qword span scanner. Row 120
  `qwordpos` is sole active.

### 120. `qwordpos` — parked before implementation

- Public surface: `qwordpos(search=.string, text=.string [,start=.int]) -> .int`
  is named as the quote-aware analogue of WORDPOS. It is Level B-only.
- Correctness/contract blockers: it computes both text/search qwords counts but
  never uses the search count, so multiword phrases cannot match. It computes a
  starting character index and never uses it. Each loop instead accepts SEARCH
  as a prefix of one qword at position 1, or at position 2 to skip an opening
  quote; this is neither documented exact word-sequence equality nor a clearly
  named prefix operation. Starts below one are not validated.
- Test/doc finding: no harness, RexxDoc, selector-local page, or behavioral book
  section exists. There is no Level C BIF.
- Performance baseline: isolated output is 225 noopt / 302 opt RXAS lines with
  six call sites. It invokes qwords twice, qwordindex once, and qword once per
  candidate; because each dependency rescans from the beginning, ordinary
  search is at least quadratic even before phrase matching is repaired.
- Completion summary: parked on an exact qwordpos match contract plus the shared
  word-span scanner. Row 121 `qsubword` is sole active.

### 121. `qsubword` — parked before implementation

- Public surface: `qsubword(text=.string, word_number=.int) -> .string` returns
  the source tail starting at the selected quote-aware word. It is Level B-only
  and, unlike ordinary SUBWORD, has no optional count argument.
- Contract blocker: its only boundary comes from qwordindex, so quote delimiters,
  attached text, blanks, doubled/unmatched quotes, and invalid index handling
  are wholly inherited. A number below one silently returns empty rather than a
  signal, but changing that in this wrapper alone would leave the family
  inconsistent. The two-argument tail-only surface is not behaviorally
  documented well enough to infer a missing count parameter.
- Test/doc finding: no harness, RexxDoc, selector-local page, example, or book
  behavior section exists. There is no Level C BIF.
- Performance baseline: isolated output is 77 noopt / 286 opt RXAS lines. Noopt
  calls qwordindex and SUBSTR; optimized output retains qwordindex and inlines
  the final slice. The index dependency performs the entire scan.
- Completion summary: parked on the shared qword span scanner and explicit
  tail-versus-count contract. Row 122 `fsayfmt` is sole active.

### 122. `fsayfmt` — parked before implementation

- Public surface: `fsayfmt(template=.string) -> .string` is a Level B compile-
  time expression generator, not a runtime value formatter. The existing FSAY
  compiler exit inserts its returned source directly after `say`; no Level C
  BIF or class method exists.
- Correctness/contract blockers: width without explicit alignment is parsed but
  ignored despite book examples; unmatched `{` returns unquoted template text
  as source; unmatched `}` is literal; empty/invalid variables, widths, and
  decimals are emitted for a later compiler failure rather than signalling.
  Brace recognition is delegated to qpos, so quote-like literal text follows
  the unresolved quote grammar. Literal quote escaping and the unconditional
  removal of the first/last character rely on compiler-token representation.
- Test/doc finding: the large book page documents a runtime-looking formatting
  facility and width behavior not implemented by the expression generator.
  The sole FSAY exit test exercises one valid aligned/numeric report and checks
  only a final SUCCESS marker, with no direct returned-expression/error vectors.
- Performance baseline: isolated output is 714 noopt / 3,790 opt RXAS lines,
  with 18/11 calls and 36/62 concatenation sites. General qpos, string, word,
  and formatting-source helpers dominate; optimization expands helper bodies
  substantially without removing the two qpos scans per placeholder.
- Focused validation: the normal CTest name unexpectedly selected its build
  fixture and attempted the deferred aggregate rebuild; that stopped after two
  selector steps on the already-known stale ABBREV metadata conflict, before
  aggregate linking. It was not retried. Running the already-built noopt/opt
  FSAY artifacts directly with the existing runtime suffix produced all five
  expected report rows and `SUCCESS` in both modes.
- Completion summary: parked on an explicit placeholder/expression/diagnostic
  contract plus shared quote grammar, with compiler changes excluded. `git diff
  --check` passes and compiler/interpreter source diffs remain empty. All 122
  selector rows have now been processed; the one deferred aggregate wiring/build
  and final validation/benchmark phase is active.

For each started row, add a short subsection here before editing:

```text
### <number>. <selector> — in progress

- Source/public surface:
- Dependencies and known consumers:
- Current Level B signature/error/algorithm findings:
- Level C contract and current implementation/lowering status:
- Existing Level B tests:
- Existing Level C tests:
- Existing Level B docs:
- Existing Level C docs/spec clauses:
- Approved implementation notes or decisions:
- Focused validation commands/results:
- Performance/RXAS review evidence:
- Completion summary:
```

When complete, keep the evidence subsection, check every applicable table
gate, set the row to `done`, and identify the next row as the sole active item.
