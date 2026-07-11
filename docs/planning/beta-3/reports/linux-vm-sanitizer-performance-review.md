# Linux VM Sanitizer And Performance Review

Date: 2026-07-11

## Scope

This review covers the Release 1 VM infrastructure on the Linux ARM64 VM,
including RXAS instruction execution, fixed signed 64-bit `.int`, binary
memory, jump tables and corruption handling, optimizer metadata, interface
lookup, JSON, references, and compiler-generated performance workloads.

The source baseline was `654d42a8312ac2ebd28ffd384be80ed71c67e43a`
(`develop`, beta 3 WIP). The clean sibling `DSL-Syntax-Highlighter` checkout
had to be fast-forwarded from `694801a` to `a64c993` to provide the byte-span
API required by current CREXX. A local `dev-snapshot` tag collision prevented
an all-tags refresh, but `develop` itself matched `origin/develop` exactly.

## Host And Toolchain

| Item | Value |
| --- | --- |
| OS | Ubuntu 26.04 LTS |
| Kernel | Linux 7.0.0-27-generic |
| Architecture | AArch64, Apple virtual CPU |
| CPUs | 6 |
| Memory | 9.2 GiB RAM, 4.0 GiB swap |
| Compiler | GCC/G++ 15.2.0 |
| CMake | 4.2.3 |
| Ninja | 1.13.2 |
| perf | 7.0.6, blocked by `kernel.perf_event_paranoid=4` |

Build parallelism was limited to four for Debug/ASan and three for optimized
Release builds. Builds and CTest never overlapped.

## Validation

Normal Debug completed the initial full build and passed `1580/1580` CTests in
222.87 seconds. After all fixes, the rebuilt Debug tree passed `1580/1580` in
266.97 seconds. The final full ASan/LSan run completed a full build and passed
`1580/1580` CTests in 280.23 seconds with build-time and test-time leak
detection enabled.

Sanitizer coverage used `tools/asan-run.sh` exclusively:

| Surface | Result | Log directory |
| --- | ---: | --- |
| Requested VM/RXAS focus | 213/213 | `cmake-build-debugasan/asan-logs/20260711-151413-ctest` |
| Documented focused LSan | 28/28 | `cmake-build-debugasan/asan-logs/20260711-151454-focused-lsan` |
| Final full ASan/LSan | 1580/1580 | `cmake-build-debugasan/asan-logs/20260711-164000-full` |
| Focused interface Debug | 82/82 | `cmake-build-debug/asan-logs/20260711-161222-ctest` |
| Focused interface ASan/LSan | 82/82 | `cmake-build-debugasan/asan-logs/20260711-161606-ctest` |

UBSan was not added. CREXX documents and validates ASan/LSan through the
runner, but does not currently document a supported broad UBSan workflow.

### Findings And Classification

| Finding | Classification | Resolution |
| --- | --- | --- |
| `readkey()` leaked its heap read buffer after `SETSTRING` copied the result | Real CREXX defect | Free the plugin-owned buffer after copying it into the VM return value; focused Debug and ASan/LSan pass |
| Optimized and unoptimized KeyDB tests shared `testKeyDB.db` and raced under parallel CTest | Test-harness defect | Pass the CTest name to the program and use a mode-specific database filename; the pair now passes concurrently at `--parallel 2` |
| Current CREXX required a newer clean sibling DSLSH checkout | Dependency/environment issue | Fast-forward the sibling checkout; no CREXX compatibility workaround added |
| Linux hardware/software perf events unavailable | Environment limitation | Record `perf_event_paranoid=4`; use gprof plus in-process benchmark counters |
| Third-party sanitizer findings | None | No suppressions added |

The focused integer, optimizer metadata, binary bounds, jump-table corruption,
RXAS parser, reference lifetime, JSON, and interface tests were clean. No
signed-overflow, stale opcode-effect metadata, RXBIN corruption, or Linux-only
standalone link defect was reproduced in the covered surface.

## Benchmark Method

The wall-clock tree was a normal `Release` build with `-O3 -DNDEBUG`. A
separate `cmake-build-profile` tree used `-O3 -g -fno-omit-frame-pointer -pg`.
Optimized benchmark modules were linked once before sampling, so the VM results
exclude compilation, assembly, and linking. Every reported runtime number is
the median of seven serial samples after one warmup.

Representative standalone tool medians were:

| Workload/mode | rxc | rxas | rxlink |
| --- | ---: | ---: | ---: |
| Binary noopt | 69.682 ms | 2.825 ms | n/a |
| Binary opt | 70.045 ms | 7.256 ms | 4.981 ms |
| Jump noopt | 73.473 ms | 6.915 ms | n/a |
| Jump opt | 75.742 ms | 128.204 ms | 5.175 ms |

The jump opt assembler cost reflects packed jump-table construction and ACPH
policy work; it is not included in VM execution time.

## Runtime Results

All values below are in-process microseconds. `noopt` is unlinked compiler and
assembler output loaded with its runtime modules. `opt` is the pre-linked
optimized image.

| Workload and hot section | rxvm noopt | rxvm opt | rxbvm opt |
| --- | ---: | ---: | ---: |
| Binary `.u32` write, 200k cells | 2,357 | 2,019 | 1,530 |
| Binary `.u32` read, 200k cells | 2,957 | 2,532 | 1,837 |
| Binary `.i64` write, 200k cells | 2,177 | 2,149 | 1,499 |
| Binary `.i64` read, 200k cells | 2,892 | 2,563 | 1,803 |
| JSON validate | 19,141 | 19,335 | 16,365 |
| JSON count | 37,851 | 38,295 | 32,832 |
| Reference dynamic backing iterator | 1,387 | 1,397 | 1,143 |
| Classlib live iterator | 4,006 | 4,060 | 3,427 |
| StringTreeMap insert | 9,032 | 8,863 | 7,532 |
| Tinyexpr lex | 52,188 | 51,570 | 44,799 |
| Tinyexpr evaluate | 145,510 | 146,298 | 114,338 |

The current non-threaded `rxbvm` is consistently faster than computed-goto
`rxvm` on this Linux AArch64 VM for these workloads, usually by 10% to 30%.
The long string jump benchmark is approximately tied. This result is strong
enough to justify a perf-enabled cross-platform dispatch review, but not to
change the default VM from one virtualized ARM64 host.

### Jump Dispatch

The eight-case runs use actual packed jump tables. Times are `table / branch`:

| Shape, 1,000,000 iterations | rxvm noopt | rxvm opt | rxbvm opt |
| --- | ---: | ---: | ---: |
| Integer, uniform | 46,673 / 56,611 | 45,559 / 57,715 | 28,939 / 41,220 |
| Padded string, last hit | 22,694 / 147,245 | 21,030 / 147,578 | 16,632 / 147,642 |
| Numeric string, last hit | 38,341 / 250,733 | 35,845 / 245,303 | 28,304 / 241,849 |

This confirms the checked-in profitability policy: eight-case integer tables
win for uniform traffic, while padded and numeric text tables have a much
larger benefit. The default three-case integer fixture remains slower than its
comparison ladder and should not be used to weaken the integer threshold.

### Interface Fast Path

Profiling identified repeated type normalization and allocation in factory
signature matching. A safe exact-metadata-spelling fast path was added before
normalization. Seven-run medians before and after were:

| Path | Before | After | Change |
| --- | ---: | ---: | ---: |
| rxvm noopt factory | 89,664 | 86,798 | 3.2% faster |
| rxbvm noopt factory | 90,307 | 87,970 | 2.6% faster |
| rxvm opt factory | 5,542 | 5,560 | flat |
| rxbvm opt factory | 5,448 | 5,483 | flat |

The remaining optimized factory cost is dominated by semantically compatible
but differently qualified contract spellings. Caching those compatibility
results must define registry rebuild and late-load invalidation and is therefore
a design candidate, not part of this narrow fast path.

## Profile And RXAS Review

`perf` could not run under the VM policy. Gprof sampling therefore provides
function attribution, while benchmark sections provide instruction-family
timing. Because the interpreter handlers are labels inside monolithic `run()`,
gprof cannot honestly split individual opcodes into functions.

Dominant self/cumulative samples were:

| Profile | Dominant function | Self and cumulative share |
| --- | --- | ---: |
| Binary rxvm / rxbvm | `run` | 100% / 100% |
| Jump rxvm / rxbvm | `run` | 87.4% / 84.1% |
| JSON rxvm / rxbvm | `run` | 98.4% / 97.6% |
| Tinyexpr rxvm / rxbvm | `run` | 93.9% / 93.2% |
| Interface rxvm / rxbvm | `runtime_signature_type_assignable` | 50.0% / 66.7% |
| Tree-map rxvm / rxbvm | `copy_value` | 23.6% / 30.5% |

Generated and disassembled RXAS confirm:

- Binary loops use direct `bsetu8`, `bgetu8`, `bsetu32`, `bgetu32`,
  `bseti64`, and `bgeti64`. There are no separate loop bounds-check opcodes,
  allocations, or binary copies; each VM handler performs its required range
  check and fixed-width `memcpy` fast path.
- Jump handlers call packed lookup directly and retain malformed-table and
  out-of-range target validation. No safety check should be removed from these
  results.
- The RXAS optimizer reduces instruction counts without losing direct binary
  or jump opcodes. For example, binary opt falls from 305 generated
  instructions to 263 final instructions.
- Compiler inlining can increase register and copy pressure. The reference
  workload changes from 31 to 62 main locals, 738 to 817 final instructions,
  7 to 38 full copies, 20 to 64 integer copies, and 61 to 110 `linkattr1`
  operations. Its optimized timings do not improve. This needs an inliner cost
  model, not another local peephole rule.
- Other optimized main-local changes are smaller but visible: binary 34 to 41,
  JSON 22 to 33, and classlib iterator 21 to 36. Tinyexpr improves from 21 to
  19.

## Opportunities

### Safe Release 1 Fixes With Measured Benefit

- Land the KeyDB native return-buffer ownership fix.
- Keep mode-specific KeyDB test files so parallel tests remain race-free.
- Keep the exact-spelling interface type fast path; it is semantics-preserving
  and improves the unlinked factory stress path by 2.6% to 3.2%.
- Preserve the current fixed-width binary `memcpy` handlers and jump-table
  validation. They are already direct fast paths.

### Release 1 Candidates Requiring Design Approval

- Add a cache for normalized interface assignability/signature comparisons,
  with explicit invalidation on registry rebuild and late module loading.
- Decide whether AArch64 builds should prefer `rxbvm`, or whether dispatch
  selection should be configurable. Repeat with permitted hardware counters
  and at least one native Linux ARM64 host before changing defaults.
- Add an inliner growth/copy budget for reference-heavy methods. The semantic
  and diagnostics effects need compiler-owner approval.

### Post-Release 1 Or CFG/Data-Flow Work

- Use CFG/reaching-definitions/liveness analysis before recognizing general
  branch ladders or eliminating link/copy/unlink sequences across calls.
- Consider proven range-check hoisting only when dominance and mutation facts
  show the binary length and offsets are stable. Do not remove checks based on
  this synthetic benchmark.
- Add common-subexpression handling for repeated binary base-offset arithmetic
  in real parsers and packed data structures.

## Remaining Risks

- Hardware counter and sampled line-level perf evidence is missing because of
  the VM security policy.
- Results cover one virtualized Linux AArch64 host; x86-64 and native ARM64
  validation remain necessary before dispatch-default decisions.
- Gprof sees the interpreter loop as one function, so opcode attribution relies
  on isolated fixture sections and RXAS inspection.
- Broad UBSan remains unvalidated until CREXX documents a supported runner
  configuration.
