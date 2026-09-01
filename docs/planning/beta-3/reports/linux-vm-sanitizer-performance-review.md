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

### Repeatable Cross-Platform Protocol

Dispatch comparisons must run the same `.rxbin` under both interpreters. Do
not compare independently rebuilt `rxvm` and `rxbvm` benchmark modules, and do
not include compilation, assembly, or linking inside a timed sample. Record the
source commit, compiler version, architecture, power mode, CPU count, and
Release flags. Run on AC power where applicable, stop unrelated builds and
CTests, perform one unrecorded warmup, then retain seven serial samples and
report their median. Preserve the seven raw outputs for later dispersion checks.

Configure and build the common Release surface from the repository root:

```sh
build=cmake-build-release
cmake -S . -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$build" --parallel 4 --target \
  rxvm rxbvm rxlink library classlib rxfnsl \
  performance_binary_fastpath performance_rxjson_parser \
  performance_reference_iterator performance_classlib_iterator \
  performance_classlib_treemap performance_tinyexpr_dispatch \
  performance_jump_dispatch performance_runtime_lookup
```

Adjust build parallelism for the host's memory. Builds and benchmark runs must
not overlap. Pre-link each optimized program exactly once. This binary example
shows the required shape; classlib fixtures additionally link `classlib.rxbin`,
and tinyexpr additionally links `rxfnsl.rxbin`:

```sh
perfdir="$build/tests/performance"
linked="$perfdir/manual-linked"
mkdir -p "$linked"
"$build/bin/rxlink" -s -o "$linked/binary_fastpath_compare_opt" \
  "$perfdir/binary_fastpath_compare_opt.rxbin" \
  "$build/bin/library.rxbin"
```

Run the unlinked noopt image with its runtime modules. Run the one pre-linked
opt image under both VMs. Sample zero is the discarded warmup:

```sh
for sample in 0 1 2 3 4 5 6 7; do
  "$build/bin/rxvm" "$perfdir/binary_fastpath_compare_noopt.rxbin" \
    "$build/bin/library.rxbin"
done
for sample in 0 1 2 3 4 5 6 7; do
  "$build/bin/rxvm" "$linked/binary_fastpath_compare_opt.rxbin"
done
for sample in 0 1 2 3 4 5 6 7; do
  "$build/bin/rxbvm" "$linked/binary_fastpath_compare_opt.rxbin"
done
```

On Windows PowerShell the equivalent execution form is:

```powershell
$Build = "cmake-build-release"
$Perf = Join-Path $Build "tests/performance"
$Linked = Join-Path $Perf "manual-linked"
New-Item -ItemType Directory -Force $Linked | Out-Null
& "$Build/bin/rxlink.exe" -s -o "$Linked/binary_fastpath_compare_opt" `
  "$Perf/binary_fastpath_compare_opt.rxbin" "$Build/bin/library.rxbin"
0..7 | ForEach-Object {
  & "$Build/bin/rxvm.exe" "$Linked/binary_fastpath_compare_opt.rxbin"
}
0..7 | ForEach-Object {
  & "$Build/bin/rxbvm.exe" "$Linked/binary_fastpath_compare_opt.rxbin"
}
```

Use this workload matrix without changing defaults for the first comparison:

| Workload | Program base | Extra linked/runtime module | Arguments after `-a` |
| --- | --- | --- | --- |
| Binary | `binary_fastpath_compare` | `library.rxbin` | none: 200,000 cells |
| JSON | `rxjson_parser_compare` | `library.rxbin` | none: 60 rows, 30 passes |
| Reference | `reference_iterator_compare_rxvm` | `library.rxbin` | none: 12,000 iterations |
| Classlib iterator | `stringarraylist_iterator_compare_rxvm` | `library.rxbin`, `classlib.rxbin` | none: 4,000 rounds |
| Tree map | `stringtreemap_insert_compare` | `library.rxbin`, `classlib.rxbin` | none: 2,500 inserts |
| Tinyexpr | `tinyexpr_dispatch_compare` | `library.rxbin`, `rxfnsl.rxbin` | none: 20,000 iterations |
| Integer jump | `jump_dispatch_compare` | `library.rxbin` | `int 8 3 1000000` |
| Padded-string jump | `jump_dispatch_compare` | `library.rxbin` | `string 8 0 1000000 key07` |
| Numeric-string jump | `jump_dispatch_compare` | `library.rxbin` | `numeric 8 0 1000000 8` |
| Interface lookup | `runtime_interface_lookup_compare` | `library.rxbin` | none: 50,000 methods, 5,000 factories |

For native Ubuntu x86-64, repeat the full normal Debug and ASan/LSan exercise,
not only the timing subset. Use `tools/asan-run.sh --phase full --test-jobs N`
as documented in `docs/ai-context/CREXX_ASAN_TESTING.md`. Then configure a
symbolized Release profiling tree without changing optimization:

```sh
cmake -S . -B cmake-build-perf -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS="-g -fno-omit-frame-pointer"
```

Check `perf list` and `sysctl kernel.perf_event_paranoid` before profiling.
After one unrecorded warmup, compare the same linked image with both runners:

```sh
perf stat -r 7 \
  -e cycles,instructions,branches,branch-misses,cache-references,cache-misses \
  -- cmake-build-perf/bin/rxvm MANUAL_LINKED_IMAGE.rxbin
perf stat -r 7 \
  -e cycles,instructions,branches,branch-misses,cache-references,cache-misses \
  -- cmake-build-perf/bin/rxbvm MANUAL_LINKED_IMAGE.rxbin
perf record -g --call-graph dwarf -- \
  cmake-build-perf/bin/rxvm MANUAL_LINKED_IMAGE.rxbin
perf report --stdio
```

Use `taskset` and a performance CPU governor only when the host supports them,
and record that choice. For hardware-counter runs, multiply fixture iterations
by ten so startup and module loading are negligible, but first verify that the
default-size in-process ratio has the same direction. Events such as
`L1-icache-load-misses` and `iTLB-load-misses` should be added only when listed
on that CPU. Absolute times are not compared between machines; the primary
cross-platform result is the paired `rxbvm / rxvm` ratio, with branch misses,
instructions, cycles, and instruction-cache evidence used to explain it.

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

### Native macOS ARM64 Repetition

The runtime comparison was repeated at
`e5c912b93f70ca6c3dc7e03d889379352e5b3525` on native macOS 26.5.1, Apple M5
ARM64 with 10 logical CPUs and 24 GiB RAM, using Apple clang 21.0.0 and
`-O3 -DNDEBUG`. The machine was on AC power. The same pre-linked optimized
image was run under both VMs, with one warmup and seven serial samples.

| Workload and hot section | rxvm noopt | rxvm opt | rxbvm opt |
| --- | ---: | ---: | ---: |
| Binary `.u32` write | 1,317 | 1,253 | 1,037 |
| Binary `.u32` read | 1,581 | 1,517 | 1,299 |
| Binary `.i64` write | 1,322 | 1,278 | 1,027 |
| Binary `.i64` read | 1,605 | 1,562 | 1,296 |
| JSON validate | 16,053 | 16,046 | 15,394 |
| JSON count | 32,694 | 32,625 | 31,050 |
| Reference dynamic backing iterator | 913 | 908 | 808 |
| Classlib live iterator | 2,953 | 3,064 | 2,833 |
| StringTreeMap insert | 6,059 | 6,058 | 5,488 |
| Tinyexpr lex | 38,619 | 38,318 | 37,396 |
| Tinyexpr evaluate | 103,429 | 101,827 | 93,703 |

The native host reproduces the Linux VM's direction: `rxbvm` is 2.4% to 19.6%
faster in this matrix. The smaller and more workload-dependent spread shows
that UTM and/or GCC may amplify the Linux ARM64 result, but virtualization
cannot be its sole cause. Numeric-string jump and optimized interface lookup
were approximately tied between modes.

### Computed-Goto Regression Investigation

Source history identifies a credible regression boundary. Commit `720d3253c`
on 2026-04-22 separated runtime dispatch state from serialized instructions.
Before that commit, each threaded instruction slot held its handler pointer and
sequential dispatch loaded `(next_pc)->impl_address`. The current path stores
handlers in `module->prepared_dispatch` and, on every instruction, evaluates
`next_pc - current_module->segment.binary` before indexing the second array.

A disposable macOS worktree cached the current module's binary base and
dispatch pointer in `run()` locals whenever the module changed. No serialized
format or instruction semantics changed. Median improvements were 15.4% to
16.0% for fixed-width binary reads/writes, 2.5% to 3.6% for JSON, 2.7% to 3.4%
for Tinyexpr, 9.2% for integer jump dispatch, and 5.1% for padded-string jump
dispatch. Numeric-string jump was 1.2% slower. The binary medians then nearly
matched `rxbvm`, strongly implicating the extra hot-path module-field loads.

Removing the pending-interrupt check in a separate measurement-only build
improved selected sections by approximately 1% to 5%. That experiment changes
semantics and is not a candidate fix. The check is present in both VM modes and
therefore does not explain the main reversal, although its code-generation and
layout cost should remain visible in hardware-counter analysis.

The computed-goto `run()` body was 423,732 text bytes on this build versus
408,508 for switch-dispatch `rxbvm`, 15,224 bytes or 3.6% larger. Compiler code
layout and instruction-cache pressure therefore remain plausible secondary
causes. Apple clang also merged the normal handlers onto one main indirect
dispatch site, removing the classic per-handler predictor advantage.

A measurement-only unique compiler barrier expanded the computed dispatches to
729 indirect sites and grew `run()` from 422,216 bytes with cached pointers to
487,164 bytes. It improved Tinyexpr and string/numeric jump cases but regressed
binary, JSON, and integer jump cases relative to pointer caching alone. Global
dispatch-site replication is therefore not a current fix candidate.

The extended review found that module-pointer caching alone is too narrow a
production design. A coherent active-frame cache, updated atomically whenever
the frame changes, benefits both VM modes by caching the current module,
execution base, binary space, constant pool, and local-register array. Combined
with opcode-indexed computed dispatch it improved the current macOS `rxvm`
baseline by about 2% to 25% in six measured hot sections and was effectively
tied in the seventh. JSON, Tinyexpr, and integer jump improved by about 8% to
11%. It was close to or faster than the original `rxbvm` in six of seven
sections. The same coherent frame cache also improved `rxbvm`; applying
individual cached fields without the common activation contract produced
inconsistent results and is not recommended.

An in-place handler-pointer experiment restored the original one-load dispatch
shape and provided a measurement upper bound, but it overwrote canonical
opcodes and therefore breaks reflection. The production design is instead a
separate runtime instruction image for `rxvm`: preserve the canonical RXBIN
image for reflection and serialization, copy each instruction and its operands
into a process-local image, and replace only instruction cells in that copy
with handler pointers. This uses approximately the same memory as the current
canonical image plus full pointer side table while removing the hot index into
the second array. Opcode-indexed dispatch is the measured lower-risk fallback
if runtime-image validation exposes unacceptable Release 1 risk.

Homebrew GCC 16 was also tested on native macOS ARM64 with TLS disabled because
the default Apple blocks TLS backend is clang-specific. Unmodified GCC was
slower than Apple clang, and its `run()` body was approximately 1.6 MiB, but
GCC retained many per-handler indirect branches. With coherent frame state and
the in-place upper-bound dispatch, it improved its own baseline by about 12% to
40% and became competitive in branch-heavy cases. This supports testing GCC
after the architecture fix; it does not support a blanket compiler switch.

Implementation proceeded through that semantics-preserving simplification of
the dispatch macros. The `CALC_DISPATCH`/`CALC_DISPATCH_MANUAL` pairing and
scattered raw target assignments are replaced by an intent-based surface for
frame activation, sequential advance, indexed branches, existing
call/return/resume targets, common interrupt-aware dispatch, and execution
pointer/index conversion. Early next-handler loading remains. Compile-time
no-op extension hooks cover instruction begin/retire, frame changes, interrupt
paths, and VM entry/exit without a runtime callback or condition in ordinary
builds. A coherent active-frame cache and the separate process-local runtime
instruction image then completed the production path. The detailed handoff,
implemented measurements, and native Intel experiment are recorded in
`docs/planning/beta-3/notes/vm-dispatch-performance-investigation.md`.

### Dispatch Refactor Validation (2026-07-12)

The implementation passed the full macOS ARM64 Debug and Release builds and
all 1,584 CTests in each tree at CTest parallel 30. Focused runtime-image ASan
coverage passed 24/24 reflection, corruption, semantics, signal, breakpoint,
and late-load tests. The initial full ASan+UBSan tree completed 1,581/1,584
tests. Its three output-comparison failures exposed two pre-existing
undefined-behaviour defects: signed overflow in `compiler/rxcp_opt.c` for the
optimized int64 contract fixture, and signed `1 << 31` in
`interpreter/rxvmintp.c` reached by both multi-tail-stem variants. No
AddressSanitizer memory error was reported.

The same-day follow-up replaced subtraction-based integer ordering with direct
relational comparison and centralized validated VM signal-mask construction.
Valid signals 1 through 31 now occupy only bits 0 through 30, and the interrupt
scan does not inspect the `RXSIGNAL_MAX` sentinel bit. Strengthened int64,
signal-mask, ignored-signal, breakpoint, instrumentation, and the three former
failure cases pass 19/19 in both normal Debug and ASan+UBSan focused runs; the
corresponding optimized Release surface passes 15/15. A subsequent full Debug
sweep passed 1,588/1,589 in parallel, with the unrelated syntax-highlighting
parser timeout passing immediately when rerun serially. Apple's sanitizer
runtime still rejects `detect_leaks=1`, so LSan is unsupported on this host
rather than reported as passing.

The final Apple clang matrix uses the exact protocol above and is tabulated in
the investigation note. The seven dispatch headline sections improve the
captured `rxvm` baseline by 1.3% to 28.2%, with a 10.7% median, and the final
`rxvm / rxbvm` median ratio is 0.999. The result is close to the earlier 11.8%
median runtime-image projection while continuing to preserve canonical RXBIN
reflection and serialization.

The final Homebrew GCC 16.1.0 matrix used Release `-O3 -DNDEBUG` with TLS off
and the identical prelinked images. GCC `rxvm` won all seven headline sections
with a 0.651 median `rxvm / rxbvm` ratio. Its improvement against the historical
GCC `rxvm` baseline had a 16.0% median, below the measurement-only 26.2% upper-
bound projection. The GCC `run()` bodies remain large at 1,589,344 bytes for
`rxvm` and 1,569,856 bytes for `rxbvm`, versus Apple clang's 409,048 and 405,764
bytes. These results reinforce that compiler code generation changes the
dispatch balance; they do not justify a default compiler or VM policy on one
ARM64 host.

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
- Keep the implemented dispatch macro contract, coherent active-frame state,
  and separate `rxvm` runtime instruction image without weakening interrupt,
  metadata, corruption, reflection, or late-load behavior. Opcode-indexed
  dispatch remains the fallback. Native Intel counters and full cross-platform
  validation remain required.

### Release 1 Candidates Requiring Design Approval

- Add a cache for normalized interface assignability/signature comparisons,
  with explicit invalidation on registry rebuild and late module loading.
- Decide the documented/default dispatch policy only after the computed-goto
  regression candidate is tested on native Linux x86-64 and Windows x86-64.
  Do not infer a CPU-family policy from the two ARM64 results.
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
  the Linux ARM64 VM security policy.
- Results cover virtualized Linux AArch64 and native macOS ARM64. Native Linux
  and Windows x86-64 validation remains necessary before dispatch-default or
  compiler-flag decisions.
- The macro cleanup, coherent frame cache, and runtime instruction image are
  implemented and locally validated, including reflection, signal,
  dynamic-load, focused sanitizer, and full Debug/Release coverage. Linux
  x86-64, Windows x86-64, and the cross-platform pipeline remain pending
  because the local commits were intentionally not pushed.
- The two UB findings from the initial full macOS ASan+UBSan run are fixed and
  their focused normal/sanitizer regression surfaces pass. LSan cannot run
  with Apple's current sanitizer runtime.
- GCC results were obtained with TLS disabled and do not establish that a full
  macOS GCC build is supported. Native x86-64 hardware counters remain needed
  before any compiler or dispatch-default policy change.
- Gprof sees the interpreter loop as one function, so opcode attribution relies
  on isolated fixture sections and RXAS inspection.
- Broad UBSan remains unvalidated until CREXX documents a supported runner
  configuration.
