# PERF2-01 current baseline and attribution refresh worklist

Status: complete; Gate A accepted 2026-07-23

Started: 2026-07-23

Accepted: 2026-07-23

Purpose: resumable control plane for the first executable activity in the
second cREXX performance programme. This activity establishes current truth;
it does not authorize or implement a production optimization.

## Gate A exit and mandatory stop

PERF2-01 reaches Gate A only when all of the following are true:

- [x] one final PERF2-01 evidence bundle is checksum-closed and independently
      reverified;
- [x] the five-workload cREXX/ooRexx/decimal-NetRexx comparison is from one
      governed same-session capture and the cREXX/ooRexx ratios are accepted;
- [x] canonical external RexxCPS and the separately disclosed cREXX 2.2d
      diagnostic are current same-session evidence;
- [x] every Tier A comparability/adaptation label is current and no invalid or
      adapted cell enters a common aggregate;
- [x] all eleven workload dossiers and the cross-workload mechanism census are
      complete;
- [x] every candidate placed in the PERF2-02/03/04/06/07 panels has a measured
      mechanism footprint and a named likely owner; and
- [x] no accepted profile has an unexplained degraded or overflowed domain.

**Stop point:** present the refreshed ratios, dominant workload costs,
limitations, ownership map and first bounded PoC recommendation to Adrian, then
stop for acceptance. Do not begin PERF2-02 quickening, any production
optimization, or any commit/push in this activity.

## Scope guard

- [x] No quickening or private instruction specialization.
- [x] No inlining cleanup, BIF/RXAS opcode, frame/value representation, LTO/PGO,
      language, Level B/G, RXBIN, public ABI or benchmark-shortcut work.
- [x] Diagnostic instrumentation may be extended only where the coverage audit
      proves required evidence is unavailable.
- [x] Profile time and counts are diagnostic only. Ordinary profiling-off
      `Release` product timing is the sole product authority.
- [x] Every PERF2-01 analysis program and maintained orchestration tool added by
      this activity is cREXX Level B under `performance/tools/`; Python is not
      used. Native host sampling utilities remain permissible diagnostic inputs.
- [x] Historical charter and closed initial roadmap remain untouched.

## Stage 0 - exact baseline and dirty-tree isolation

### Live repository state before the first PERF2-01 edit

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- HEAD: `d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6`
- Upstream: `origin/develop`
- Upstream commit: `d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6`
- Ahead/behind: `+0/-0`
- Product-source relationship: product source is identical to HEAD. Every
  pre-existing path is under `performance/` and is documentation/control-plane
  content; no compiler, assembler, linker, VM, library or benchmark source was
  modified before PERF2-01.
- Clean formal-product worktree:
  `/private/tmp/crexx-perf2-01-product.dbLYqo`, detached at the exact HEAD above.

Pre-existing user changes to preserve exactly:

| Path | Initial state | Initial SHA-256 where applicable |
| --- | --- | --- |
| `performance/README.md` | modified tracked documentation | tracked base `f97a91adc2346681121c53434215a27eae1b4b22`; user content preserved |
| `performance/ROADMAP.md` | modified tracked live roadmap | tracked base `df83025ad9ec0ada71250758c39a6790f23c53c0`; user content preserved |
| `performance/PERF2-01-HANDOVER-PROMPT.md` | untracked documentation | `57a1c06e5d874c26afd5978a9ba03499c3bbb2a7f732645fcf9c508ab3448283` |
| `performance/ROADMAP-INITIAL-SWEEP-2026-07-23.md` | untracked historical archive | `6c0c8c01a123271f41acdeb33ec3ae3101028eaf619c22d2149fc742e8478034` |
| `performance/TEAM-PERFORMANCE-UPDATE-2026-07-23.md` | untracked documentation | `6e4c17f63a114e6a48163553048f95196931b37dbb30841bc10202ef33892eee` |

Never reset, stash, overwrite, stage, commit or push these changes merely to
obtain a clean build tree.

### Host and power state at start

- Host: `Mac.lan`; model identifier `Mac17,3`; Darwin `25.5.0`; macOS
  `26.5.2` build `25F84`; arm64.
- CPU: Apple M5; 10 logical CPUs.
- Memory: 25,769,803,776 bytes (24 GiB).
- Power: AC power, battery 77% and charging; AC low-power mode `0`.
- Thermal: no recorded thermal, performance or CPU-power warning.
- Initial load averages: `2.03 1.96 2.32`; uptime 9 days 10:46.
- Available storage: approximately 639 GiB on the data volume and `/private/tmp`.
- Formal-run rule: re-capture AC/low-power/thermal/load immediately before and
  after every formal block; run under `caffeinate -i`; do not overlap builds,
  tests or other benchmark campaigns.

### Toolchain and installed runtimes at start

| Component | Version / path | Initial SHA-256 where meaningful |
| --- | --- | --- |
| CMake | 4.3.2 | version capture retained later |
| Ninja | 1.13.2 | version capture retained later |
| Apple clang | 21.0.0 (`clang-2100.1.1.101`) | `/usr/bin/clang` |
| JDK/JVM | Temurin OpenJDK 26.0.1+8, mixed mode | `/usr/bin/java`: `d641f84fbed5fcd611d603fe5aa364f152462d7d099e09eeaf36f046be4c3f32` |
| ooRexx | 5.1.0 r12973 | `/Users/adrian/.local/opt/oorexx/5.1.0-12973/bin/rexx`: `42b631e871a70f3da782af8c1045f7ed4167c3ed58834e7e4dcfd86b514c2ee8` |
| Regina | 3.9.7, 64 bit | `/opt/homebrew/bin/rexx`: `ef19776959f9b3193dd66924202f4ae0e8c19ff2c69ccdb0a458451991f035f5` |
| NetRexx | 5.10-GA build 18-20260320-1410 | compiler wrapper: `ca22d3a2a9236b3597ef2c5c8cbd6fd3b9edd86015c6892059480ee4d21a8d3b` |
| installed cREXX | beta.3 local `g86006fac18a8.dirty`; orientation only | `crexx`: `652e548a94767116d98ced6993d35a83aba64f7d30a15f9229eee2518c6498dd`; `rxvm`: `9a6d6d05ab241230d6c107190ebe6f2111677c07d2d2fe51446f6fd3be9cd058`; `rxbvm`: `f818fb7c6327c62f72d7e7e6e10ed7dc94d477962e247492ebff9e5e1dc1c48c` |

The installed cREXX binaries are tool-bootstrap inputs only and cannot become
the current timing authority. Formal product binaries must be built from the
frozen scratch worktree and hashed.

### Exact build configurations

Ordinary product authority:

```sh
cmake -S /private/tmp/crexx-perf2-01-product.dbLYqo \
  -B /private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release \
  -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF -DBUILD_TESTING=ON
cmake --build /private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release \
  --parallel 10 --target rxc rxas rxlink rxvm rxbvm crexx language_benchmarks
```

Diagnostic build (created only after the coverage audit; exact base commit is
the same and any diagnostic patch is separately inventoried):

```sh
cmake -S DIAGNOSTIC_SOURCE \
  -B DIAGNOSTIC_SOURCE/cmake-build-perf2-profile \
  -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=ON -DBUILD_TESTING=ON
cmake --build DIAGNOSTIC_SOURCE/cmake-build-perf2-profile \
  --parallel 10 --target rxc rxas rxlink rxvm rxbvm rxseq \
  test_rxvmprofile language_benchmarks
```

The final provenance record must replace `DIAGNOSTIC_SOURCE` with the exact
scratch path and record the diagnostic patch hash plus both CMake caches.

### Intended retained layout

One final compact bundle:

```text
performance/evidence/2026-07-23-perf2-01-current-baseline/
  README.md
  00-provenance/       repo, host, power, runtime, source/tool/build hashes
  01-build/            cache/flags and compile-out/focused-test proof
  02-product-timing/   all 11 Tier A x rxvm/rxbvm raw and consolidated rows
  03-external/         same-session common matrix and RexxCPS runtimes
  04-lifecycle-rss/    separately labelled lifecycle, RSS and artifact lanes
  05-profiles/         optimized and bounded no-opt schema profiles/summaries
  06-rxseq/            exact N=2/3/4 captures, module hashes and decoded rows
  07-native/           uninstrumented samples/counters and host limitations
  08-heap/             targeted allocation lifetime evidence
  09-controls/         exact-hash RexxCPS family controls
  10-dossiers/         workload dossiers, gap ledger, mechanism census/panels
  11-verification/     commands, status audit and recursive checksums
  checksums.sha256
```

Planned versioned manifests live under `performance/manifests/` and contain
only exact paths/arguments/hashes. Reproducible build output and disposable
native-system capture intermediates remain outside the repository.

### Stage 0 commands and completion

```sh
git status --porcelain=v2 --branch
git diff --name-status
git ls-files --others --exclude-standard
git rev-parse HEAD '@{upstream}'
git -C /private/tmp/crexx-perf2-01-product.dbLYqo status --porcelain=v2 --branch
pmset -g batt
pmset -g custom
pmset -g therm
sysctl -n hw.logicalcpu hw.memsize
df -h /private/tmp /Users/adrian/CLionProjects/CREXX
```

- [x] Exact live branch, HEAD, upstream and dirty scope recorded.
- [x] Product-source identity and documentation-only dirty scope confirmed.
- [x] Clean detached formal-product worktree frozen.
- [x] Initial host/power/thermal/storage state recorded.
- [x] External runtime inventory and initial hashes recorded.
- [x] Frozen product build configured, built, tested and hashed.
- [x] Pre-formal-run host state accepted.

Blockers: none at Stage 0 start.

## Stage 1 - profiler and evidence-tool gap audit

Write the coverage table before changing profiler or evidence code. For every
row choose exactly one disposition: `available now`, `derivable now`, `missing
and required`, or `not justified`, and name the authoritative source/derivation.

| Domain | Required telemetry | Disposition | Exact source / gap | Action |
| --- | --- | --- | --- | --- |
| Opcode/transition/procedure/call | counts, timings, sites, arity, return mechanics | available now | schema 4 `instruction`, `transition`, `procedure`, `census`, `call`, `return`, `mechanics` and `unwind` rows; exact call module/index and arity are retained | Keep; schema 5 preserves these rows and adds explicit domain status. |
| RXSEQ | N=2/3/4 counts, sites, modules, exact-image status | available now | versioned RXSEQ files retain module table/hashes and site windows; `rxseq` checks the supplied module set | Keep; capture all 11 optimized images for both VMs and state the taken-branch/call window boundary. |
| Frames | fresh/reused and entry/reset subphases | missing and required | schema 4 has fresh/reused activations, high water and procedure entry/exit time, but no attribution for local/global relink, `a0`, inherited/root context or plugin-context sync; `frame_f()` performs these paths | Add schema-5 fixed frame-entry phase counters/timing split by fresh/reused disposition; use counts mode when deterministic counts are needed. |
| Value transfers | copy, typed copy, move, clear, reset, destroy by shape/bytes | missing and required | opcode rows see explicit RXAS operations but miss recursive/helper transfers and teardown; all 11 optimized benchmark RXAS files contain transfer sites | Add schema-5 fixed operation/shape count and payload-byte rows at the common value helpers plus scalar/status/decimal typed-copy handlers. Nested helper calls remain explicitly labelled rather than silently deduplicated. |
| Representations | string/numeric/decimal conversions, materialization, cache hit/miss | derivable now | explicit conversion/materialization opcodes are already counted dynamically; string/binary/decimal buffer requests and bytes plus schema-5 transfer bytes expose payload materialization. The current `value` model keeps independent eager fields and has no general representation cache whose hits/misses could be counted. UTF-8 validity flags and selector caches are separate mechanisms. | Derive conversion families in the Level B summarizer. Do not invent a generic representation-cache counter; report the absence of that mechanism and use exact buffer/transfer rows. |
| Control flow | branch direction, loop/backedge and exceptional exits | missing and required | schema 4 transition totals do not bind sequential/taken outcomes to an opcode site. Every optimized workload has static branch sites (8 to 305 in the benchmark module alone). | Add schema-5 branch-site rows with module/index/opcode, executions, taken, fall-through, cross-module and backward-target counts. |
| Stable sites | static identity, types/targets, hits/misses, invalidations | not justified | the 11 optimized benchmark modules contain zero `srcmethodsel`/`srcfprocsel` sites. Schema 4 already reports aggregate selector attempts/success/failure and exact dynamic-call sites. The only current VM selector caches are those two opcode families. | Retain aggregate rows and check the split-library profiles. If both selector attempt totals are zero, record `not exercised`; only reopen a site-cache extension if current evidence shows executed selectors. |
| Lifecycle | load/bind/image/plugin/first-frame/teardown phases | derivable now | the maintained lifecycle tool provides process-inclusive load-to-exit time; schema 4 provides external-root/first-frame entry and VM execution attribution; artifact hashes/sizes separate image input. Inner loader/binder/plugin/teardown phase clocks would perturb a lane whose authority is the profiling-off process result. | Keep lifecycle/RSS/artifact lanes separate and use uninstrumented native samples for load/teardown ownership. Do not add inner timing hooks in PERF2-01. |
| Allocation | VM requests plus system alloc/free lifetime, retained/high water, size classes/stacks | available now | schema 4 records VM allocation requests, requested/max bytes, value slots and frame high water. macOS supplies `/usr/bin/heap`, `leaks`, `vmmap`, `malloc_history` and `sample`; `xctrace` is unavailable on this host. | Keep VM rows; use native tools with `MallocStackLogging` for selected outliers. Record that hardware cycles/instructions, branch-miss and i-cache/iTLB counters are unavailable locally. |
| Compiler artifacts | static instructions/cells/bytes/register ceiling/inline sites and rejects | derivable now | optimized/no-opt RXAS, `rxdas`, artifact inventory, compiler metadata and schema profiles provide the required static/dynamic views | Extend the Level B PERF2 summarizer, not the product or compiler. |

Audit inputs and commands:

```sh
rg -n "RXVM_INSTRUMENTATION_|rxvm_profile_|RXVM_PROFILE_" interpreter
rg -n "copy_value|move_value|clear_value|reset|convert|material" interpreter
rg -n "profile|schema_version|allocation|census|rxseq" \
  performance/tools performance/manifests interpreter/tests
```

- [x] Coverage table completed with no roadmap-driven speculative counters.
- [x] Schema decision recorded: implement schema 5 for counts mode, explicit
      domain status, frame-entry phases, value-transfer/teardown bytes and
      branch-site direction/backedges; preserve every schema-4 row.
- [x] Backward handling specified: the new Level B summarizer must accept both
      schema 4 and schema 5, treating schema-5-only domains as unavailable on a
      schema-4 input rather than failing or fabricating zeroes.
- [x] Native/system-tool availability recorded: `sample`, `heap`, `leaks`,
      `vmmap` and `malloc_history` are installed; `xcrun xctrace` is absent, so
      hardware cycles/instructions, branch-miss, i-cache and iTLB counters are
      not locally available and must be reported as a host limitation.

Blockers: none; any required language/RXBIN/public-ABI/architecture expansion
becomes `decision required` and stops for Adrian.

## Stage 1B - diagnostic extension, only if the audit requires it

- [x] Stable schema/row/field definitions and per-domain status documented.
- [x] Counts-only mode is deterministic where used.
- [x] Schema-4 inputs remain accepted by maintained summarizers.
- [x] Focused unit, CSV, documentation and Level B tool self-tests pass.
- [x] Ordinary `CREXX_VM_PROFILING=OFF` preprocessing/object proof shows the
      new instrumentation compiles out.
- [x] Diagnostic patch is isolated from the clean product build, hashed and
      separately attributable.
- [x] No profile elapsed time is used as product evidence.

Focused command template (all verbose output to `mktemp` logs):

```sh
cmake --build DIAGNOSTIC_BUILD --parallel 10 \
  --target test_rxvmprofile rxvm rxbvm rxseq
ctest --test-dir DIAGNOSTIC_BUILD -R \
  'rxvmprofile|rxbvmprofile|vmprofilingdocumentation|rxvmsequence|rxbvmsequence' \
  --output-on-failure
```

## Stage 2 - product-authority timing matrix

Contract: all eleven optimized Tier A workloads, both VMs, ordinary
profiling-off Release, split exact image plus current `library.rxbin`, retained
source/TRACE metadata, serial rotated sampling, two warmups and ten recorded
runs, then unchanged ten-run appends for any MAD/span trigger. No passing sample
is removed.

- [x] Focused `language_benchmarks` correctness gate passes.
- [x] Exact source/module/image/runtime hashes retained.
- [x] 22 timing cells complete with every warmup/recorded row passing.
- [x] Noise append rules applied without sample deletion.
- [x] RexxCPS native clauses/s and process-inclusive elapsed both retained.
- [x] Lifecycle, RSS and artifact-size lanes captured separately.

Blockers: any correctness failure or measurement defect invalidates the
affected cell and is isolated before evidence use.

## Stage 3 - same-session external comparison

- [x] Five common workloads run with equal work for cREXX `rxvm`, cREXX
      `rxbvm`, ooRexx and decimal-semantics NetRexx.
- [x] Runtime order rotates by workload/round; two warmups and ten recorded
      observations per cell; policy appends applied.
- [x] Canonical Classic RexxCPS run for ooRexx, Regina and NetRexx.
- [x] cREXX RexxCPS 2.2d reported separately, never in the common aggregate.
- [x] Exact runtime versions, commands, JVM substrate and hashes retained.
- [x] Historical columns, if shown, are labelled orientation only.
- [x] Common aggregate membership is exactly Sieve, Permute, Bounce, Richards
      and Base64; no imputation.

## Stage 4 - diagnostic attribution

- [x] Optimized diagnostic profiles for all 11 workloads in both VMs.
- [x] No-opt attribution for all 11, or at minimum RexxCPS, Bounce, Richards,
      Base64 and one already-winning control with an affordability record.
- [x] Exact RXSEQ N=2/3/4 captures and module sets for both VMs.
- [x] RXSEQ interpretation states that calls/taken branches terminate windows.
- [x] Native uninstrumented portfolio sample/hot-stack coverage captured once.
- [x] Cycles/instructions/branch/miss/i-cache/iTLB availability or exact host
      limitation recorded; largest gaps/noisy hotspots repeated.
- [x] Targeted system allocation lifetime profiles for Bounce, Richards,
      Storage and any newly selected outlier.
- [x] Exact-hash RexxCPS controls cover BIFs, internal calls/ARG,
      TRACE/ADDRESS, stems, decimal/string loops and PARSE without replacing
      the published 2.2d diagnostic.

## Stage 5 - deliverables and verification

Each workload dossier must contain:

- [x] comparability status and exact source/image/runtime hashes;
- [x] `rxvm`, `rxbvm` and ooRexx throughput where qualified;
- [x] current ratio and gain required for parity, 1.50x cell target and 2.00x
      common-geomean target;
- [x] static/dynamic instructions and image footprint;
- [x] top procedures, opcodes, transitions and native stacks;
- [x] calls, frame reuse/reset, transfers/conversions and bytes;
- [x] allocation requests, lifetime evidence and RSS;
- [x] stable-site/cache evidence where available; and
- [x] selected likely owner: compiler/inliner, RXAS, link/load, VM quickening,
      value/frame work or capability/equivalence.

Portfolio deliverables:

- [x] `performance/evidence/benchmark-median-summary.md` updated.
- [x] Cross-workload mechanism census complete.
- [x] Bounded candidate panels for PERF2-02/03/04/06/07 tied to measured rows.
- [x] Every profile domain status audited; no unexplained degraded/overflow.
- [x] Commands, exact counts and verification record retained.
- [x] Recursive `checksums.sha256` generated and reread/verified.
- [x] Worklist boxes and blockers reflect the actual final disposition.
- [x] Gate A result packaged for Adrian; stop for acceptance.
- [x] Adrian accepts PERF2-01 Gate A as the refreshed selection baseline.

Final disposition: Adrian accepted PERF2-01 Gate A on 2026-07-23. PERF2-01 is
complete. No PERF2-02 implementation has started, no production change was
authorized by this acceptance, and no file has been staged, committed or
pushed.

## Resumption rule

On resume, first reread root/performance instructions and the live roadmap,
then inspect this worklist from the first unchecked box. Reverify live git and
host power state before any formal block. Never regenerate or overwrite a
passing retained block unless its exact provenance is invalid; append under the
governed rule and record why.
