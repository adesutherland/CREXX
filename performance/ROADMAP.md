# cREXX performance roadmap

Last updated: 2026-07-29

Status: live planning register for the second performance programme.
Production work requires the explicit decision gates recorded in each activity;
PERF2-02 contains the first accepted production slice.

The initial sweep is closed and preserved, without rewriting its accepted or
rejected history, in
[`ROADMAP-INITIAL-SWEEP-2026-07-23.md`](ROADMAP-INITIAL-SWEEP-2026-07-23.md).
The original dated charter remains
[`performance-programme-report-2026-07-15.md`](../docs/planning/release-1/performance-programme-report-2026-07-15.md).
This file is now the live control plane.

Status values are `queued`, `in progress`, `decision required`, `blocked`,
`deferred`, `complete`, `rejected` and `superseded`. `Complete` means the
stated exit gate and retained evidence both exist; it never means merely that a
prototype ran.

## North star

cREXX should become a materially faster Rexx implementation than ooRexx while
preserving the language, portability, debugging, linking and embeddability
advantages of the current architecture.

The programme has an intermediate threshold and a final outcome:

1. **Parity checkpoint, not completion:** on a clean exact commit and one
   governed same-host session,
   the selected default cREXX VM must be clearly faster than ooRexx on every
   qualified equal-work common Tier A workload, and the governed disclosed
   cREXX RexxCPS 2.2d diagnostic must beat same-session canonical Classic
   ooRexx RexxCPS. “Clearly” means the median is above parity and the governed
   sampling/interval disposition is favorable rather than noisy or
   inconclusive. There must be zero correctness failures and no unresolved
   regression guard.
2. **Unquestionable-superiority exit:** the selected default VM must reach at
   least 1.50x ooRexx median throughput on every qualified comparable cell and
   at least 2.00x on the common-workload geometric mean. The separately
   governed RexxCPS diagnostic must also reach at least 1.50x ooRexx. The
   alternate/non-default VM must itself be clearly faster than ooRexx on every
   qualified cell, not merely remain within a cREXX regression budget.

The numerical bands are the roadmap's working definition of “unquestionable”:
large enough that normal noise, one favorable workload or a marginal aggregate
cannot reverse the conclusion. They can be raised by Adrian, but ordinary
parity cannot be substituted for programme completion.

The product/architecture conclusion must be equally clear. The winning results
must survive the supported host/compiler matrix, retain both VM modes and the
portable canonical RXBIN, and demonstrate that new Rexx semantics can be
placed at compile, RXAS, link/load or guarded-runtime specialization without a
benchmark shortcut or a mandatory public-ISA fork. ooRexx is the comparator,
not the performance ceiling; cREXX should be the no-brainer foundation for
future Rexx enhancement work.

These thresholds apply only to semantically qualified cells. They cannot be
met by dropping a hard benchmark, changing its timed work, exploiting its
correctness check or treating a disclosed adaptation as equivalent. Every Tier
A row must finish either as a governed common comparison or with a named
capability/equivalence activity that explains what must change before it can
join the score.

Performance is not the only release dimension. Correctness, lifecycle, peak
RSS, installed/native use, artifact size, applicable RXBIN/ABI compatibility
and feature-gating, TRACE/source identity and both VM modes remain separate
gates under
[`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md) and
[`AGENTS.md`](AGENTS.md).

## Strategic conclusion

The cross-runtime RexxCPS review does **not** reduce to “hoist variables.”
Regina, ooRexx and NetRexx repeatedly apply a broader rule:

> Resolve, prove or compile a semantic decision at the earliest phase where it
> is stable, retain the result close to the execution representation, and keep
> a complete dynamic fallback.

cREXX already gives typed locals direct register/index placement. The larger
remaining opportunity is to hoist **semantic work**: call and BIF identity,
validation, parse/scan plans, type and representation decisions, loop
invariants, frame setup, conversions and repeated sequence selection.

The ownership rule for the new programme is:

| Facts become stable at | Preferred owner | Typical result |
| --- | --- | --- |
| Compile time | `rxc`, typed flow analysis and inlining | direct binding, invariant motion, result placement, dead scaffold removal |
| RXAS assembly time | RXAS whole-procedure effects/flow | destination forwarding, machine-level cleanup, coherent semantic instruction selection |
| Link time | `rxlink` | provider/member/procedure identity and immutable graph facts |
| Load/preparation time | private process execution image | decoded operands, runtime pointers, eagerly prepared process facts |
| First or repeated execution | guarded quickening/site cache | type- or target-stable private form with invalidation |
| Unstable or exceptional path | canonical fallback | late load, mutation, TRACE/debug, signals, unusual types and full semantics |

The earliest safe, fastest end-to-end placement wins. Runtime quickening is a
priority because the process-local execution image and recent semantic
fast-path work make it timely, not because runtime specialization should absorb
facts already provable by the compiler or RXAS.

Three further boundaries are fixed:

- Core Level B BIF work is **inlining first**. A small number of general RXAS
  or VM assists may support the irreducible semantic kernels; blanket native
  conversion is not the plan.
- More-than-three-operand RXAS support is already complete across assembler,
  RXBIN, linker, disassembler, metadata, compiler and both VMs. Width is an
  available design tool, not evidence that a wide instruction is profitable.
- Opcode operand width and procedure-call arity remain separate questions.
  `CALL1` through `CALL4` are already complete and historical evidence placed
  90.997% of calls at arity 0–4; PERF2-01 must refresh the residual census
  before any `CALL5+` or higher-arity frame work is proposed.
- VM work is first-class. Dispatch, execution-image layout, frames, values,
  conversions, interrupt state, code layout and lifecycle all receive current
  measurement rather than being treated as a residual implementation detail.
- RexxCPS is first-class in sampling as well as in the programme exit. Every
  multi-workload representative benchmark, profile, native-PMU,
  compiler/layout and candidate-verdict set carries its separately governed
  cREXX 2.2d lane. Its disclosed adaptation status keeps it outside the
  common-five aggregate; it does not justify omitting it from the evidence set.

The detailed competitor evidence is retained in
[`rexxcps-runtime-source-review-2026-07-22.md`](rexxcps-runtime-source-review-2026-07-22.md).
Its mechanism findings remain useful; its pre-NR-15/16/17 gap sizes and
priority order are historical.

## Initial-sweep closeout

The closed register contains 29 activity rows plus an architecture gate:

| Closing disposition | Count | Activities |
| --- | ---: | --- |
| Complete | 21 | NR-01 through NR-06, NUMERIC-01, NR-08 through NR-11, NR-13 through NR-18, NR-21, NR-26 and NR-27 |
| Rejected | 1 | NR-07 |
| Deferred | 1 | NR-12 |
| Queued | 6 | NR-19, NR-20, NR-22, NR-23, NR-24 and NR-25 |
| Unstarted gate | 1 | architecture selection |

This closes the **initial sweep**, not the underlying open questions. Their
transfer is explicit:

| Initial item | Successor |
| --- | --- |
| NR-12 by-value/return cleanup | PERF2-03 flow-aware inlining and result/copy cleanup |
| NR-19 LTO/PGO/code layout | PERF2-10 toolchain, layout and lifecycle |
| NR-20 and NR-25 allocation/value ideas | PERF2-07 value, frame, representation and allocation work |
| NR-22 compact execution stream | PERF2-06 VM execution-engine programme |
| NR-23 quickening | PERF2-02 semantic quickening priority programme |
| NR-24 selected fusion | PERF2-02, PERF2-05 and PERF2-06 placement comparison |
| old architecture-selection footer | PERF2-11 explicit cross-platform architecture gates |

Ideas already proved complete, rejected, superseded or subsumed remain in the
archive. Deferred `FDIVSUB`, compact TRACE-correct `ILOADSETUNLINKN`, broader
copy/propagation and metadata ideas may enter a new panel only if current
profiles give them a mechanism footprint; they are not silently re-queued.

This roadmap was written against clean `develop` at
`d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6`. PERF2-01 must freeze and record
its own exact execution baseline and upstream state rather than inheriting that
snapshot by assumption.

## Current same-session Mac closure

The 2026-07-27 PERF2-09 bundle is the current accepted-product same-session
Apple baseline. Its competitor cells, work arguments and ordinary
profiling-off Release products were frozen and captured together. It supersedes
the July 20/23 cross-session orientation for Mac candidate ranking but is not a
cross-platform or final-superiority claim.

| Workload | Qualification | `rxvm / ooRexx` | `rxbvm / ooRexx` | Approximate gain needed to pass ooRexx | Programme disposition |
| --- | --- | ---: | ---: | ---: | --- |
| Sieve | common | 7.214291x | 5.338790x | already ahead | Guard the win; `rxbvm` remains 1.400305x NetRexx. |
| Permute | common | 8.005043x | 7.015322x | already ahead | Guard accepted direct placement; current deficit is only to NetRexx. |
| Bounce | common | 3.902513x | 2.963270x | already ahead | Guard; only `rxbvm` misses the 1.50 NetRexx band. |
| Richards | common | 0.267262x | 0.264171x | 3.74x / 3.79x | Largest qualified common deficit; re-attribute residual accepted-product work before selecting a mechanism. |
| Base64 | common | 0.719817x | 0.724922x | 1.39x / 1.38x | Both cREXX series remain noisy after the governed append; require exact materialization reduction. |
| RexxCPS | governed disclosed diagnostic, not common | 0.995754x | 0.933193x | 1.004x / 1.072x | Near-parity guard; still 1.506x/1.607x from the separate strong band. |
| Mandelbrot | approved exclusion | — | — | ooRexx checksum invalid | No ratio; ordinary decimal modes do not reproduce the binary64 contract. |
| Towers | qualified separate object lane | 0.328060x | 0.321343x | 3.05x / 3.11x | Re-attribution required before any allocation/value candidate. |
| Storage | diagnostic exclusion | — | — | cREXX container mismatch | CAP-02 deferred to an explicit post-Release 1 Level G decision. |
| List | diagnostic exclusion | — | — | ownership adaptation | cREXX's weak-reference arena is material extra work. |
| JSON | diagnostic exclusion | — | — | representation/API mismatch | CAP-01 deferred; no common ratio. |

The exact common-five geometric means are 2.125260/1.842840 versus ooRexx and
0.742985/0.644251 versus decimal NetRexx (`rxvm`/`rxbvm`). Richards is the
largest qualified common deficit. The accepted product is ahead of ooRexx in
aggregate but does not yet meet the per-benchmark or NetRexx exit criteria.

Source:
[`2026-07-27 PERF2-09 Mac closure`](evidence/2026-07-27-perf2-09-mac-closure/).

## Live activity register

| ID | Priority | Activity | Status | Dependency / next gate |
| --- | --- | --- | --- | --- |
| PERF2-00 | P0 | Close and archive the initial sweep | complete | Historical register retained; successor mappings recorded here. |
| PERF2-01 | P0 | Clean same-session baseline and complete current attribution refresh | complete | Gate A accepted 2026-07-23; refreshed selection baseline frozen. No production change authorized. |
| PERF2-02 | P0 | Stable-site semantic quickening architecture and PoC panel | complete | Adrian accepted the favorable zero-state Q3b verdict on 2026-07-24. Broad Debug, Release, ASan, isolated-install and retained-RXBIN compatibility gates pass. |
| PERF2-03 | P0 | Flow-aware inlining 2.0 and post-inline cleanup | complete | Architecture H and all five approved slices are accepted. Final production commit `d1c5245d4`; Debug QA 1,915/1,915; decisive List gain 52.818%/53.212%. Residual proof opportunities are routed below and do not keep PERF2-03 open. |
| PERF2-04 | P0 | Inlining-first core Level B BIF campaign | complete | Accepted ladder production commit `f8f34092e`; focused QA 24/24 and broad Debug QA 1,919/1,919; retained closeout checksum-closed. No push authorized. |
| PERF2-05 | P1 | Profile-selected RXAS semantic assists and instruction improvement | complete | P05-CF1, R2a and R1a are accepted and closed green. R1a broad Debug/Release QA is 1,924/1,924. R2b and neutral B1 are evidence-gated future points, not queued work. |
| PERF2-06 | P0/P1 | VM execution-image, dispatch, stream, call and lifecycle audit | Apple tactical frame tuning and initial Intel Linux attribution complete | Linux selects copy/value/storage and workload-sensitive compiler layout as PoC questions, not a production candidate. `PERF2-06-D01`, compact/hot-cold stream selection and the final VM recommendation remain unstarted and return to the faster Mac for bounded design work. |
| PERF2-07 | P1 | Value/frame/copy/representation/allocation programme | Apple slice complete; V1R01-R1 accepted | The proof-wide direct-placement candidate and V3-R01 correctness prerequisite are installed. At the 36-pair cap Richards is -21.224%/-21.076%, Permute -58.019%/-56.466%, common ratios are 1.244352x/1.242301x and no guard hits. Debug/Release each pass 1,925/1,925; focused ASan passes 10/10; lifecycle, RSS, retained-RXBIN and isolated-install guards pass. Other high-cost shapes are evidence-backed defer/reject/architecture transfers. Evidence: [`first verdict`](evidence/2026-07-27-perf2-06-07-v1r01-r1-first-release-verdict/) and [`Apple closeout`](evidence/2026-07-27-perf2-06-07-v1r01-r1-closeout/). |
| PERF2-08 | P1 | Benchmark capability/equivalence and Level B/G decision lane | Mac gate complete; dispositions approved | Common `N=5`; Towers and RexxCPS qualified separately; Mandelbrot, Storage, List and JSON have explicit no-ratio dispositions; CAP-01 through CAP-04 remain under their recorded future owners. Evidence: [`qualification`](evidence/2026-07-27-perf2-08-qualification/) and [`worklist`](PERF2-08-09-WORKLIST.md). |
| PERF2-09 | P0 | Per-benchmark ooRexx closure campaign | Mac closure complete | Current same-session Apple scorecard is checksum-closed. Richards is the largest qualified common deficit. No next candidate is selected. Evidence: [`Mac closure`](evidence/2026-07-27-perf2-09-mac-closure/). |
| PERF2-10 | P2 | LTO/PGO/code layout, build and lifecycle options | Initial Intel Linux baseline/attribution complete; no option selected | GCC/Clang ordinary-product, sanitizer, schema-5 and native-PMU evidence is retained under [`PERF2-10-11-INTEL-LINUX-WORKLIST.md`](PERF2-10-11-INTEL-LINUX-WORKLIST.md). Rebuild-heavy PoCs return to macOS; LTO/PGO/layout selection remains unstarted. |
| PERF2-11 | P1 | Cross-platform architecture selection and final scorecard | Linux x86-64 evidence retained; Gate E incomplete | PERF2-09 Mac and the initial GCC/Clang Linux x86-64 lanes are frozen. A selected candidate, supported Linux ARM64, same-hardware Windows and final VM/default selection remain open. |
| PERF2-12 | P3 | JIT/AOT/native-backend architecture decision | deferred | Revisit only if the non-JIT programme cannot meet the unquestionable-superiority exit. |
| HIGHLIGHT-01 | P0 | Parser-mode UTF-8 projection and missing-token scalability | complete; Release verdict accepted 2026-07-28 | The selected DSLSH position index and cREXX projected-token lookup reduced the formerly timing-out large parser path to a roughly 0.36-second median. THE then removed per-request profile/render compilation, per-span variable-pool traffic and quadratic rendering; the 1,942-line end-user wrapper median is 0.61 seconds. Cross-project Release/Debug suites are green; evidence and boundaries are in [`HIGHLIGHT-01-WORKLIST.md`](HIGHLIGHT-01-WORKLIST.md). |

## Execution order

The programme is deliberately evidence-first but not analysis-only:

1. **Batch 0 — closeout:** preserve the initial sweep, establish this register
   and make no product change. This batch is complete.
2. **Batch 1 — current truth:** execute PERF2-01, accept the same-session gap
   ledger and freeze the first candidate panel. No production optimization is
   selected from July 20/23 cross-session orientation alone.
3. **Batch 2 — competing PoCs:** time-box quickening, inline cleanup/BIF and the
   highest-profile VM/value alternative. Compare placement at compiler, RXAS,
   load and runtime rather than assuming one layer.
4. **Batch 3 — production slices:** take one accepted mechanism at a time
   through the mandatory first profiling-off Release verdict. Stop after each
   verdict for Adrian's direction; do not bury a regression inside a broad
   batch.
5. **Batch 4 — benchmark closure:** rerun the full scorecard after accepted
   slices, select the next largest qualified deficit and guard every existing
   win.
6. **Batch 5 — architecture selection:** complete cross-platform VM and build
   evidence, decide the default execution architecture, and publish the final
   ooRexx comparison.

The dependency shape is:

```text
PERF2-01 current profiles and same-session comparisons
├── stable-site census ───────────────> PERF2-02 quickening
├── inline/call/BIF census ───────────> PERF2-03 ──> PERF2-04
├── RXSEQ/effects/native profiles ────> PERF2-05 and PERF2-06
├── copy/frame/conversion counters ───> PERF2-07
└── capability/equivalence ledger ───> PERF2-08

PERF2-02 through PERF2-08 accepted slices
└── PERF2-09 per-benchmark closure ──> PERF2-11 architecture/final scorecard
```

### Approved remaining platform sequence — 2026-07-27

Adrian selected a Mac-first completion sequence followed by one physical Intel
x86-64 machine running Linux and then Windows. The operational order is:

1. **Mac combined PERF2-06/07 slice.** Finish current value/reference,
   copy/move/clear/conversion/allocation ownership and only the VM/frame questions
   that those results genuinely require. Do not retry rejected C2/C3 or
   cleanup-only interpreter shapes. The resumable control plane is
   [`PERF2-06-07-WORKLIST.md`](PERF2-06-07-WORKLIST.md).
2. **Mac PERF2-08 gate.** Resolve equivalence/capability status before claiming a
   complete PERF2-09 scorecard. This gate may end in an explicit approved
   exclusion; it need not manufacture new language work.
3. **Mac PERF2-09+ first pass.** Run per-benchmark closure, bounded PERF2-10
   Apple controls and the PERF2-11 Apple pre-handover scorecard. Freeze exact
   source/product/workload hashes and the remaining deficit/debt ledger.
4. **Intel Linux handover.** The initial GCC/Clang correctness, sanitizer,
   baseline, schema-5 and native-counter campaign is complete. Its build cost
   makes this host a validation authority rather than the primary iteration
   machine. Use macOS for bounded design/PoCs and rebuild-heavy candidate
   selection, then return here only for a batched selected candidate,
   Linux-specific PMU/sanitizer questions or final validation. D01 closes only
   after retained Apple and later Linux ARM64/Windows lanes reconcile.
5. **Linux ARM64 coverage.** Gate E still requires supported Linux ARM64
   correctness/timing evidence. Apple ARM64 is not a substitute. This may be a
   separate supported host/runner and is a validation lane, not the principal
   tuning machine.
6. **Same-machine Windows finish.** Freeze the Linux-selected candidate, boot
   the same x86-64 hardware into the supported Windows toolchain, and run the
   final dual-VM correctness, timing, lifecycle, RSS and artifact matrix. A
   Windows guard failure reopens the cross-platform decision; it does not
   authorize a Windows-only shortcut.

PERF2-12 remains deferred until the accepted non-JIT programme and final
cross-platform scorecard show it is economically necessary.

### Intel Linux handback — 2026-07-29

The initial Linux x86-64 campaign meets its explicit sufficiency gate. GCC and
Clang ordinary Debug/Release pass 1,925/1,925; supported GCC ASan/LSan and the
bounded exploratory Clang UBSan inventory also pass 1,925/1,925. Formal
timing/RSS/lifecycle/artifact evidence, full dual-VM GCC schema-5 profiles, a
bounded dual-VM Clang profile control, 154 non-multiplexed PMU captures and 16
focused cycle profiles are retained in
[`2026-07-28-perf2-10-11-intel-linux`](evidence/2026-07-28-perf2-10-11-intel-linux/).

### Windows baseline completion - 2026-07-29

The supported CLion MinGW Windows x86-64 Release product now passes
1,926/1,926 tests after correcting UTF-8 decoding in six CMake-script tests
and isolating nine driver/native tests from an inherited stale `CREXX_HOME`.
No compiler, VM, RXAS or language behavior changed.

The formal same-session Windows scorecard retains dual-VM timing, a governed
variability append, native child-process peak working set, 20-sample lifecycle
phases, artifact hashes/sizes and exact ooRexx 5.1.0, Regina 3.9.7, NetRexx
5.10-GA and Temurin 26.0.1+8 identities. The common-five geometric means are
2.363219x/1.984737x versus ooRexx and 0.749453x/0.629424x versus decimal
NetRexx for `rxvm`/`rxbvm`. Richards, Base64, Towers and the separately
disclosed RexxCPS row remain below ooRexx, so no final superiority or default
VM claim is made. Evidence:
[`2026-07-29-perf2-11-windows-x86-64`](evidence/2026-07-29-perf2-11-windows-x86-64/).

No profiling, tuning candidate or rebuild-heavy analysis was performed on the
Windows host. Gate E remains open for supported Linux ARM64 coverage and the
later whole-scorecard architecture decision.

### Windows compiler and RexxCPS follow-up - 2026-07-30

A bounded same-session Windows experiment compared MinGW GCC 15.2 and portable
Clang 22.1.8 `rxvm`/`rxbvm` products at Release `-O3 -DNDEBUG`, profiling off,
with identical GCC-generated RXBIN/library inputs. The common-five Clang/GCC
geometric means are 0.989609x for threaded `rxvm` and 1.178044x for switch
`rxbvm`; RexxCPS itself is 0.968556x/1.029856x. Compiler choice therefore does
not explain the 31-34% Windows/Linux cREXX RexxCPS rate difference.

The cross-platform review also ran the exact retained macOS/Linux Java 8
NetRexx class and identical runtime JAR on Windows. It reaches 19,032,852.5
clauses/s versus 7,980,203.5 in the retained Linux GCC session, excluding the
Windows ECJ versus Unix `javac` class-generation difference as the main cause.
The remaining comparator difference belongs to the Windows JVM/OS lane.

No production compiler, VM or optimization was selected and no Windows
profiling was performed. Clang's workload-specific `rxbvm` gains are a future
code-layout/dispatch lead for a faster profiling host; Linux remains the
tuning-control report. Evidence:
[`2026-07-30-perf2-11-windows-compiler-comparison`](evidence/2026-07-30-perf2-11-windows-compiler-comparison/).

A subsequent target-only MSVC control made `rxbvm` portable under MSVC 19.44
and reused the exact retained RXBIN/library inputs. In the stable cooldown
RexxCPS block MSVC is 1.148026x GCC and 1.134453x Clang, but only 0.648559x
ooRexx. Compiler choice therefore explains part of the Windows ratio without
closing the comparator or cross-platform gap. The initial post-build block was
machine-wide noisy, all samples remain retained, and no production MSVC lane
was selected. Evidence:
[`2026-07-30-perf2-11-windows-msvc-rxbvm`](evidence/2026-07-30-perf2-11-windows-msvc-rxbvm/).

A supplementary static-CRT (`/MT`) control then improved MSVC `rxbvm` by
4.9-6.0% over the DLL CRT (`/MD`) across three randomized blocks. The exact
clean block moved cREXX from 0.645731x to 0.677584x ooRexx and closed only 8.99%
of the absolute gap. `/MT` remains experimental pending plugin/API allocator
ownership validation.

No production optimization or compiler/layout option is selected. Richards
selects general value copying and attribute-storage trimming; Towers selects
copy/clear/reset/allocation work plus front-end/indirect-branch pressure;
Base64 selects the large dispatch function and string-copy path. Clang/GCC
direction reverses by workload, so a future compiler/layout decision requires
a paired same-session candidate experiment rather than comparing the two
formal lanes directly. RexxCPS is now present in the Clang schema, native-PMU
and cycle-sampling controls as well as formal timing; its samples add
front-end, decimal conversion/formatting and string-movement pressure.

The GCC profile build is the practical stop signal for iteration on this host:
two concurrent `rxvmintp.c` compiles caused a kernel OOM kill, while the serial
completion took 1:01:49 and peaked at 15.95 GiB RSS. Retain every build tree and
profile as an immutable asset. Next perform disposable mechanism ceilings and
candidate ranking on macOS, batch any selected production edit, apply the
mandatory first ordinary Release verdict there, and return to Linux only after
Adrian accepts that verdict.

## PERF2-01 — current baseline and attribution refresh

### Question

After NR-14 through NR-27, what actually consumes time and machine work in
each benchmark at current HEAD, and what is the exact same-session gap to
ooRexx?

Older profiles remain valuable historical controls, but the accepted parse,
stem, TRACE/ADDRESS, direct-call and flow changes are large enough that they
cannot rank the next production work.

### Capture plan

1. Freeze a clean exact commit and exact compiler, library, RXBIN and VM hashes.
2. Build an ordinary profiling-off Release product for timing authority and a
   separate optimized profiling build. Record compiler flags and confirm that
   `CREXX_VM_PROFILING=OFF` is real, not a runtime-disabled instrumented build.
3. Run all 11 optimized Tier A steady-state workloads in both `rxvm` and
   `rxbvm`, serially, with the existing correctness, warmup, recorded-sample,
   rotation, append and noise rules. Retain lifecycle and RSS as separate
   results.
4. In the same session, rerun all five qualified common cells for cREXX,
   ooRexx and decimal NetRexx. Capture canonical Classic RexxCPS for ooRexx,
   Regina and NetRexx, and report cREXX's disclosed 2.2d adaptation separately.
   RexxCPS never enters the common aggregate. Retain qualified lifecycle, RSS
   and artifact-size lanes separately, and record runtime versions rather than
   inheriting the July 20 labels.
5. Capture optimized diagnostic profiles for all 11 workloads in both VM
   modes. Use no-opt only as attribution: all 11 if affordable, otherwise at
   minimum RexxCPS, Bounce, Richards, Base64 and one already-winning control.
6. Capture RXSEQ N=2/3/4 from exact images and module sets in both VM modes.
   Treat straight-line windows as candidate evidence, not as loop- or
   semantic-unit truth; calls and taken branches terminate current windows.
7. Run native system sampling/counters on the uninstrumented product: cycles,
   retired instructions, branches/misses, instruction-cache/iTLB evidence and
   sampled/annotated hot stacks where the host supports them. Cover the full
   portfolio once and repeat the largest gaps and noisy hotspots.
8. Produce one dossier per workload and one cross-workload mechanism census.

### Required telemetry

Schema 4 already provides opcode, transition, procedure, call, frame,
allocation and RXSEQ evidence. The refresh must also provide deterministic,
counts-first attribution for the missing domains below. Extend profiling only
where existing data cannot answer the question, and keep instrumentation edits
separate from product optimization.

| Domain | Required observation |
| --- | --- |
| Values | copy, typed copy, move, clear, reset and destroy counts by payload shape and bytes |
| Representations | string/numeric/decimal conversion, materialization, normalization and retained-cache hit/miss counts |
| Calls/frames | fresh/reused frame, local reset work, argument/result copies, interrupt-state inheritance and numeric-context synchronization |
| Sites | static site identity, observed types/targets, cache hits/misses, generation and invalidation |
| Control | branch taken/fallthrough, loop-backedge counts and exceptional exits |
| Strings/binary | scan/slice/append/access counts and bytes, including native temporary conversion buffers where observable |
| Loader | link/bind, execution-image copy/preparation, plugin initialization, first execution and teardown phases |
| Compiler artifacts | static instructions, operands/cells, RXAS/RXBIN bytes, locals/register ceiling, inline sites and rejection reasons |

A counts-only profile mode is preferred for full-portfolio census. Per-opcode
clock reads and profile elapsed time remain diagnostic; ordinary Release timing
is authoritative.

The added fields require a versioned profiling-schema revision (schema 5 or an
explicit equivalent), stable row/field definitions, per-domain
overflow/degraded status, backward handling of schema 4, updated evidence
summarizers and profiling documentation, and focused CTest coverage. Prove that
the ordinary `CREXX_VM_PROFILING=OFF` generated path remains compile-time
empty. Counts-only output never becomes product timing evidence.

For Bounce, Richards, Storage and any selected allocation outlier, complement
VM request counters with targeted system heap/allocation profiles: alloc/free
counts, retained and high-water bytes, size classes, reuse and allocator call
stacks. Keep system lifetime evidence distinct from VM allocation-request
counters and RSS.

### RexxCPS family controls

Retain cREXX 2.2d's disclosed 100 × 100 adaptation as the published cREXX
diagnostic score; ooRexx, Regina and NetRexx retain the canonical Classic
external workload where qualified. Add exact-hash cREXX diagnostic variants
that remove or replace one timed family at a time—BIFs, internal
calls/argument parsing, TRACE/ADDRESS, stems, decimal/string loops and PARSE—to
estimate attributable ceilings. These controls must preserve nominal
clause-accounting provenance and never replace either published form.

### Deliverables and exit

- a checksum-closed evidence bundle;
- an updated
  [`benchmark-median-summary.md`](evidence/benchmark-median-summary.md) that
  includes the current checkpoint and same-session external run;
- a per-benchmark gap ledger with cREXX/ooRexx ratio and gain-to-target;
- top procedure/opcode/transition/native-stack tables;
- call/frame, copy/conversion, allocation/RSS, BIF and site-stability tables;
- an explicit mechanism footprint and owner decision for every candidate that
  enters Batch 2; and
- no degraded/overflowed profile accepted without a named limitation.

PERF2-01 completes only when Adrian accepts the refreshed ledger as the
selection baseline. It does not itself authorize a production change.

Gate A was accepted by Adrian on 2026-07-23. The accepted selection baseline is
`performance/evidence/2026-07-23-perf2-01-current-baseline/`; that acceptance
does not authorize a PERF2-02 implementation or any other production change.

## PERF2-02 — stable-site semantic quickening

Started and completed its bounded design/PoC exit on 2026-07-23. Adrian
approved the exact Q3b production slice, then accepted its favorable mandatory
first Release verdict on 2026-07-24. Broad QA and closeout are complete. The
resumable control plane is
[`PERF2-02-WORKLIST.md`](PERF2-02-WORKLIST.md), and the pre-implementation
semantic/design comparison is
[`PERF2-02-ARCHITECTURE.md`](PERF2-02-ARCHITECTURE.md). No stateful quickener
was selected; the accepted direct reference path retains no learned site state,
public format change or invalidation lifecycle.

### Bounded PoC result

The final identical-guard control makes runtime site state unnecessary. The
zero-state canonical-handler Q3b reduces Bounce elapsed time by
80.261%/78.503% (`rxvm`/`rxbvm`), beats eager Q4 by 7.584% in `rxvm`, and is
tied with it in `rxbvm`. Q7 is tied on Bounce, neutral on Richards, adds
56,264/62,536 requested state bytes and retains lifecycle gaps. The one
PERF2-02 recommendation is **direct value/reference helper work belongs first
in PERF2-07/PERF2-06**. Richards' separate Q1 control reduces elapsed by about
24% and assigns its removable receiver capture to the compiler/inliner.

The smallest proposed slice is the exact A-LOCAL/A-ATTR guard in canonical
`MKREF_REG_REG`, with no persistent state or public change. Adrian approved
that slice on 2026-07-24. The first production verdict retains 12 `rxvm` pairs
and 22 `rxbvm` pairs after the required noise append: paired elapsed medians are
-80.596%/-78.464%, every pair is favorable and both mean 95% intervals are
wholly favorable. Full Debug, ordinary Release and supported macOS ASan CTest
each pass 1907/1907, and the isolated installed tree passes native compilation
plus retained pre-change RXBIN execution in both VMs.

### Priority and scope

Quickening is the first architecture priority after PERF2-01. Here it means a
guarded private execution form that remembers a semantic decision stable at a
particular site. It does not mean merely joining adjacent opcodes or replacing
computed-goto dispatch.

cREXX has unusually good substrate for this work:

- canonical RXBIN remains immutable and re-linkable;
- both VM modes already own a process-local execution-image copy;
- stable direct calls already bind process-local `proc_runtime *` operands;
- graph/member/provider generations and existing method/factory site caches
  supply relevant invalidation experience; and
- late-load refresh, source metadata, profiling and two execution modes already
  provide the boundary conditions a quickener must respect.

### Candidate selection

PERF2-01 must identify the sites. Candidate families, in likely evaluation
order, are:

1. residual stable BIF or small-helper sites that cannot be removed statically,
   selected only after the relevant PERF2-03/04 cleaned-inline ceiling;
2. generic type/conversion operations with a strongly stable observed shape;
3. dynamic selector, member, factory or call sites not already closed by direct
   binding or the existing site caches;
4. validation or prepared-plan objects whose process representation is cheaper
   than repeating the semantic setup; and
5. profile-selected semantic sequences whose decisive fact is not known to the
   compiler, assembler or linker.

Indexed local-variable access, direct static calls and already-frozen PARSE do
not become quickening work merely because competitors cache them; cREXX has
already moved those facts earlier.

### PoC panel

For each candidate semantic unit compare the same exact workload and fallback:

| Variant | Placement | Purpose |
| --- | --- | --- |
| Q0 | current canonical path | baseline and full semantic fallback |
| Q1 | compiler-owned result-only lowering | machine ceiling when compiler proof makes intermediate temporaries unobservable |
| Q2 | assembler-visible static RXAS lowering/rule | public authored-sequence control with every observable intermediate effect |
| Q3 | canonical runtime-only bytecode form | test a stable portable VM form without automatically exposing authored RXAS syntax |
| Q4 | eager load/preparation specialization | cost/benefit when process facts are known before execution |
| Q5 | lazy first-hit specialization | avoid preparing cold sites and measure first-hit cost |
| Q6 | guarded threshold/tiered specialization | test whether observed stability/hotness justifies mutation |

Private quickened forms should be prototyped before assigning canonical opcode
numbers. Placement decisions distinguish a private form, compiler-owned
result-only lowering, a canonical runtime-only bytecode form and an
assembler-visible public RXAS instruction. Authored RXAS fusion must preserve
observable intermediate effects; a compiler-owned result-only form may omit an
intermediate temporary only when compiler proof makes it unobservable. Public
RXAS is considered only when authored assembly benefits and the static form
beats private specialization without losing compatibility.

### Architectural requirements

- Quickened state belongs to the process-local image or an explicit
  process-owned side table, never serialized handler pointers or mutated
  canonical RXBIN.
- The guard must encode the smallest fact that proves the fast path: type,
  target, generation, representation or context. A miss executes the complete
  semantic fallback and updates, replaces or disables the site according to a
  documented state machine.
- Preparation and mutation must be safe for the actual VM/process concurrency
  model; do not assume a single writer without proof.
- Dequickening or refresh must cover late load/provider generation change,
  dynamic mutation, TRACE/debug/source stepping, signals/unwind, plugin/native
  boundaries and any semantic context used by the specialization.
- Cold `prepare_only`/`rxvm_prepare()` must not learn execution-only facts.
  Quickened state must be proved valid or reset across a later `run()`, repeated
  runs on one context, embedded/RXVML entry and changes of TRACE, debug or
  profiling mode. Cover prepare-only, re-entry and late-load fixtures in both
  VM modes.
- `rxvm` quickening must preserve the computed-goto label-owner invariant:
  `run()` currently owns the threaded labels through `RXVM_LABEL_OWNER`
  (`noinline`/`noclone`). Keep that single stable owner or prove an equivalent,
  and test prepared handler addresses under every supported compiler/configuration.
- A fused private form must preserve every canonical exception, retirement,
  interrupt-poll, TRACE/breakpoint and profiler boundary. If equivalence at an
  intermediate boundary cannot be proved, dequick/de-fuse before execution;
  fusion must not silently reduce delivery or observation points.
- Profiling and RXSEQ must retain canonical opcode/site identity while also
  exposing quickened state, hits, misses, replacements and invalidations.
- `rxvm` and `rxbvm` must implement the same semantics. Different private
  layouts are allowed only when the evidence explains the difference.
- Startup, load-first-result, RSS, private-image size and cold-site preparation
  are measured alongside steady-state throughput.

### Adoption and exit

A quickened candidate advances only when:

1. PERF2-01 shows a repeated semantic cost and site stability;
2. the variant beats the current path in an ordinary profiling-off Release
   comparison; it also beats the best safe static form when one exists, or
   documents why no static form can consume the runtime fact;
3. the complete-product result is clearly favorable on a target workload or
   the common aggregate, with no unexplained portfolio guard;
4. invalidation, fallback, TRACE/source, signal, late-load and dual-VM fixtures
   pass; and
5. the code/image/RSS/startup trade-off is explicit.

The first PERF2-02 deliverable is an approved quickening design and a bounded
PoC panel, not a broad opcode family. The first production slice then follows
the mandatory first Release verdict and remains provisional until Adrian
accepts it. BIF/helper quickening additionally waits for the PERF2-03/04 cleaned
static ceiling so it cannot pre-empt the inlining-first policy.

## PERF2-03 — flow-aware inlining 2.0

Status: **complete** — closed 2026-07-25 after the accepted Slice 5 production
commit `d1c5245d4` and documentation closeout.

Adrian approved H and production slices 1-4 with QA and an independent commit
after each, then authorized slice 5 on 2026-07-24. Slice 1's receiver
transaction and slice 3's gated scalar/result cleanup are favorable; slice 2's
gate infrastructure is byte-identical parity. Slice 4 adds body-reconstructed
I6 callable summaries and opens the proved imported read-only scalar binding
case while retaining missing/old/contradictory evidence on the normal call
path. Its Richards program image is exactly slice-3-identical and its timing
verdict is neutral, so the accepted slice-3 gain remains cumulative without an
additive slice-4 claim. Slice 5 adds mathematically proved same-register
receiver placement and exact reference-attribute accessors while every
unproved alias, lifetime or cleanup case retains the existing materialized
path. Removing the 3,820,600 dynamic List `next()` calls reduces profiling-off
Release median elapsed by 52.818%/53.212% on `rxvm`/`rxbvm`; all other guarded
workloads and the linked library are byte-identical to the immediate baseline.
Final slice-5 Debug QA is 1,915/1,915. See `PERF2-03-WORKLIST.md` and the
retained `production/slice-5.md` evidence.

### Current evidence

Inlining still runs before the whole-program typed flow pass, so that pass sees
the expanded tree. Architecture H now supplies versioned local/imported
pre-inline summaries and a detached per-candidate profitability/fallback gate.
I6 readers reconstruct formal read/write/escape, result/context and cost facts
from the transported body, compare the result with the independently parsed
declaration, and fail closed on missing, old, malformed or contradictory
evidence. Candidate-local cleanup can coalesce the currently proved scalar and
result cases. Slice 5 also admits only fully proved direct local receiver
placement and exact reference-attribute getters/setters. Broader
reference/object ownership/last-use and block-expression equivalence remain
explicit fail-closed proof boundaries.

The retained NR-12/21 comparison found a small helper at 16 instructions after
inlining versus 13 in the hand-equivalent lowering, with two extra copies, one
extra branch/register and 412 extra RXAS bytes. A literal case remained nine
instructions versus three manually. This is concrete evidence that frame
removal and semantic inlining are not enough without cleanup.

### Stage A — current inline census and cost model

For every hot or size-significant inline site, record:

- callee identity, imported/local body, call arity and execution count;
- eligibility/rejection reason and structural node size;
- call versus inline versus hand-equivalent dynamic instructions;
- formal/default/result copies, branches, temporaries and initialization;
- maximum locals/registers, temporary footprint and call-window effect;
- RXAS, standalone RXBIN and linked-image bytes; and
- complete-product timing contribution where measurable.

The output is a ranked panel with explicit `inline`, `do not inline` and
`cleanup required` cases. Code size and register pressure are part of the cost
model, not after-the-fact caveats.

### Stage B — analysis architecture

Compare a small set of coherent designs:

1. lightweight pre-inline callee summaries for mutability, escape, return and
   block-result behavior, followed by the existing post-inline full analysis;
2. clone first, then extend NR-26 facts over formal bindings, block results,
   compiler temporaries and inline exits before final lowering; and
3. a bounded fixed point in which accepted cleanup exposes constants/dead
   paths, without repeatedly cloning or destabilizing source identity.

The design should make analysis facts explicit rather than matching one AST
shape. Handwritten RXAS remains RXAS's responsibility; no compiler-only
annotation is required for ordinary machine cleanup. Every structural rewrite
must either preserve a declared set of flow facts or invalidate and rebuild the
CFG/def-use overlay before another transform or final emission. Because current
inlining is destructive, make the profitability decision before irreversible
cloning or retain an untouched original call tree that can be emitted when the
cleaned inline loses.

### Initial transformation panel

- direct formal binding for proved read-only actual/formal pairs;
- dead formal/default initialization removal;
- constant propagation through formal bindings and inline block results;
- formal-to-result or return-result placement only with separate block-result
  equivalence, no harmful aliasing and exact return/cleanup ownership proof;
- dead inline-exit, block-result, branch and temporary removal;
- join-safe copy propagation and last-use moves where ownership is proved; and
- one final profitability check after cleanup, with a non-inline fallback when
  expansion still loses.

### Correctness boundaries

The fixture matrix must cover writable by-value isolation, `.ref`, optional
and default arguments, omitted/status arguments, repeated actuals, aliasing,
returns, joins, zero-trip loops, recursion, signals/unwind, inherited numeric
context, TRACE/source identity, imported inline metadata and optimized/no-opt
behavior. Register lifetime must be verified from the final typed instruction
stream.

### Exit

PERF2-03 completes when accepted inline fixtures contain no avoidable
formal/result copy, initialization, branch or compiler temporary; the output
approaches the hand-equivalent instruction/register/image footprint; a measured
profitability policy rejects losing sites; and a target workload confirms value
beyond static instruction reduction in the smallest decisive profiling-off
Release verdict. Report that verdict and stop for Adrian's acceptance before a
full-portfolio Release refresh.

That exit is satisfied. Architecture H supplies the detached fallback and
profitability decision, the accepted slices remove the proved receiver/formal/
result overhead, and Slice 5 supplies the decisive end-to-end target result.
The broad Debug closeout is green and the retained evidence is checksum-closed.
No full-portfolio Release refresh is required to close PERF2-03; later portfolio
campaigns apply the standing governance rules to the cumulative accepted
product.

### Successor proof ledger

These are future evidence routes, not incomplete PERF2-03 deliverables. Each
candidate site continues on the ordinary proven path until the named evidence
exists; no construct family is permanently excluded.

| ID | Future point | Evidence required to reopen | Route / disposition |
| --- | --- | --- | --- |
| PERF2-03-F01 | Residual scaffold around exact reference-attribute accessors, including the observed seven additional static general copies | A current profile must show that scaffold or image pressure is material after the accepted greater-than-52% List win, plus an exact direct-attribute semantic ceiling | Route reference/value ownership to PERF2-07; do not reopen solely to make the RXAS shorter. |
| PERF2-03-F02 | Broader reference/object ownership, escape and last-use placement | Per-site alias, lifetime, cleanup, unwind and observation proof with positive and negative CTests | Route to PERF2-07 or a later compiler-analysis successor selected by evidence. |
| PERF2-03-F03 | Remaining formal, block-result, inline-exit or temporary cleanup | A current hot candidate whose cleaned RXAS still loses materially to its hand-equivalent ceiling | Admit as a bounded companion to the selecting activity, especially PERF2-04; do not restart blanket inliner cleanup. |
| PERF2-03-F04 | Dynamic vararg indexing, generated association transport and assembler alias/effect facts | Exact locator/liveness/effect reconstruction and a measured multi-site deficit | Route compiler facts to a later analysis slice and semantic instruction work to PERF2-05. |
| PERF2-03-F05 | Future I6 summary fields or newly trusted body facts | Independent declaration/body reconstruction, exact producer/consumer comparison and a review-derived contradictory-evidence CTest | Standing requirement owned by the change that consumes the new fact. |

## PERF2-04 — inlining-first core Level B BIF campaign

Status: **complete — accepted ladder closed 2026-07-26**. The exact-current
census, semantic/machine panel and bounded
controls are complete from documentation-only closeout HEAD `6567f0ba2`, whose
exact accepted production parent is `d1c5245d4`. The resumable control plane
and full decision package are [`PERF2-04-WORKLIST.md`](PERF2-04-WORKLIST.md).
Older PERF2-01 profiles remain orientation evidence and do not replace the
current BIF ranking.

Adrian approved the compiler ladder on 2026-07-25: P04-CAS1 (`UPPER` through
general classified assembler-effect/read-only-exposed binding and constant
result), P04-SLC1 (general certified-call evaluation, initially `UPPER` and
proved-domain `SUBSTR`), the P04-CEX1 certificate expansion (`LENGTH`, `LEFT`,
`RIGHT`, `LOWER`), then P04-WRD1 (`WORD` certified constant evaluation plus
ordinary consumer folding). The original three timed families' combined
exact-site ceiling improves RexxCPS by 32.878%/34.400% and removes 46.52% of
normalized instructions. LEN-H1 is neutral; the Base64 algorithm result routes
to CAP-03.

P04-CAS1, P04-SLC1 and P04-CEX1 have favorable accepted first verdicts.
P04-WRD1's frozen provisional first ordinary Release verdict is also
favorable: the exact timed word path disappears, executable RXAS falls by 68,
and RexxCPS improves by 7.650234%/8.812155% on `rxvm`/`rxbvm`. All 18
invocations pass, relative MAD is 0.881617%/0.547875%, and neither cell requests
a rerun. Adrian accepted this verdict on 2026-07-26. The complete accepted
ladder is production commit `f8f34092e`; focused affected-surface QA is 24/24
and final broad Debug QA is 1,919/1,919. The retained bundle is checksum-closed.
No full formal portfolio or push was performed.

### Objective

Make the hot, bootstrap-safe Level B BIF surface execute as the simplest
semantic machine path while retaining the maintainable Level B source as the
complete fallback and documentation of behavior.

The component catalogue identifies a measured Level B bootstrap closure rather
than “make every function native.” PERF2-04's completed exact library
callable/module inventory is ranked by current product calls, phase and machine
work. Known RexxCPS timed controls are `LENGTH`, `SUBSTR` and `WORD`;
formatting BIFs outside its timed kernel are not RexxCPS causes.

### Per-BIF ladder

Every candidate moves through the same ladder:

1. **Clean source inline:** compile the current Level B body with PERF2-03
   cleanup and existing primitives.
2. **Hand-equivalent ceiling:** express the simplest known semantically correct
   lowering to quantify remaining scaffold/scan/copy cost.
3. **Best Level B algorithm:** remove repeated scans, materialization or other
   avoidable source-algorithm work while keeping the complete semantic body.
4. **Compiler lowering/composition:** use existing primitives when proved call,
   value, range or consumer facts can reach the machine ceiling.
5. **General assist control:** prototype one narrow RXAS/VM semantic kernel only
   if cleaned source cannot reach the ceiling.
6. **Native/intrinsic control:** use a direct runtime implementation to bound
   overhead, not as the automatic production answer.
7. **Placement decision:** choose Level B inline, compiler lowering, public
   RXAS assist or private quickening using the shared PERF2-02/05 gate.

### Seed panel

| BIF/family | First question | Possible assist only after cleanup evidence |
| --- | --- | --- |
| `LENGTH` | Can result initialization/copy scaffolding around existing `strlen` disappear completely? | None initially. |
| `SUBSTR`, `LEFT`, `RIGHT` | What remains after validation, optional/padding and result cleanup? | Non-mutating codepoint slice/span operation. |
| `WORD`, `WORDS`, `WORDPOS` | Is repeated scanning/cursor/slice setup dominant across workloads? | General word-span/count/extract plan or assist. |
| `POS` and related search | Does the current primitive already dominate, or does wrapper/setup remain? | General codepoint search only if reused. |
| typed conversion BIFs | Are representation crossings still material after NUMERIC-01? | Representation-preserving conversion path, preferably private until stable. |

The final panel is selected from current profiles, not frozen by this seed.

### Semantic and adoption gates

Preserve validation and signals, Unicode/codepoint behavior, 1-based indexing,
padding, optional/default/status semantics, numeric context, empty/boundary
cases, references and TRACE/source behavior. Test direct, imported inline,
unoptimized and both-VM forms.

A new assist advances only if it is general beyond one benchmark, occurs at
multiple static/product sites, beats the fully cleaned inline form, reduces
machine work, and is demonstrably better as public RXAS than a compiler-owned
combination or private quickened form. A BIF may complete with no new opcode.

## PERF2-05 — RXAS semantic assists and instruction improvement

Status: **complete — accepted production ladder closed 2026-07-26**.
Adrian accepted P05-CF1's favorable first ordinary Release verdict on
2026-07-26 because the body-driven design optimizes ordinary user-written and
future functions without a BIF registry. `LENGTH`, `SUBSTR` and `WORD` all
reach their constant ceilings from unregistered Level B bodies; existing
PERF2-04 selected output remains RXAS-identical. The completed slice adds
bounded body evaluation plus exact opcode cursor/evaluator metadata without a
public RXAS/RXBIN/ABI or VM-semantic change. Review-derived scalar-result,
callable-scope and procedure-level `EXPOSE` fences are installed; final Debug
QA is 1,920/1,920, focused Release QA is 6/6 plus metadata, and the final
generated decision cell is byte-identical to the accepted timed candidate.
Closeout evidence is retained under
[`2026-07-26-perf2-05-generic-partial-evaluation/`](evidence/2026-07-26-perf2-05-generic-partial-evaluation/).
The resumable control plane is
[`PERF2-05-WORKLIST.md`](PERF2-05-WORKLIST.md). Adrian approved the bounded
order on 2026-07-26: prove generic evaluation with `LENGTH`, prove useful
composition with `SUBSTR`, audit and lock cursor/effect contracts, then require
certified and equivalent user-written `WORD` bodies as the acceptance case.
The design objective is that user-written and future functions receive the
optimization without adding BIF-specific evaluator cases. The existing
PERF2-04 evaluator remains the comparison oracle until equivalence is proved;
no public RXAS/RXBIN or legacy cursor-semantic change is selected by this
approval.

The post-P05 P05-SA1 refresh is retained under
[`2026-07-26-perf2-05-semantic-assist-panel/`](evidence/2026-07-26-perf2-05-semantic-assist-panel/).
It confirms that the List benchmark's weak/arena-owned references permit a
narrow **descriptor** materialization without materializing or owning the
target object. The direct descriptor-copy ceiling improves the selected List
cell by 6.173%/6.154% on `rxvm`/`rxbvm`; exact relink independently improves it
by 2.253%/1.623%. Both pass focused dual-VM semantics and exact instruction
reduction. Their combined ceiling is favorable, but the recommended production
rungs remain separate. The scalar `ICOPY; BR` control is neutral because its
timing is noisy and VM-dependent.

The placement recommendation was compiler/RXAS proof for eligibility, TRACE
and fallback, with private execution only where needed and canonical RXBIN
unchanged. At that selection stop no public opcode or production edit had been
selected; Adrian's subsequent decision is recorded below.

Adrian accepted recommendations 1-4 on 2026-07-26: R2a first, R1a second as a
separate rung, exact canonical-sequence eligibility with private execution, and
no public RXAS/RXBIN change. The mandatory R2a first Release verdict is
favorable: work-100 List improves by a paired median 2.731%/1.745% on
`rxvm`/`rxbvm`, with wholly favorable mean 95% intervals and no absolute-cell
noise rerun. Focused dual-VM and compiler/import/no-opt correctness pass.
R2b is deferred because both the scratch ceiling and R2a retain canonical
`copy_value`; the remaining ceiling gap does not isolate payload-copy cost.
Evidence is retained under
[`2026-07-26-perf2-05-r2a-first-release-verdict/`](evidence/2026-07-26-perf2-05-r2a-first-release-verdict/).
Adrian accepted R2a on 2026-07-26. Its full Debug and ordinary profiling-off
Release products build, and both broad CTest configurations pass 1,922/1,922.
R2a is closed green. R2b remains deferred because it lacks separate cost
attribution and shape/lifetime proof; it was not selected or implemented. At
that closeout, R1a remained the next independently revertable rung and had not
started, so the broader PERF2-05 activity remained in progress.

Adrian authorized the separate R1a production slice on 2026-07-26. It targets
only exact adjacent `UNLINK destination; LINKREF destination,source` shapes,
preserves unlink-before-validation failure state, and keeps public RXAS plus
canonical RXBIN unchanged through a process-local private execution form. The
accepted R2a ordinary Release product is preserved as the comparison baseline.
R1a must stop after focused correctness and its own smallest decisive first
Release verdict; broad QA, closeout, R2b and B1 remain outside that gate.

The corrected R1a product passes 12/12 focused core/reference/TRACE checks,
49/49 compiler/import/optimized/no-opt checks and its fresh Release dual-VM
guard 2/2. Its exact-input List verdict reached the governed 36-pair cap:
`rxvm` is `-1.151991%` paired median with 32/36 favorable pairs and a
`[-2.337441%, -0.187062%]` mean 95% interval; `rxbvm` is `-3.022743%` with
36/36 favorable and a `[-3.204016%, -2.814217%]` interval. Both are clear
favorable, so the recommendation is to accept R1a for closeout. Evidence is
retained under
`performance/evidence/2026-07-26-perf2-05-r1a-first-release-verdict/`.
Adrian accepted R1a on 2026-07-26. The full Debug and ordinary profiling-off
Release products rebuild, the 12-test core/reference/TRACE and 49-test
compiler/import/optimized/no-opt sets pass in each configuration, and broad
CTest passes 1,924/1,924 in both configurations. R1a is closed green. In line
with the bounded closeout path, sanitizer, install/package, cross-platform,
expanded-portfolio and repeated-baseline work were not added.

Accepted P05-CF1, R2a and R1a complete PERF2-05. The final production result
uses compiler composition plus exact private execution-image forms; it adds no
public RXAS instruction and changes no canonical RXBIN or ABI contract. The
neutral and deferred opportunities remain explicitly governed below and do
not keep the activity open.

| ID | Future point | Evidence required to reopen | Route / disposition |
| --- | --- | --- | --- |
| PERF2-05-F01 | R2b payload-only reference-descriptor copy | A fresh post-R2a profile must attribute material residual cost to canonical `copy_value`, followed by mechanical destination-cleanup, cell retain/release, identity, representation and invalidation proof | Route ownership/value work to PERF2-07; do not infer benefit from the older public ceiling gap. |
| PERF2-05-F02 | Neutral `ICOPY; BR`/B1 result forwarding | A compiler-owned result-forwarding PoC must be mathematically equivalent, reduce work and show stable benefit on multiple workloads in both VMs | Route to the selecting compiler/flow activity; do not reopen as frequency-only fusion. |

Final R1a evidence is retained under
[`2026-07-26-perf2-05-r1a-first-release-verdict/`](evidence/2026-07-26-perf2-05-r1a-first-release-verdict/).

### Starting point

NR-09 and NR-18/27 established the machinery: arbitrary operand signatures,
opcode effects, compiler exact-template combination, RXAS local and
whole-procedure flow, dual-VM support and RXSEQ evidence. The broad NR-09 batch
also showed the risk: many legal wide forms were neutral or were withdrawn,
while the accepted complete product gained only about 1.4%/2.9% RexxCPS.

The next RXAS programme therefore targets **semantic units**, not operand
count or mnemonic volume.

### Candidate generation and placement

1. Refresh N=2/3/4 RXSEQ across all current images and both VMs.
2. Rank sequences using dynamic executions, static sites, distinct modules,
   retired-instruction reduction and native profile footprint. Do not add
   overlapping sequence counts as though they were independent savings.
3. Prove effects, liveness, alias/reference behavior, intermediate-write
   observability, signal/throw order, TRACE anchors and register pressure.
4. Compare distinct owners/forms: compiler-owned result-only lowering, RXAS
   effect-clean authored rule, canonical runtime-only bytecode, private
   quickened form and assembler-visible public RXAS instruction.
5. Prototype candidates as a bounded panel with exact mathematical correctness
   and instruction-reduction gates before formal timing.

Likely sources are PERF2-04 string/word kernels, PERF2-03 result placement,
PERF2-07 payload-aware copy/conversion work and PERF2-02 stable private forms.
Legacy `FDIVSUB`, `ILOADSETUNLINKN` and deferred propagation ideas remain
inspection candidates only when the new profile selects them.

### Public-form gate

A canonical instruction must:

- have a coherent, documented semantic contract independent of one compiler
  template;
- preserve all intermediate effects that language/RXAS authors can observe
  when the form is assembler-visible; compiler-owned result-only and
  runtime-only forms require their separately proved contract;
- reduce exact machine work in representative linked products with no
  instruction growth elsewhere;
- win ordinary Release time clearly in the selected/default VM and remain
  within accepted regression guards in the other, without unacceptable image,
  handler or instruction-cache growth;
- have complete mechanical opcode effects, assembler/linker/disassembler
  round-trip where public, profiling/RXSEQ visibility and an explicit
  public-source versus runtime-only classification;
- retain old-RXBIN execution and new-feature gating as required; and
- receive Adrian's explicit ISA/RXBIN approval before production assignment.

Otherwise retain the optimization in the earliest private layer that owns the
facts, or reject it with evidence.

## PERF2-06 — VM execution-engine programme

Activity start (2026-07-26): exact `develop` HEAD `e7090198e` was verified six
commits ahead of `origin/develop` with a clean worktree. The five accepted
PERF2-01/02/05 evidence manifests actually used replayed successfully. The
resumable control plane is [`PERF2-06-WORKLIST.md`](PERF2-06-WORKLIST.md).
Production installation remains blocked until the complete attribution and
bounded PoC panel is presented to Adrian.

First mandatory stop (2026-07-26): the complete package is retained at
[`evidence/2026-07-26-perf2-06-vm-audit/`](evidence/2026-07-26-perf2-06-vm-audit/).
Exact current profiles show 432,950/572,500 optimized Permute/List bytecode
calls but only 7/44 fresh frames; allocation is already amortized, while each
child call still copies the 1,280-byte interrupt table and recycled entry
relinks 9,957,712/13,312,071 local pointers. Native List samples put the table
`memmove` at 7.4% in both VMs. The bounded COW control removes it and passes
65/65 focused tests, but its repeated cold-failure shape grows `run()` by
10,700/6,268 bytes and is clearly adverse on Base64 and Sieve `rxbvm`; that
exact patch is rejected. The recommendation is a call/frame direction:
VM-C1b centralizes shared interrupt-policy mutation off the hot path, while
Adrian's VM-C2 non-moving segmented value arena plus compact control stack is a
first-class broader architecture PoC whose decisive issue is eliminating
pointer-map relinks without taxing every operand. No production edit, broad
closeout, commit or push is authorized before Adrian selects the direction.

Approval (2026-07-26): Adrian selected the recommended VM-C1b-first sequence.
The production slice may centralize shared/COW interrupt-policy mutation and
remove eager child-table copying, then must stop at the mandatory first
profiling-off Release verdict. The retained COW PoC shape remains rejected;
VM-C2, broad QA, closeout, commit and push are not authorized by this approval.

First Release verdict and acceptance (2026-07-26): the checksum-closed package is retained at
[`evidence/2026-07-26-perf2-06-vm-c1b-first-release-verdict/`](evidence/2026-07-26-perf2-06-vm-c1b-first-release-verdict/).
VM-C1b removes the 1,280-byte child copy, reduces `stack_frame` by 1,264 bytes
and shrinks `run()` by 5,660/796 bytes in `rxvm`/`rxbvm`.  Twelve balanced
pairs are clear favorable for List `rxvm` and both Permute modes, but Sieve
`rxbvm` is clear adverse: paired mean `+5.368694%`, 95% interval
`[+4.720473%, +6.016915%]`, 0/12 favorable. Adrian explicitly accepted that
trade-off after diagnosis because the call-heavy gains and faster `rxvm` Sieve
justify retaining the slice. Full Debug and ordinary profiling-off Release QA
both pass 1,924/1,924. `PERF2-06-D01` retains the unresolved Apple-Clang global
code-layout/register-allocation debt and requires a supported Intel x86-64
GCC/Clang matrix before final architecture selection. The implementation
commit is `a39608426e2c1bb84d5fc0c4f767f4c9492339a9`. VM-C2 is authorized as the
next separate clean-base PoC; no push is authorized.

VM-C2 first PoC result and reset follow-on (2026-07-26/27): C2-A's segmented
allocator retained the full pointer map and did not win; C2-B removed
97.31-100% of reuse relink stores but replaced them with 8.02/8.93 million
mapping-mark attempts on List/Permute and 2.11 million attempts on Sieve despite
zero frame reuse. C2-B is rejected: no future variant may add bookkeeping to
ordinary mapping writes or operand reads. Adrian approved the bounded
`PERF2-06-C2R01` follow-on: compare one fixed-core reset for locals/globals,
with `a0` still restored and reinitialized separately and `a1...aN` remaining
a call-bound variable tail, then a conservative
preparation-time per-procedure `may_rebind_core` flag. `CALL1...CALL4` are the
future embedded-argument fast path. Exact static reset lists and new quickened
link/use/unlink forms were deferred pending this verdict. The resumable
isolated control plane is
[`PERF2-06-VM-C2-RESET-WORKLIST.md`](PERF2-06-VM-C2-RESET-WORKLIST.md).

VM-C2 reset verdict (2026-07-27): the retained package is
[`evidence/2026-07-27-perf2-06-vm-c2-reset-poc/`](evidence/2026-07-27-perf2-06-vm-c2-reset-poc/).
R1 replaces the fixed local/global reset loops with one copy; R2 guards that
copy with a canonical, fail-closed procedure-static proof. Both pass 143/143
focused tests per variant and preserve exact instruction/call/branch/value
operation rows. R2 skips every Base64 reuse reset, no Permute reset and only
3,099/572,457 List reuse resets. Thirty-four balanced pairs reject both
variants: List is adverse in both VMs, and `rxbvm` Sieve is adverse by
`+1.801135%` for R1 and `+2.284914%` for R2 despite zero frame reuse. The
R1 control has no classifier and therefore exposes native
code-layout/register-allocation sensitivity, not residual reset cost. R2 scans
55,104 Sieve instructions at startup, so its result combines possible
classifier and layout effects, but likewise cannot be reset work. The R1
absolute cell is span-flagged while its paired interval remains wholly adverse.
This strengthens `PERF2-06-D01` and its supported Intel x86-64 GCC/Clang
requirement. The isolated reset implementation was discarded and the earlier
C2 source was restored after the final review; retained patches and evidence
remain the result authority. Exact reset lists and quickened clearing do not
advance from this result.

Final tactical review (2026-07-27): after restoring every rejected C2 source
change, the accepted frame implementation was reviewed once more for a bounded
current-design win. Allocation is already amortized, mapping reset alternatives
are exhausted by the C2 controls, and reference-lifetime release is already
guarded by `has_reference_lifetimes`. The only remaining measured local
candidate was C3R01: rebind the decimal plugin's context pointer but omit the
redundant child-entry synchronization after an exact context copy, then
synchronize on return only when the effective context or backend differs.
Twelve balanced profiling-off Release pairs were all inconclusive. Means were
`-0.384474%`/`-0.204769%` for Permute, `-0.835582%`/`+0.613827%` for List and
`+1.440812%`/`+0.936526%` for Sieve in `rxvm`/`rxbvm`. Optimized Sieve performs
zero bytecode calls, so its adverse movement cannot be numeric-context work and
again exposes global native-code layout sensitivity. C3R01 was discarded; its
compact raw record is retained at
[`evidence/2026-07-27-perf2-06-vm-c3-tactical-rejection/`](evidence/2026-07-27-perf2-06-vm-c3-tactical-rejection/).

Do not repeat C2-A/B, C2R01, an exact reset list, quickened clearing, C3R01 or
cleanup-only reshaping of the flattened interpreter on the current Apple
product. Reopening requires a materially different ownership/representation
contract or cross-platform counter evidence that identifies a stable mechanism
and includes zero-work drift controls. Source tidiness or smaller text alone is
not evidence of a faster VM.

Mac continuation boundary (2026-07-27): PERF2-06 no longer runs as a separate
Apple tuning campaign. Its relevant value/reference-helper ownership,
payload/frame coupling and future-hardware manifest work join PERF2-07 in
[`PERF2-06-07-WORKLIST.md`](PERF2-06-07-WORKLIST.md). `PERF2-06-D01`, native
compact/hot-cold stream selection and the final VM recommendation stay open for
the Intel/Linux, Linux ARM64 and Windows matrix. C2R03 may enter the combined
panel only if current payload-capacity/high-water evidence creates a materially
different mechanism; its earlier pointer-map and reset forms remain rejected.

Combined selection panel (2026-07-27): the fresh dual-VM Apple attribution
selects compiler/inliner ownership before another VM/frame intervention.
Optimized Richards performs 73,307,574 recursive copies and moves 582,076,729
copy bytes per profiled run in both VMs; optimized Permute performs
10,259,602/74,012,810. Native stacks and retired-instruction controls agree
that `copy_value` is the Richards mechanism. Adrian selected
`PERF2-06-07-V1R01`. Its rejected first form accidentally disabled the accepted
ordinary receiver path in Bounce; that exact +1,000,000-copy failure remains
retained. Adrian then approved a proof-wide rework. V1R01-R1 restores the
ordinary path and directly places nested `§this` through arbitrary branches and
calls when the reconstructed result has at most one explicit return, including
fallthrough. Already-proved receiver aliases survive later inline cloning;
multiple explicit returns retain materialisation because per-exit receiver-link
balance is not yet proved. Exact Bounce work returns to status quo. At the
36-pair cap Richards is -21.224%/-21.076%, Permute -58.019%/-56.466%, common
aggregates are 1.244352x/1.242301x and no workload, aggregate or artifact-size
guard hits. Adrian accepted the candidate. Proportional closeout passes broad
Debug/Release 1,925/1,925, focused ASan 10/10, lifecycle/RSS, 12/12 retained
RXBIN cells and the isolated install; Apple LSan is unsupported and recorded as
such. C2R03 still fails its current payload-capacity/high-water entrance gate,
and V6 remains a supported-platform architecture decision. Panel evidence:
[`2026-07-27-perf2-06-07-selection-panel`](evidence/2026-07-27-perf2-06-07-selection-panel/).
Rejected first-form evidence:
[`2026-07-27-perf2-06-07-v1r01-first-release-verdict`](evidence/2026-07-27-perf2-06-07-v1r01-first-release-verdict/).
Reworked first Release evidence:
[`2026-07-27-perf2-06-07-v1r01-r1-first-release-verdict`](evidence/2026-07-27-perf2-06-07-v1r01-r1-first-release-verdict/).
Accepted Apple closeout:
[`2026-07-27-perf2-06-07-v1r01-r1-closeout`](evidence/2026-07-27-perf2-06-07-v1r01-r1-closeout/).

| Stable ID | Hypothesis and surface | Semantic/evidence gate | Disposition |
| --- | --- | --- | --- |
| PERF2-06-C2R01 | One contiguous fixed-core reset, then a procedure-static `may_rebind_core` flag, can reduce recycled frame-entry work without touching mapping writes/reads; arguments remain a separately rebound tail. | Exact coverage of `LOAD`/`SWAP`/`LINK*`/private mapping effects, recursion, fixed/count calls, refs, signals, TRACE, late load, both VMs, Release time and text. | rejected: correct but adverse and code-layout-sensitive at 34 pairs |
| PERF2-06-C2R02 | Private quickening may remove exact nonescaping `LINK/LINKATTR; use; UNLINK` mappings rather than accelerating their cleanup. | No activation pointer/state in the shared image; exact debug/signal/reference/failure/resume fallback and independent code-layout verdict. | deferred; C2R01 provides no reason to advance clearing work |
| PERF2-06-C2R03 | A linear segmented control stack can point to procedure-affine non-moving value slabs, preserving same-procedure/register payload capacity without retaining the full frame/control design. | Compare strict LIFO value slices; exact reference/native/object/decimal teardown and signal unwind; context/thread ownership; high-water memory; fixed-call embedded arguments; no hot per-register transfer ledger. | post-verdict architecture candidate; control/mapping split remains PERF2-06, payload-capacity policy routes to PERF2-07; analysis only, not approved |
| PERF2-06-C3R01 | Skip decimal-backend synchronization when child entry copies the already-active effective numeric context; on return always rebind the pointer and synchronize only for a changed context/backend. | Exact five-field context equality and plugin ownership; decimal modes and nested context changes; both VMs; call-heavy Release cells plus zero-call Sieve drift control. | rejected: all 12-pair intervals inconclusive; Sieve moved adversely despite zero calls, so no stable mechanism beat code-layout noise |

### Objective

Determine and remove the remaining VM-owned cost after compiler, BIF and RXAS
work are attributed. This includes private instruction representation,
dispatch/fetch, calls and frames, values, context/interrupt maintenance,
runtime helpers, code layout and preparation. It is not a mandate to rewrite
the dispatcher.

### Current substrate and constraints

- `rxvm` uses computed-goto dispatch; `rxbvm` uses switch dispatch.
- Both modes now prepare an owned process-local execution image. `rxvm` stores
  handler cells and both modes can hold process-local bound function operands.
- Canonical RXBIN code cells remain immutable and portable.
- Each opcode and operand in the current runtime image occupies an eight-byte
  cell; wide forms can reduce dispatch while increasing fetch/image footprint.
- The VM polls for interrupts at every retired instruction and preserves
  signals, TRACE, numeric context, references, native/plugin calls and late
  load. These are language/runtime semantics, not optional benchmark overhead.

Documentation that still says `rxbvm` executes only the canonical stream must
be reconciled with current code as part of the activity.

### VM-A — native current-HEAD attribution

PERF2-01 must identify, per workload and per VM:

- top C handlers/helpers and their caller stacks;
- retired native instructions, branch/mispredict and instruction-cache costs;
- dispatch versus handler/helper/body proportion;
- runtime-image bytes/cells touched and hot code/text footprint;
- call/frame entry and return subphase costs;
- value copy/move/clear/conversion bytes; and
- interrupt, TRACE, numeric-context, allocation and loader/preparation costs.

Only these results rank the following PoCs.

### VM-B — execution stream and fetch layout

Compare, without changing canonical RXBIN:

1. current wide-cell private image;
2. a compact switch-oriented stream or compact operand overlay;
3. decoded hot/cold forms that keep rare metadata away from the hot fetch path;
4. PERF2-02 quickened private forms; and
5. only RXSEQ-selected fused semantic units.

Measure decode/preparation time, steady-state cycles, branch and i-cache data,
private-image/RSS size, source/profile mapping and both VM modes. A compact
stream is adopted only with representative portfolio evidence on at least two
architectures; code density alone is not a speed result.

### VM-C — residual call and frame path

Run this after PERF2-03/04 removes avoidable small calls. Frame recycling
already exists, but remaining bytecode calls reset local mappings, inherit the
interrupt table, copy/synchronize numeric context and later restore context.
Higher-arity fixed calls are not assumed: refresh their dynamic callee/cost
population after inlining and quickening, and distinguish product use from the
deliberately high-arity JSON fallback control.

Profile and compare:

- shared or copy-on-write inherited interrupt state with a per-frame overlay;
- numeric-context activation only when the effective context changes;
- proved-leaf/lightweight frame activation;
- argument/result placement coordinated with compiler flow; and
- targeted frame pooling or reset specialization by frame shape.

Gates include recursion, repeated calls, all defined signal codes,
reserved-code bounds and the `RXSIGNAL_MAX` sentinel, handler push/pop,
branch-handler ownership, signal call/unwind, writable inputs,
references/aliases, plugin/native calls, decimal-plugin modes, TRACE and late
load. Measure the inherited 32-entry table copy separately.

### VM-D — interrupt, TRACE and cold path separation

Do not remove per-instruction interrupt semantics. First measure individual
poll components and hot/cold code layout. A split synchronous/asynchronous poll
or cold outline is a design candidate only if it preserves observable delivery
and handler behavior, proves the remaining hot test, and wins outside a single
microbenchmark.

Inactive TRACE should remain near-zero-cost, but source coordinates and
debug/profiler identity must survive quickening/fusion. Signals, unwind and
late-load repair paths may be cold-outlined only after complete coverage.

### VM-E — cross-platform dispatch completion

The previous dispatch investigation remains useful negative evidence but did
not complete native Linux x86-64 counters and Windows x86-64 timing. Re-run the
current product on:

- Apple ARM64 with Apple clang and native counters/sampling available on the
  host;
- Linux ARM64 release-build/correctness/timing coverage, with native counters
  where available;
- Linux x86-64 with native branch/cache/perf evidence under supported GCC and
  Clang versions where available; and
- Windows x86-64 timing and code/artifact evidence under the supported Windows
  toolchain.

Preserve early next-target resolution and compare the actual modern
execution-image implementation, not stale pre-NR-16/17 prose. Record exact
compiler versions, `run()` text size and branch/instruction-cache counters;
dispatch layout is compiler-dependent.

### VM-F — lifecycle and preparation

Establish the CAP-04 load-only boundary before optimizing it. Attribute
canonical image verification/copy, semantic graph rebuild, function binding,
quickening preparation, plugin initialization, first frame and teardown.
Current load-first-result is around 2.7 ms on the July 23 host, so this remains
below the large steady-state deficits unless refreshed lifecycle or embedded
use evidence changes the ranking.

### Explicit non-candidates unless new evidence overturns them

| Prior idea | Current disposition |
| --- | --- |
| Remove interrupt polling | Rejected as a semantic break; the measured ceiling was only about 1–5%. |
| Lockstep dispatch cursor | Rejected after prior 15–30% regressions. |
| Force globally separate computed-goto dispatch sites | Deprioritized: mixed results with substantial code growth. |
| Serialize handler pointers or mutate canonical RXBIN | Rejected by portability, process ownership and compatibility. |
| Replace the dispatcher as the first-order answer | Unsupported by competitor evidence and current architecture. |
| Add broad superinstruction families from raw RXSEQ counts | Rejected without effects, overlap, footprint and complete-product proof. |
| Remove `RX_FLATTEN` | Rejected: it reduced text but was neutral/slower. |
| Mark the interrupt path globally `unlikely` | Rejected after severe Apple-clang layout regressions. |
| Retry fixed-core reset, reset-needed flags, exact reset lists or quickened clearing | Rejected by C2R01 and its zero-reuse Sieve control; no retry without a materially different mapping contract and new profile selection. |
| Changed-only numeric/plugin synchronization in the current frame model | Rejected as C3R01: call-heavy movement was inconclusive and zero-call Sieve moved adversely. |
| Cleanup-only reshaping of the flattened interpreter | Not a performance candidate: repeated zero-work controls demonstrate global compiler/layout sensitivity. Require an independently measured mechanism and drift controls. |

Opcode-indexed/switch dispatch remains a safe portable comparison and fallback,
not a rejected design.

### Exit

PERF2-06 completes with accepted/rejected VM PoCs, a cross-platform
recommendation, current documentation, dual-VM correctness and ordinary
Release evidence that any selected VM change improves real product workloads
without hiding startup, RSS, image or compatibility costs. PERF2-11 Gate E
owns the final default/private execution-architecture decision.

## PERF2-07 — value, frame, representation and allocation work

The Mac execution of PERF2-07 is combined with the still-relevant PERF2-06
value/reference/VM ownership boundary. The approved plan is
[`PERF2-06-07-WORKLIST.md`](PERF2-06-07-WORKLIST.md); its completed execution
prompt is preserved in
[`PERF2-06-07-HANDOVER-PROMPT.md`](PERF2-06-07-HANDOVER-PROMPT.md), and the
successor hardware protocol is in the accepted closeout evidence. This does
not pull `PERF2-06-D01` or final stream/default selection onto the Mac; those
remain in the hardware handover matrix.

### Question

Which parts of cREXX's general `value` and frame semantics still create repeated
copy, conversion, reset, cache or allocation work, and what is the smallest
safe intervention?

The current `value` is approximately 248 bytes and can carry scalar, decimal,
string, binary/native, reference, type and object-attribute state. Whole-value
copy can recursively duplicate populated representations/attributes, while
move transfers ownership after clearing the destination. Those facts make
both avoidable copies and a premature global layout rewrite risky.

### Required order

1. Verify current sizes/layouts on each target ABI; do not inherit historical
   Linux or pre-NR-15 profiles.
2. Count operations and bytes by payload shape, caller, procedure and
   benchmark, including conversions/materializations and cache hits.
3. Use compiler/RXAS flow first to eliminate the operation or choose a typed
   copy/move when ownership is proved.
4. Compare payload-shape fast paths and retained representation validity.
5. Only then test frame/value pooling or hot/cold representation changes.

### Candidate ladder

| Level | Candidate | Principal risk |
| --- | --- | --- |
| V1 | eliminate dead/full copies; direct result placement; typed copy/move | aliases, hidden payload release, join/exception ownership |
| V2 | payload-shape fast copy/clear/reset | stale type/attribute/native state |
| V3 | retain validated string/numeric/decimal representations | mutation invalidation, numeric-context semantics, memory growth |
| V4 | share inherited frame state and reset only live slots | signal/context ownership and recursion |
| V5 | size/shape-targeted frame/value pools | teardown, plugins, sanitizer visibility, memory retention |
| V6 | hot/cold `value` split | ABI/layout breadth, cache trade-off, widespread code complexity |

### Proved entry cases

| ID | Evidence and exact failure | Required distinguishing regression | Boundary |
| --- | --- | --- | --- |
| PERF2-07-V3-R01 | PERF2-04's valid Level B sequence initializes a string to `""`, initializes a decimal from `"2.2"`, assigns that decimal to the same string using `as .string`, then executes `strlen`. Both VMs and both optimization modes preserved `2.2` but returned stale codepoint length `0`. `DCOPY` retained the old trusted zero count and `DTOS` changed bytes/byte length without completing VM-private UTF validity. | The maintained regression preserves `dcopy; dtos; strlen` and covers empty, non-empty Unicode, typed-null, live reference alias, numeric siblings and `DEXTR` on optimized/no-opt `rxvm` and `rxbvm`. All four retained reproducer cells and the related focused group pass. | Correctness fixed with one explicit in-place string-write completion contract; no performance claim, language, public RXAS/RXBIN or ABI change. Broad selective cache retention remains deferred because the current mechanism count is zero. Evidence: [`CORRECTNESS.md`](evidence/2026-07-27-perf2-06-07-selection-panel/CORRECTNESS.md). |

Mutable object/string copy-on-write and a general allocator replacement do not
enter the first panel. They require explicit alias/reference proof and current
evidence that narrower work cannot close the cost.

### Exit

Each accepted slice must reduce exact operations/bytes or demonstrated native
cost, pass sanitizer and ownership/lifecycle fixtures proportional to the
changed ownership surface, and improve a target
workload without breaching the common-portfolio guard. PERF2-07 closes only
after all high-cost shapes have an accepted optimization or an evidence-backed
defer/reject decision. Pooling decisions additionally require the targeted
alloc/free lifetime, retained/high-water, size-class, reuse and allocator-stack
evidence defined by PERF2-01; request counts and RSS alone are insufficient.

Apple closeout (2026-07-27): Adrian accepted V1R01-R1. The prerequisite V3-R01
fix and proof-wide compiler placement are installed; all other panel rows retain
their explicit defer/reject/architecture-owner dispositions. The accepted Mac
slice passes broad Debug and profiling-off Release 1,925/1,925, focused ASan
10/10, lifecycle/RSS guards, retained-RXBIN compatibility 12/12 and isolated
install smoke in both VMs. The bundle is
[`2026-07-27-perf2-06-07-v1r01-r1-closeout`](evidence/2026-07-27-perf2-06-07-v1r01-r1-closeout/).
This is not a cross-platform claim and does not close `PERF2-06-D01`, Gate E or
the final VM/default decision.

## PERF2-08 — capability, equivalence and Level B/G decision lane

Performance comparisons have exposed missing capabilities as well as slow
paths. These require separate treatment so a language design is neither
smuggled into an optimization nor blocked by an optimizer-only worklist.

### Existing capability gaps

| Gap | Performance relevance | Default route | Decision boundary |
| --- | --- | --- | --- |
| CAP-01 JSON parse-once/indexed document | Current JSON cells build/use different result models. | Library/runtime handle or indexed object model first. | Language change only if the library/runtime surface cannot express ownership/use safely. |
| CAP-02 owned heterogeneous/nested containers | cREXX Storage performs materially different allocation/object work. | Specify ownership, nested references and lifecycle; build an equivalent control. Post-Release 1 Level G is the default source-language route. | A Level B source surface requires a separate explicit scope decision after proving a library/runtime solution insufficient; Adrian approval required. |
| CAP-03 standard Base64 surface | Base64 is a valid common cell but lacks one standard portable cREXX product API. | Keep the current common codec-loop benchmark unchanged. Develop a pure Level B API as a separate product track; measure before native/SIMD. | A new API benchmark is separate unless every runtime receives an equivalent workload. Native/SIMD is an optional backend/control, not the semantic API. |
| CAP-04 load-only lifecycle boundary | Current public CLIs combine load and first result. | Measurement/runtime lifecycle boundary. | Promote to public API only with a product use case and API approval. |

### Level B performance closure

Re-audit the provisional selector/module inventory against current HEAD and
identify the bootstrap-critical scalar text, word/parse, arrays, binary
conversion, call/link, classlib collection and runtime-control surface. For
each hot selector choose and document one of:

- clean inline Level B;
- inline plus a general RXAS/VM assist;
- native/runtime intrinsic with a portable Level B fallback;
- cold/not selected; or
- blocked by a named capability/language decision.

Strict read versus grow-on-write and indexed attribute-array access are design
questions only if current semantics prevent an efficient common path. Do not
invent new syntax when flow or a runtime assist can prove the same operation.

### Level G boundary

The planned object/interface collection lowering and nested collection model
remain post-Release 1 Level G work. Interface-led collection contracts,
equality/hash/order and owned heterogeneous containers may ultimately improve
expressiveness and comparable benchmark forms, but they are not near-term
speed patches. Existing `arraydrop` and `objectarraydrop` remain Release 1
compatibility surfaces.

Any syntax, type-system, ownership, collection or public ABI change pauses at
`decision required` with an options paper and explicit Adrian approval before
implementation.

### Equivalence closure

The Apple gate completed on 2026-07-27. Towers now has a qualified
object/allocation-equivalent ooRexx port. Mandelbrot is an approved exclusion
because ordinary ooRexx decimal modes cannot satisfy the binary64 checksums.
Storage, List and JSON are approved diagnostic exclusions under their exact
ownership/result-model mismatches. CAP-01 and CAP-02 are deferred to separately
approved library/runtime or post-Release 1 Level G decisions; CAP-03 does not
block the qualified Base64 benchmark; CAP-04 retains honestly named lifecycle
phases and excludes an unavailable pure-load comparison.

## PERF2-09 — per-benchmark ooRexx closure campaign

PERF2-09 is the outcome lane. It consumes general mechanisms from PERF2-02
through PERF2-08; it does not authorize benchmark-specific shortcuts.

The first complete closure run is on the Mac after PERF2-06/07 and the PERF2-08
qualification gate. It is the source of the exact handover scorecard, not the
final cross-platform claim. Repeat the frozen portfolio on Intel Linux and
Windows after any accepted Linux tuning; do not compare unmatched Mac/Linux/
Windows sessions as before/after results.

Each dossier contains exact source/image/runtime hashes, comparability status,
same-session cREXX/ooRexx throughput, gain to parity and strong band, optimized
and diagnostic static/dynamic work, top native/procedure/opcode/call paths,
copies/conversions/allocations/RSS, selected mechanism, machine ceiling and
accepted/rejected verdicts.

The current Mac closure routing is:

| Workload | Current planning hypothesis | Candidate owners | Closure gate |
| --- | --- | --- | --- |
| Sieve | Large current cREXX win; no selected hot value/reference mechanism. | Guard only; compiler/VM regression control. | Preserve both-VM ooRexx win and the NetRexx bands. |
| Permute | Accepted direct placement removed the proved receiver-copy explosion; current product remains far ahead of ooRexx but behind NetRexx. | Guard accepted V1R01-R1; re-attribute before any residual call/value candidate. | Preserve ooRexx win and image size; do not infer a mechanism from the NetRexx gap alone. |
| Bounce | Current cREXX is ahead of ooRexx; reference traffic remains visible without one proved redundant owner operation. | Residual PERF2-06/07 proof only if a new exact nonzero reduction appears. | Guard both VMs; bytecode VM remains below the NetRexx strong band. |
| Richards | Largest qualified common deficit after accepted direct placement; residual copy/value work remains large. | Re-attribute accepted-product caller/copy shapes; no rejected reset/ledger/slab/layout retry. | Select only an exact general mechanism with a material current ceiling; preserve semantics and guards. |
| Base64 | Current deficit and post-append timing noise; string copy/length materialization remains visible. | PERF2-04/05/07 evidence only; CAP-03 API remains separate. | Require exact reduced work and a stable decisive cell before a candidate. |
| RexxCPS | Near ooRexx parity, but below the separate 1.50 band; conversions remain visible. | Later general representation/BIF evidence, not a selected slice. | Preserve disclosed 2.2d semantics and reach 1.50x canonical Classic ooRexx. |
| Mandelbrot | Approved not-comparable result: ooRexx decimal checksums differ. | No current performance owner. | No ratio unless an honest same-numeric-contract port exists. |
| Towers | Qualified object/allocation port is 3.05–3.11x from ooRexx parity. | Current payload/lifetime/high-water attribution before any PERF2-06/07 candidate. | No pooling, slab or broad layout inference without the required proof/decision. |
| Storage | Approved diagnostic exclusion under CAP-02. | Explicit post-Release 1 Level G ownership/container decision. | Equivalent ownership/allocation contract before any ratio. |
| List | Approved diagnostic exclusion because the weak-reference arena is material extra work. | Future ownership architecture only if separately selected. | No common ratio under the current forms. |
| JSON | Approved diagnostic exclusion under CAP-01. | Independently approved parse-once/result-access design. | Common contract before any ratio. |

After every accepted production slice, run the smallest decisive target and
guard comparison first, report it and stop. A full portfolio refresh follows
only after the verdict is accepted. The next slice is selected by the largest
qualified remaining deficit with an attributable general mechanism, not by
which benchmark is easiest to improve.

## PERF2-10 — toolchain, code layout, build and lifecycle

This is a bounded optional lane, not a substitute for semantic work.

Sequence: screen coherent options on Apple ARM64 only to bound them, then do
decision/tuning work on the Intel Linux machine under both GCC and Clang. Freeze
the selected source/flags before the same-machine Windows run. An Apple-only or
Linux-only win is retained as a control unless the required platform matrix
accepts it.

### Experiments

- C/C++ link-time optimization and interprocedural optimization;
- profile-guided optimization using a disclosed representative training set;
- hot/cold handler/helper outlining and source layout;
- compiler-specific flags only where supported and maintainable;
- shared VM-core build impact and text duplication;
- eager versus lazy execution-image preparation after CAP-04 attribution; and
- package/install/native artifact and startup consequences.

The VM interpreter is a very large translation unit built in threaded and
switch variants. Record build time/memory, binary text/data size, i-cache and
branch effects, startup, steady state and both VM modes on Apple ARM64, Linux
x86-64, Linux ARM64 and Windows. Keep all supported release architectures in
scope. Never select a flag/layout from one host or from instrumented-profile
elapsed time.

Adopt only repeatable supported-platform improvements with reproducible build
inputs and no benchmark-specific PGO training claim. Otherwise retain the
result as rejected/deferred evidence.

## PERF2-11 — architecture and release gates

The old unstarted architecture footer is replaced by explicit gates.

### Gate A — refreshed truth

PERF2-01 evidence is checksum-closed, same-session comparison is accepted,
comparability labels are current, and every Batch 2 candidate has a measured
mechanism footprint.

### Gate B — placement selection

For each semantic family, compare compiler/inliner, RXAS, link/load and runtime
quickening placement. Record the selected and rejected variants, machine
ceiling, code/image/RSS/startup trade-off and why later-phase knowledge is
required if an earlier owner is not selected.

### Gate C — language, ISA and ABI decisions

Any new syntax/type/ownership rule, public RXAS opcode/RXBIN feature or public
runtime ABI change has an options paper and explicit Adrian approval. Private
PoCs do not create a de facto public contract.

### Gate D — mandatory first Release verdict

After the minimum focused correctness checks pass, freeze the production edit,
build the ordinary profiling-off Release product and run the smallest decisive
exact-hash end-to-end comparison against a retained valid baseline. Report and
stop for direction. The implementation remains provisional/revertable until
accepted. Broad QA, sanitizer, packaging and documentation closeout follow the
decision, not precede it.

The existing formal guards remain: no unexplained worse-than-3% individual
workload and no worse-than-1% common aggregate without explicit acceptance.
Instruction reduction, profile time and microbenchmarks do not replace the
complete-product verdict.

After acceptance, select broad QA, sanitizer, install/package,
RXBIN/ABI/feature-gating and documentation closeout in proportion to the
changed surface, observed failures and Adrian's direction. They follow the
first verdict; the roadmap does not mandate every broad lane for every slice.

### Gate E — cross-platform/default VM

Before default-architecture selection, retain ordinary product evidence from
at least Apple ARM64, Linux x86-64, supported Linux ARM64 and Windows timing,
with both VM modes, code/artifact size and relevant native counters. Keep all
supported release architectures in the regression matrix. Include exact
supported compiler/version coverage: Apple clang on ARM64, GCC and Clang on
native Linux x86-64 where supported/available, and the supported Windows
toolchain. Select the default/private stream using the whole scorecard rather
than one dispatch microbenchmark.

The approved collection order is Apple ARM64 pre-handover, Intel x86-64 Linux
GCC/Clang tuning and decision, supported Linux ARM64 validation, then the same
Intel x86-64 hardware under the supported Windows toolchain. Gate E is not met
by Apple plus Intel/Windows alone: Linux ARM64 remains a required supported
release-architecture lane.

The 2026-07-29 Windows x86-64 baseline is retained under
[`2026-07-29-perf2-11-windows-x86-64`](evidence/2026-07-29-perf2-11-windows-x86-64/).
It completes the requested Windows scorecard lane but does not close Gate E:
supported Linux ARM64 and a selected cross-platform/default-VM decision remain
outstanding.

### Gate F — final external claim

- zero correctness failures;
- every Tier A cell qualified or explicitly resolved under PERF2-08;
- selected default VM reaches at least 1.50x ooRexx on every qualified common
  cell and the separately disclosed cREXX RexxCPS 2.2d diagnostic reaches at
  least 1.50x same-session canonical Classic ooRexx RexxCPS;
- common-workload geometric mean reaches at least 2.00x ooRexx;
- alternate/non-default VM is clearly faster than ooRexx on every qualified
  cell;
- both VMs, lifecycle, RSS and artifact results shown separately;
- runtime/compiler/source versions and exact hashes published; and
- no common/aggregate claim based on a cross-session ratio, adapted cell or
  excluded diagnostic. cREXX 2.2d versus canonical Classic ooRexx RexxCPS
  remains a separately disclosed, governed diagnostic comparison rather than a
  common cell.

## PERF2-12 — JIT/AOT decision

NetRexx demonstrates the ceiling available when a hot semantic graph reaches a
mature optimizing VM, but it does not prove that a cREXX JIT yields its exact
ratio. A JIT, trace compiler, native AOT backend or external optimizer is a
separate architecture programme.

Keep PERF2-12 deferred until the accepted non-JIT programme and current
cross-platform scorecard show that the unquestionable-superiority exit cannot
be reached economically. Reopening it requires:

- the residual gaps and hot semantic graph after PERF2-02 through PERF2-10;
- a comparison of native AOT, baseline JIT, tracing/quickening and existing VM
  maintenance cost;
- debugger/TRACE, signals, dynamic loading, plugins, portability, packaging,
  sandboxing and deterministic-build requirements; and
- an explicit architecture decision from Adrian.

## Worklist and evidence contract

Before the first production edit in any activity, create a resumable worklist
that records:

1. exact clean baseline commit, branch state and artifact hashes;
2. one falsifiable performance hypothesis and named mechanism footprint;
3. candidate variants, static/runtime ownership and machine ceiling;
4. semantic risks and focused correctness matrix;
5. exact profiling-off Release comparison and regression guards;
6. first Release verdict stop point;
7. proportional broad QA, sanitizer, install/package,
   RXBIN/ABI/feature-gating and documentation closeout after acceptance and
   Adrian's direction;
8. retained accepted, rejected and neutral evidence; and
9. status/update links back to this register.

Target-only builds and focused PoC loops come first. Formal baselines use the
ordinary product and existing governance. All new performance orchestration is
Level B cREXX under `performance/tools/`; temporary host-side analysis may be
used for investigation but does not become the maintained control plane.

## Authoritative references

- programme rules: [`AGENTS.md`](AGENTS.md) and
  [`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md)
- initial closed register:
  [`ROADMAP-INITIAL-SWEEP-2026-07-23.md`](ROADMAP-INITIAL-SWEEP-2026-07-23.md)
- dated charter:
  [`performance-programme-report-2026-07-15.md`](../docs/planning/release-1/performance-programme-report-2026-07-15.md)
- cross-runtime mechanisms:
  [`rexxcps-runtime-source-review-2026-07-22.md`](rexxcps-runtime-source-review-2026-07-22.md)
- latest current-product checkpoint:
  [`2026-07-23 NR-16/NR-17 closeout`](evidence/2026-07-23-nr-16-17-closeout/README.md)
- capability ledger: [`capability-gaps.md`](capability-gaps.md)
- compiler and VM architecture:
  [`CREXX_ARCHITECTURE.md`](../docs/ai-context/CREXX_ARCHITECTURE.md),
  [`RXAS_ASSEMBLER.md`](../docs/ai-context/RXAS_ASSEMBLER.md) and
  [`RXVM_INTERPRETER.md`](../docs/ai-context/RXVM_INTERPRETER.md)
- inlining and flow evidence:
  [`NR-12-21-WORKLIST.md`](NR-12-21-WORKLIST.md),
  [`NR-26-WORKLIST.md`](NR-26-WORKLIST.md) and
  [`NR-27-WORKLIST.md`](NR-27-WORKLIST.md)
- language/capability planning:
  historical/working
  [`levelb-language-improvement-backlog.md`](../docs/planning/beta-3/notes/levelb-language-improvement-backlog.md)
  (reconcile completed items before reuse),
  [`array-statements.md`](../docs/planning/beta-3/notes/array-statements.md) and
  [`cross-cutting-conclusions.md`](../docs/planning/release-1/component-catalogue/cross-cutting-conclusions.md)
- VM investigation history:
  [`vm-dispatch-performance-investigation.md`](../docs/planning/beta-3/notes/vm-dispatch-performance-investigation.md)
