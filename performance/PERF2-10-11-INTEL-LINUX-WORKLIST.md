# PERF2-10/11 Intel Linux baseline and attribution worklist

Status: **initial Linux baseline and attribution complete; sufficiency gate met,
no tuning candidate selected**

Started: 2026-07-28

Purpose: execute the approved Intel x86-64 Linux baseline, correctness,
sanitizer and native-attribution portion of the successor hardware handover.
This activity validates the accepted Apple product on the principal Linux
tuning host. It does not authorize a production optimization, the separate
`PERF2-06-D01` parent-versus-VM-C1b experiment, an LTO/PGO/layout selection,
a default VM decision, or a cross-platform/final-superiority claim.

## Frozen source and execution root

- Main checkout at start: clean `develop` and `origin/develop` at
  `d5f0827ca2708eae9d9be182c6d0d53bd6229b74`.
- Accepted production source: `39d3c652e27860222f5d5ed43af71147589b1121`;
  later product-affecting change is the test-only regex correction at
  `057592681c0c68e90f436bf02d8c5a116111952a`.
- Detached execution source:
  `/tmp/crexx-perf2-linux-x64.d5f0827ca/source`. It started clean at
  `d5f0827ca2708eae9d9be182c6d0d53bd6229b74` and now carries only the
  sanitizer corrections listed below. The retained source patch and hashes
  are part of this evidence bundle.
- Execution root: `/tmp/crexx-perf2-linux-x64.d5f0827ca`.
- Evidence root:
  `performance/evidence/2026-07-28-perf2-10-11-intel-linux/`.
- Linux evidence closure: 3,024/3,024 regular files verified; recursive
  `checksums.sha256` SHA-256
  `5efb48b290a3c1ae48f6b21c41e9ea26308ba2d5ad8017feeb0e03a480b2ac1c`.
- Mac PERF2-09 closure checksum file SHA-256:
  `5591ae327f246e2d0918a0ebd760fb5dc475302ea06662bc5b73b6903e4af677`.
- Mac PERF2-06/07 closeout checksum file SHA-256:
  `ec403e407012df01e9539184a263d4e3e24efc97fca24b8fd7875c4839731581`.

The three lifecycle RXBIN files named by those checksum sets were initially
excluded by the repository-wide `*.rxbin` ignore rule. They were reproduced
from their retained RXAS with the lifecycle runner's exact working-directory
convention, matched retained SHA-256
`19216070c4921764404dc48d2c018ad54c3e67e401cd27cd5084f501e219b2df`,
and both bundles then verified 107/107 files.

## Host and toolchain freeze

- Host: `adrian-linux`, native Intel x86-64.
- CPU: 11th Gen Intel Core i5-1135G7, four cores/eight threads, one socket.
- Memory: 18 GiB RAM and 8 GiB swap.
- OS/kernel: Ubuntu 26.04 LTS, Linux `7.0.0-27-generic`.
- Power: AC online, battery full; `intel_pstate` active, EPP `performance`,
  turbo enabled and SMT enabled.
- GCC/G++: 15.2.0.
- Clang/Clang++: 21.1.8.
- CMake: 4.2.3; Ninja: 1.13.2.
- `perf`: 7.0.12; persistent
  `/etc/sysctl.d/99-crexx-performance.conf` sets
  `kernel.perf_event_paranoid=-1`.
- `kernel.kptr_restrict=1` and `kernel.nmi_watchdog=1` remain unchanged.
- Generic hardware counters, Intel raw front-end/L1I/iTLB/indirect-branch
  events and user-space DWARF `perf record` were proved before capture.
- Valgrind 3.26.0, `bpftrace`, `trace-cmd`, `turbostat`, GDB and LLVM 21
  profiling/analysis tools are available.

Formal timing runs are serial on AC with no overlapping build, CTest,
sanitizer, profile or counter work. Capture pre/post load, power, frequency
policy, thermal state and memory. Keep the formal runtime substrate unpinned;
use fixed affinity only for separately labelled native-counter diagnostics
when it reduces event noise.

## Comparator freeze

All comparators are user-local and passed correctness smokes.

| Runtime | Identity and path | Frozen identity |
| --- | --- | --- |
| ooRexx | 5.1.0 r12973, `/home/adrian/.local/opt/oorexx/5.1.0-12973/bin/rexx` | executable SHA-256 `239f1a2cb0da710ada1ad4b94d4334ed3a7e5c1307f0d8adf8dc23b4836cf1de` |
| Regina | 3.9.7 MT, `/home/adrian/.local/opt/regina/3.9.7/bin/regina` | executable SHA-256 `c868826524449db1559020b20928020ac8778c57e26f761fbb5d846a710b918a`; official source archive SHA-256 `f13701ebd542e74d0fc83b2a7876a812b07d21e43400275ed65b1ac860204bd4` |
| NetRexx | 5.10-GA build 18-20260320-1410, `/home/adrian/.local/opt/netrexx/5.10-GA` | compiler JAR `3d57cbd486f90d6db4e4a5ff45defead9442e9d6fb63710a89b2befa7ceb9095`; runtime JAR `3285e5daa1128474278babdcb6896df6ad5c7ba3a4d9371ed0a19ed3b854af7c`, both exact Mac matches |
| Java | Temurin OpenJDK 26.0.1+8, `/home/adrian/.local/opt/temurin/26.0.1+8/bin/java` | archive publisher checksum `8e512f13e575a43655fc92319436c94890c137b9035cc6bd6f9cf24239704d3a` |

Regina remains RexxCPS-only. Canonical NetRexx common cells retain
`options nobinary decimal`, NetRexx `Rexx` numeric state and the default
HotSpot JIT. The Linux lifecycle `.class` is byte-identical to the retained
Mac artifact; generated Java differs only in non-runtime source provenance.

## Build matrix

Every tree is independent and records configure argv, cache, compiler
identity, build log, artifact hashes and actual `rxvmintp.c` command.

| Tree | Compiler/configuration | Authority |
| --- | --- | --- |
| `gcc-release` | GCC Release, profiling off | ordinary GCC timing and correctness |
| `clang-release` | Clang Release, profiling off | ordinary Clang timing and correctness |
| `gcc-debug` | GCC Debug, profiling off | normal sanitizer-control correctness |
| `clang-debug` | Clang Debug, profiling off | compiler-matrix correctness |
| `gcc-profile` | GCC Release, `CREXX_VM_PROFILING=ON` | schema-5 counts/timing diagnostics only |
| `clang-profile` | Clang Release, `CREXX_VM_PROFILING=ON` | compiler-sensitive diagnostic control |
| `gcc-debugasan` | GCC ASAN build | supported full ASan/LSan through `tools/asan-run.sh` |
| `clang-debugubsan` | Clang UBSAN build | exploratory undefined-behavior inventory |

No profiling or sanitizer binary enters ordinary timing. Native `perf stat`
uses the exact ordinary profiling-off Release binary. Symbolized sampling may
use a separate optimization-preserving diagnostic build, and every conclusion
must be confirmed against ordinary Release behavior.

### Build conservation on this host

Treat every completed Linux build tree as an expensive, immutable test asset.
Do not touch its source inputs, reconfigure it or issue a build command merely
to discover whether a target is current.

- Before an intentional build, use Ninja's dry-run/query facilities and record
  the planned edges. If an intended test or analysis command would rebuild an
  input, stop and classify why before allowing it.
- Run all applicable focused and broad tests, schema/profile extraction,
  binary inspection and native sampling against the existing artifact before
  considering another build.
- Keep experimental code snippets and mechanism ceilings outside the detached
  production source and its build trees. Prefer disposable direct-compiled
  harnesses or narrowly copied units under a separate temporary root.
- Select and batch related production changes before paying for one deliberate
  rebuild. Do not alternate a small edit with a full GCC/Clang rebuild when a
  disposable harness can reject or rank the alternatives first.
- Preserve build directories and product hashes until the Linux report is
  checksum-closed. A candidate tree is new evidence; it must not overwrite a
  qualified baseline tree.
- Use one build job for GCC profiling. The first two-job profile attempt caused
  a kernel OOM kill: one `cc1` had reached 11.3 GiB RSS while both
  `rxvmintp.c` variants were active. The successful one-job retry took
  1:01:49 and its process tree peaked at 15.95 GiB RSS. Keep Clang profiling
  serial too unless a later dry-run proves no `rxvmintp.c` compilation is
  involved.

## Correctness and sanitizer sequence

1. Configure all ordinary GCC/Clang trees from the detached source.
2. Build the exact ten-test handover surface and run it under both Release
   products.
3. Complete full profiling-off Release CTest under GCC and Clang.
4. Complete full normal Debug CTest under GCC and Clang.
5. Run full GCC ASan/LSan only through `tools/asan-run.sh`, with build-time
   and test-time leak detection enabled.
6. Run the exploratory Clang UBSan build and full CTest separately. A UBSan
   failure is a correctness finding, not a performance result; do not add a
   maintained workflow without documenting it.
7. Stop on the first real correctness/sanitizer failure, classify it and
   report before timing.

The first sanitizer inventory found four correctness defects rather than
performance candidates:

- GCC ASan/LSan found a 10,000-byte successful-return leak in
  `rxtcp.tcpreceive`; the plugin now copies the result into the return value and
  releases its receive buffer.
- Clang UBSan found Lemon calling `qsort` with a null base for zero elements and
  using an incompatible merge-sort comparator type; both calls now satisfy the
  C library/helper contracts.
- Clang UBSan found null-pointer arguments to zero-length `memcpy` while folding
  empty string concatenations; the optimizer now skips empty copies and checks
  allocation failures.
- Clang UBSan found signed left-shift overflow in the bundled decNumber power
  scans; the bit scans now use the corresponding unsigned type.
- Clang UBSan found NaN-to-integer conversion in the diagnostic `db_decimal`
  rounder; NaN is now preserved for the plugin's existing signal handling.
- Clang UBSan found the Level B `x2d` implementation using out-of-domain
  signed `ishl` while constructing a 64-bit two's-complement result. The
  library now uses a range-safe negative accumulator without changing the
  instruction contract.
- Clang UBSan found an unaligned back-fence store in the linked-list plugin;
  the fence is now written with `memcpy`.
- Clang UBSan found empty binary-to-string conversion passing `(NULL, 0)` to a
  non-null UTF helper; `bintos` now uses the VM's existing null-normalizing
  bounded validation helper.

Each correction passed its focused normal-Debug control and sanitizer
reproducer. The final corrected source passes 1,925/1,925 under GCC and Clang
ordinary Debug and Release, exploratory Clang UBSan with
`halt_on_error=1`, and the supported GCC ASan/LSan runner with leak detection
enabled for both the full build and all 1,925 tests.

## Formal Linux baseline

Use separate versioned GCC and Clang manifests. Each full absolute matrix uses
two warmups and ten recorded serial samples per cell. RSS uses zero warmups
and three samples. Capture lifecycle separately. Preserve the approved exact
work:

| Workload | Work |
| --- | ---: |
| Sieve | 5,500 |
| Permute | 5,000 |
| Bounce | 4,200 |
| Richards | 20 |
| Base64 | 2,500 |
| Towers | 100 |
| RexxCPS | benchmark-native rate |

The common aggregate remains exactly Sieve, Permute, Bounce, Richards and
Base64. Towers and RexxCPS remain separate. Mandelbrot, Storage, List and JSON
retain their approved exclusions. Report GCC and Clang scorecards separately;
do not compare Linux absolute time with Mac absolute time or form a
cross-session before/after ratio.

## Native and schema-5 attribution

Build exact optimized benchmark images once and use the same image and library
within each compiler lane. Capture schema-5 counts for both VMs. Cover the
full qualified portfolio once, then focus native evidence on Richards,
Base64, Towers and the separately governed RexxCPS community lane, with Sieve,
Permute and Bounce as accepted-win/layout controls.

Minimum native event groups:

- cycles, instructions, branches and branch misses;
- top-down retiring, bad speculation, front-end bound and back-end bound;
- `frontend_retired.l1i_miss`, `frontend_retired.itlb_miss`,
  `icache_64b.iftag_miss` and `itlb_misses.walk_completed`;
- retired and mispredicted indirect branches; and
- task clock, context switches, migrations and page faults as host controls.

Avoid event multiplexing where possible; split groups and retain each raw
sample. Use `perf record`/`perf report` and focused annotation for `run()`,
value/copy helpers and selected conversion/string paths. Valgrind/Callgrind or
Massif are diagnostic fallbacks only and never timing authority.

The completed schema-5 capture contains all 11 qualified noopt/opt workload
pairs under both GCC profile VMs (44 complete cells) and a focused
Sieve/Permute/Bounce/Richards/Base64/Towers/RexxCPS compiler-control set under
both Clang profile VMs (28 complete cells). Every profile has schema 5, exact
input hashes, complete domains, zero overflow and RXSEQ N=2/3/4. The RexxCPS
counts-only Clang append uses a versioned bounded smoke form linked to the
formal canonical-default result.

The ordinary GCC Release product has 98 non-multiplexed `perf stat` files:
seven workloads, both VMs and seven isolated event passes. The bounded Clang
control has 56 files: the same 14 cells with core, top-down, L1I/iTLB-walk and
indirect-branch passes. All 154 captures passed their benchmark correctness
string, scheduled at 100% and contain no unsupported or uncounted event.
The constrained `frontend_retired.*` aliases exposed by `perf list` could not
be co-scheduled; their documented raw encodings counted successfully in
separate GCC passes.

Sixteen focused cycle profiles cover Richards, Base64, Towers and RexxCPS
under both VMs and compilers. They contain 242 to 5,268 samples per flat
capture with no lost samples. The four canonical RexxCPS captures contain 242
to 590 samples and support broad attribution percentages, not precise
cross-compiler deltas. One GCC Richards `rxvm` capture additionally retains
5,026 DWARF samples. This host's `perf report` cannot render DWARF or LBR call
graphs in reasonable time and `perf annotate` also stalls, so flat symbol
reports are authoritative here. Focused `symbol,symoff` histograms and exact
executable disassembly retain equivalent hot-instruction evidence without
another build or benchmark run. The full command, product hash and raw output
remain beside every cell under `native-perf/`.

RexxCPS native cells use the canonical default. Its calibration can select
different effective counts independently in each event pass, so raw absolute
counter totals are not directly comparable. The summary normalizes those
totals to 10 million clauses using each retained `REXXCPS-EFFECTIVE` record;
within-pass IPC, miss rates and top-down percentages remain unscaled.

The decisive observations are:

- optimized Richards records 56.9 million copy operations and 451.7 MB of copy
  payload in the bounded schema-5 run; native profiles put 55-57% of GCC and
  77% of Clang cycles in `copy_value`, with Clang also putting 16.5% in
  `maybe_trim_attribute_storage`;
- optimized Towers records 26.8 million copy operations, 31.3 million
  clear/reset/destroy operations and 5.86 GB of allocation requests; native
  profiles select `copy_value`, `clear_value_contents` and
  `reset_value_storage_for_reuse`, while PMU evidence shows about 9% indirect
  branch misses and 12-14% front-end bound in the GCC lane;
- optimized Base64 executes 46.7 million VM instructions, with
  `SCOPY_REG_REG` the third-ranked opcode; 92-96% of sampled cycles remain in
  `run`, the GCC lane is about 51% back-end bound, and libc
  conversion/string helpers are individually small;
- optimized GCC RexxCPS executes 6.11 million VM instructions in the bounded
  smoke, with 673,511 conversions and 330,174 allocation requests; canonical
  GCC `rxvm` is 42.0% front-end bound, while `run` takes 35-44% across the four
  cycle profiles and decimal parse/format/arithmetic plus string movement
  dominate the remainder; and
- compiler direction is workload-sensitive. Clang is materially smaller and
  points faster for Base64/Richards but slower for Towers. The compiler lanes
  were not paired/interleaved and their comparator drift is material, so these
  are mechanism controls, not an accepted compiler or layout verdict.

## Linux sufficiency and return-to-macOS gate

This host is the authority for Linux x86-64 correctness, sanitizer behaviour,
ELF/product identity and Intel PMU evidence. It is not the preferred machine
for repeated production rebuilds or broad candidate iteration.

The initial Linux campaign has done enough when all of the following are true:

1. GCC and Clang profiling-off Debug/Release correctness, supported GCC
   ASan/LSan and the bounded exploratory Clang UBSan inventory are retained.
2. Formal GCC/Clang timing, RSS, lifecycle and artifact lanes are retained,
   with governed appends and unresolved noise reported honestly.
3. Schema-5 counts cover the full qualified portfolio in both VMs under at
   least the primary GCC lane, with a bounded Clang compiler-sensitivity
   control sufficient to expose direction reversals.
4. Ordinary profiling-off binaries have Intel `perf stat` evidence for the
   seven selected workloads and `perf record`/report evidence for the focused
   Richards, Base64, Towers and RexxCPS hotspots.
5. Linux-specific build cost, memory pressure, sanitizer defects and native
   hotspot opportunities are summarized well enough to rank the next
   experiments without another production build.

Once these conditions hold, stop expanding the Linux campaign. Return to the
faster macOS host for mechanism design, disposable PoCs, candidate selection
and rebuild-heavy iteration. Come back to Linux only for a batched selected
candidate, Linux-specific counter/sanitizer questions, or the final
cross-platform validation. A question that can be answered from retained
Linux artifacts, schema-5 CSVs or a small external harness does not justify a
new Linux product build.

All five conditions are now met. The Linux campaign stops at attribution.
The next work belongs on the faster macOS host and remains pre-production:

1. Build disposable ceilings for a scalar/no-payload `copy_value` fast path
   versus compiler/RXAS copy specialization, preserving reference, decimal,
   native-payload and recursive-attribute semantics.
2. Isolate the measured Richards/Towers attribute-storage trim and teardown
   shapes in outside-tree harnesses. Do not retry rejected reset-list, slab or
   broad layout ideas without a new exact reduction proof.
3. Isolate Base64's `SCOPY_REG_REG`/dispatch/string shapes and compare semantic
   specialization with code-layout approaches before selecting either.
4. Rank the PoCs, select and batch at most one coherent production slice, then
   pay for one deliberate ordinary Release rebuild and mandatory first verdict
   on macOS.
5. Return to this Linux host only after that verdict is accepted and a selected
   candidate needs GCC/Clang PMU, sanitizer or final platform validation.

## Stop rules

- zero correctness failures before timing;
- no performance-motivated production source edit in this activity; genuine
  sanitizer correctness findings may be corrected only with retained patches,
  focused normal controls and full sanitizer/ordinary-product revalidation;
- stop for Adrian before `PERF2-06-D01`, any LTO/PGO/layout candidate,
  architecture/ABI/language/RXAS/RXBIN decision, or default-VM selection;
- report a GCC/Clang direction reversal or unexplained greater-than-3%
  qualified-cell difference before selecting a compiler/layout mechanism;
- preserve separate lifecycle, RSS and artifact scorecards;
- do not weaken work, correctness expectations or formal sampling because
  this host is slower; and
- do not claim Gate E completion before supported Linux ARM64 and Windows
  lanes reconcile with Apple and Linux x86-64.

## Execution checklist

- [x] Repair and verify retained Mac lifecycle RXBIN evidence.
- [x] Install and smoke exact comparator versions.
- [x] Freeze detached source, host, toolchain, comparator hashes and scope.
- [x] Configure/build independent ordinary and diagnostic products.
- [x] Refresh and pass full GCC/Clang correctness after sanitizer corrections.
- [x] Pass final GCC Debug then ASan/LSan and exploratory Clang UBSan.
- [x] Capture formal GCC timing/RSS/lifecycle/artifacts.
- [x] Capture formal Clang timing/RSS/lifecycle/artifacts.
- [x] Capture schema-5 counts and native counter/sample evidence.
- [x] Apply the Linux sufficiency gate and identify work that returns to macOS.
- [x] Publish the Linux baseline/attribution report and stop before candidate
      selection.
