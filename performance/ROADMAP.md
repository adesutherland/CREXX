# cREXX performance roadmap

Last updated: 2026-07-22

This is the live performance-programme register. The definitions, evidence and
exit criteria come from the dated
[`performance programme report`](../docs/planning/release-1/performance-programme-report-2026-07-15.md).
Use this file for status, short dated notes, idea capture and links to retained
evidence. Do not rewrite the dated report to reflect later work.

Status values are `queued`, `in progress`, `blocked`, `decision required`,
`complete`, `rejected` and `superseded`. Only use `complete` when the report's
exit criterion is met.

## P0 no-regrets register

| ID | Activity | Status | Current note / next gate |
| --- | --- | --- | --- |
| NR-01 | Reproducible benchmark portfolio and runner | complete | The approved portfolio, serial correctness-gated raw capture, compact machine/build provenance, separate lifecycle reporting and NR-11 publication policy are all implemented and proved by the NR-10 formal bundle. |
| NR-02 | Portfolio equivalence and optimizer-resistance ledger | complete | All approved steady-state/runtime cells and the lifecycle lane have correctness, equivalence, generated-form and retained-pilot evidence or an explicit `not comparable` disposition. |
| NR-03 | Automated performance evidence bundle | complete | One Level B command retains exact-hash Release timing, schema-3 profile/allocation evidence, RXSEQ N=2/3/4, ranked reports and paired deltas; the verified proof bundle is `evidence/2026-07-15-nr-03-automated-proof/`. |
| NR-04 | Opcode effects inventory | complete | All 641 opcode slots have ordered machine-readable effects; 539 source opcodes are classified or explicitly conservative and RXAS consumes the fail-closed API without new transforms. |
| NR-04A | Runtime type/dispatch architecture and metadata scan evidence | complete | Complete re-linkable RXBIN 007 images, process-local graph views/bindings, site caches, and transparent section compression pass. Retained interface is 37,458 B and RexxCPS 273,858 B; hot graph access remains approximately 1 ns with no focused end-to-end regression. Cross-image factory providers are aggregated correctly, RXAS output re-links byte-for-byte, both VMs pass, and final Debug CTest is 1,846/1,846. Evidence: `evidence/2026-07-16-nr-04a-rxbin-007-compression/`. |
| NR-05 | Call-path census | complete | Schema-4 dynamic census plus exact 22-image dashboard retained at `evidence/2026-07-16-nr-05-call-census/`; focused fixtures cover native/dynamic/signal cold paths absent from the bounded portfolio. |
| NR-06 | Compiler call-window placement fast path | complete | Typed scalar copying was rejected and removed. The retained affinity-guided numbering path removes 58/423 static portfolio swaps and 248,362 executed swaps in the bounded hot cells without adding runtime instructions. The direct cost is too small for material product speedup, but Adrian accepted the verified work reduction; Debug/Release focused checks and final Debug CTest 1,849/1,849 pass. See `NR-06-07-WORKLIST.md`. |
| NR-07 | Direct compare-to-branch and loop lowering | rejected | Interleaved Release evidence found no practical gain from either the direct-condition or specialist-loop paths, and RXAS already emitted the tested `BCTP` product forms. Both compiler changes and their dedicated fixtures are removed; exact rejected patches and evidence are retained. See `NR-06-07-WORKLIST.md`. |
| NUMERIC-01 | Native typed Level B numeric surface and RexxCPS type fidelity | complete | Eleven native integer/float BIFs are accepted; standard benchmarks are decimal-free except RexxCPS's intentional Classic workload. RexxCPS 2.2d uses explicit digits 9, honest decimal/int/float paths and typed text BIFs. Accepted medians are 1.15 MCPS (`rxvm`) and 1.11 MCPS (`rxbvm`); final Debug CTest is 1,851/1,851. |
| NR-08 | Definite initialization and reference-lifetime facts | complete | Adrian accepted Gate A after canonical RexxCPS improved by 4.358% (`rxvm`) and 5.903% (`rxbvm`). The corrected library audit changes only `ENDLIFE`, 8,006 to 151. Complete Debug build and final CTest 1,851/1,851 pass; the 71 intentional goldens remove exactly 170 additional scalar `ENDLIFE`s and no other lines. Evidence: `evidence/2026-07-17-nr-08-first-release-verdict/`. |
| NR-09 | RXSEQ-guided lowering and instruction synthesis | complete | Rule 1 and the corrected 34-form public batch are accepted and closed. The batch withdraws 26 forms behind stable reserved IDs, promotes three masked hot forms and narrows two-register `ITOF`; two rejected side-effect-preserving replacements remain future design ideas and do not block completion. Broad QA passes: Debug 1,864/1,864, supported ASan clean after locking one shared-file test race, isolated install and exact old-RXBIN execution in both VMs. Documentation covers all 574 forms/367 mnemonics and all 373 tagged examples assemble. The final 78/78 Release refresh is +1.385%/+2.868% complete-product paired median CPS (`rxvm`/`rxbvm`); only the `rxbvm` interval is wholly positive. |
| NR-10 | Cross-runtime forensic baselines | complete | The corrected same-host common baseline uses equal work and decimal-semantics NetRexx on the default HotSpot JIT. Its 500 timing/RSS/lifecycle rows and 24 labelled native-C control rows pass; the initial binary-typed NetRexx matrix is retained only as an excluded audit/non-common diagnostic source at `evidence/2026-07-20-nr-10-formal-baseline/`. |
| NR-11 | Performance governance and scorecard | complete | Approved policy is normative in `PERFORMANCE-GOVERNANCE.md`; the standard template is proved by the NR-10 scorecard with four named N=5 aggregates, explicit noisy cells and no unmatched regression claim. |

### NR-01 work notes

- 2026-07-15: Chose a top-level `performance/` operational workspace.
  `tests/benchmarks/` remains the portable language suite and
  `tests/performance/` remains the focused internal microbenchmark area.
- 2026-07-15: Added `portfolio/manifest.md` for the five-workload seed and added
  optional per-warmup/per-run serial sample CSV to `run_benchmarks.crexx`.
- 2026-07-15: Retained the first dated seed evidence bundle at
  `evidence/2026-07-15-seed-portfolio/`: clean targeted Release build, 13/13
  benchmark gate, five workloads, and 60 serial observations with exact
  provenance. The bundle is a capture-path baseline, not an approved portfolio
  threshold.
- 2026-07-15: Added the benchmark-native RexxCPS clause-rate observations to
  the evidence bundle. Three matching `rxvm` canonical runs reported a mean of
  845,834 CPS; this is distinct from the runner's startup-inclusive elapsed
  time.
- 2026-07-15: Added `portfolio/cross-runtime-plan.md`. The proposed Tier A
  planning range is roughly 12-16 workloads, but coverage cells and comparable
  ports—not a round number—are the completion gate.
- 2026-07-15: Comparative scope set: NR-02 covers every selected Tier A
  benchmark across CREXX, ooRexx and NetRexx, including porting, equivalence,
  optimizer-resistance evidence and an initial retained run. Regina is limited
  to RexxCPS. NR-10 receives each qualified workload for formal baseline and
  forensic capture.
- 2026-07-20: Closed NR-01 through the approved NR-10/NR-11 campaign. The
  formal bundle proves serial correctness-gated capture, machine/build/runtime
  provenance, separate steady-state and lifecycle reports, compact raw-sample
  retention and the standard publication scorecard for the approved portfolio.

### NR-02 work notes

- 2026-07-15: Adrian approved a bounded 12-item Tier A portfolio: the five seed
  workloads; Bounce, Storage, List, Richards, a JSON parser capability workload
  and an RFC 4648 Base64 round trip; plus compile/load/first-result as a
  separately reported lifecycle lane. Queens, DeltaBlue, Havlak,
  filesystem-I/O work and focused Classic semantic probes remain Tier B/reserve
  candidates.

- 2026-07-15: Started the temporary resumable matrix in
  `NR-02-WORKLIST.md`, including provenance, license, correctness,
  equivalence, optimizer-resistance, pilot and disposition fields per runtime
  cell. Proposed a 16-workload Tier A working set against the named coverage
  gaps; final approval remains NR-11.
- 2026-07-15: Runtime inventory found Temurin JDK 26.0.1 and Regina 3.9.7.
  No ooRexx or NetRexx executable was present. The official NetRexx site names
  5.10-GA (2026-03-20) as current; it was installed reversibly under the user
  local tool area. A minimal compile/run passed on JDK 26.0.1 and retained the
  generated Java, Java 8 class file and disassembly.
- 2026-07-15: Qualified the available CREXX and NetRexx Sieve cells. Both
  preserve the 5,000-slot algorithm, accept runtime repetition perturbations,
  observe the 669-prime result and have retained serial 50-repetition pilots;
  the later ooRexx completion is recorded below.
- 2026-07-15: Qualified the available CREXX and NetRexx Permute cells. The
  NetRexx generated form retains object construction, recursion and paired
  swaps; both ports observe 8,660 calls and have retained serial
  50-repetition pilots. The later ooRexx completion is recorded below.
- 2026-07-15: Mandelbrot passed sizes 1/500/750 on CREXX and NetRexx and has
  retained size-500 pilots. NetRexx has no source `^` operator, so its exact
  checksum uses a timed arithmetic XOR helper plus a partial-byte padding
  loop. Keep that cell a disclosed adaptation pending common-aggregate
  equivalence review; do not hide the added work.
- 2026-07-15: Qualified the available CREXX and NetRexx Towers cells. Both
  retain disk allocation, pile operations and the 13-disk recursive move
  graph, observe 8,191 moves, pass repetition perturbations and have retained
  serial 10-repetition pilots. The later ooRexx disposition is recorded below.
- 2026-07-15: Added Classic procedural sources for all four non-RexxCPS seed
  workloads. Regina surrogate checks passed Sieve/Permute/Towers at 1/2 and
  Mandelbrot at size 1; these are source checks, not Regina portfolio or
  ooRexx qualification. Classic Mandelbrot's arithmetic XOR is impractically
  slow at full size under Regina, and Classic Towers' stem/node-id model does
  not preserve object allocator/dispatch cost. Both remain disclosed
  adaptations; their subsequent ooRexx review is recorded below.
- 2026-07-15: Retained the RexxCPS qualification slice for CREXX, Regina and
  NetRexx. Byte-exact Classic 2.2 and bundled NetRexx 2.1n sources, disclosed
  2.2n/2.2c adaptations, A/B opaque/result variants, serial pilots, generated
  Java/classes/disassembly and Regina trace counts are recorded under the
  dated NR-02 bundle. The samples are noisy and not an NR-10 ratio. Regina's
  RexxCPS-only NR-02 lane is complete; the later ooRexx completion is recorded
  below.
- 2026-07-15: Homebrew offered no ooRexx formula/cask and its `rexx` command
  was confirmed as Regina. Installed the official stable ooRexx 5.1.0 r12973
  universal portable build under `/Users/adrian/.local/opt/oorexx/5.1.0-12973`
  without changing the Regina symlink or shell startup files; retained source,
  checksum, version and minimal execution proof in the NR-02 evidence bundle.
- 2026-07-15: Completed the ooRexx RexxCPS cell: canonical and A/B opaque
  pilots passed, both variants observed `1|69|1.22694`, executable translated
  images ran, and a bounded count-one `TRACE I` retained 990 source plus 4,012
  intermediate kernel records. The full four-runtime RexxCPS NR-02 slice is
  qualified; formal repeated comparison remains NR-10.
- 2026-07-15: Qualified the ooRexx Sieve and Permute seed cells with 1/2
  perturbations, translated images and serial pilots. Towers also ran correctly
  but its stem/node-id representation removes the object allocation/dispatch
  pressure under study, so the current ooRexx cell is `not comparable` for the
  common score and remains only a disclosed procedural diagnostic.
- 2026-07-15: ooRexx Mandelbrot passed size 1 but failed the common correctness
  contract at size 500 (255 versus 191) and size 750 (128 versus 50) because
  decimal numeric semantics change boundary results. Both long serial negative
  runs are retained. The cell is `not comparable`; no runtime-specific answer
  was accepted and no pilot score entered the common matrix.
- 2026-07-15: Added `evidence/benchmark-median-summary.md` as the live compact
  cross-bundle comparison. Each row is one dated benchmark/run with platform
  median columns and sample counts. Warmups, correctness failures and invalid
  diagnostic/version modes are excluded explicitly; passing values are not
  discarded merely for looking extreme. Added the exact opaque A/B input map
  to the RexxCPS ledger so those diagnostic rows are interpretable.
- 2026-07-15: Repeated the cREXX portfolio after clean CLion Release/Debug
  rebuilds. The verified Apple-clang Release VM compile and link used
  `-O3 -DNDEBUG`; canonical RexxCPS recovered to an 857,561-CPS median versus
  843,110 in the seed bundle and 527,607 in the slower pilot. Opaque A/B also
  recovered to 840,065/836,741 CPS. Sieve, Permute and Mandelbrot remained in
  the seed band. Towers ranged from 688 to 771 ms across three passing series
  on the same image, so all samples are retained and a quiet NR-10 run remains
  the regression gate. See `evidence/2026-07-15-crexx-rexxcps-o3-rerun/`.
- 2026-07-15: Implemented and qualified the approved expansion across cREXX,
  ooRexx and NetRexx: Bounce, Storage, List, Richards, JSON and RFC 4648
  Base64, plus the cREXX-scripted compile/translate/assemble/load-first-result
  lane. All six workloads pass 1/2 perturbations on all three runtimes; cREXX
  optimized/unoptimized CTest passes. One-warmup/three-recorded pilots,
  ooRexx translated images, NetRexx generated Java/classes/disassembly and
  lifecycle samples are retained under
  `evidence/2026-07-15-nr-02-portfolio-expansion/`.
- 2026-07-15: Closed NR-02 with explicit comparability boundaries. cREXX
  Storage is `not comparable` because a wrapper object plus `.object[]`
  replaces each upstream array. JSON is a native-surface diagnostic because
  cREXX uses string/path parsing while ooRexx and NetRexx build object models.
  List/cREXX retains its weak-reference arena as a disclosed adaptation. The
  resulting capability/accuracy register is `capability-gaps.md`; references
  and binary-safe `.binary` storage are confirmed present, not missing.

### NR-03 work notes

- 2026-07-15: Started the resumable implementation ledger in
  `NR-03-WORKLIST.md`. The ordinary Release build is the verified
  `-O3 -DNDEBUG`, profiling-off product baseline; no profiling build was
  configured at handover. Existing schema-v2 instrumentation and focused tests
  cover instruction/transition/procedure/native/call-mechanics timing,
  overflow/degraded status, and RXSEQ N=2/3/4 exact-module analysis. Code audit
  confirmed that allocation/value/frame counters are absent. A private
  profiling-schema extension is sufficient and does not require a language,
  RXBIN, public ABI, ownership, or architecture decision.
- 2026-07-15: Completed NR-03 with the Level B
  `tools/run_evidence_bundle.crexx` orchestration path and versioned
  `manifests/nr03-proof-v1.txt`. Schema 3 adds documented profiling-only
  allocation/value/frame counts, bytes, frame reuse/high water, explicit
  native-child accounting, and row status. The retained proof bundle at
  `evidence/2026-07-15-nr-03-automated-proof/` covers five exact
  optimized/unoptimized pairs, 40 serial ordinary-Release samples, 10 profile
  CSVs, 30 binary RXSEQ captures plus decoded candidate CSVs, generated paired
  summaries, and 311 independently reverified checksums. Release benchmark
  CTest passed 29/29 and profiling/RXSEQ CTest passed 11/11. Profiling-off
  help/symbol/compiler/preprocessor checks found no surviving instrumentation.
  The proof deliberately retains all passing samples; its timing deltas are
  observations, not a new regression policy or optimizer-causality claim.

### NR-04 work notes

- 2026-07-15: Completed the canonical opcode-effects sidecar and consolidated
  `rxop_effects()`/`rxop_effect_count()` API. The mechanically closed inventory
  covers 641 dense slots: 539 source opcodes (533 classified and six explicit
  conservative process/redirect operations), 99 reserved slots and three
  internal handlers. Effects cover explicit read/write/kill masks, implicit
  fixed/indexed/range registers, branch targets and indirect branches, calls
  and returns, alias/reference/lifetime behavior, exceptional transfer and
  opaque barriers. Unknown, reserved, internal and conservative effects fail
  closed. RXAS liveness now consumes the consolidated masks without enabling a
  new transformation. Debug and Release metadata/optimizer tests passed 37/37;
  the rebuilt broad Debug suite passed 1840/1840. NR-04A, CFG/global data flow,
  dead NULL/ENDLIFE removal and RXBIN/public ABI changes remain out of scope.

### NR-05 work notes

- 2026-07-16: Completed profiling schema 4 with authoritative dynamic
  path/site/actual-arity/kind/frame/outcome rows, actual return placement,
  method/factory selection outcomes, and bytecode/native signal-unwind work.
  NR-04 effects drive an executed backward slice for call-window setup swaps
  and defensive argument copies; normal restoration is credited only when the
  observed swap sequence recovers the reconstructed pre-call mapping. Every
  executed `SWAP_REG_REG` and `COPY_REG_REG` remains mechanically partitioned,
  with unproven work explicitly unclassified. The feature is compile-time
  absent from ordinary Debug/Release and changes neither RXBIN nor public ABI.
- 2026-07-16: The one-command, checksum-closed bundle at
  `evidence/2026-07-16-nr-05-call-census/` covers all eleven current language
  workloads in noopt/opt form (22 exact images). Across the disclosed bounded
  argv it records 16,439,420 calls: 16,439,188 direct bytecode, 210 dynamic
  bytecode and 22 external roots; arity 1 dominates at 8,767,984. Recycler
  frames supply 16,438,977 activations versus 443 fresh. The dynamic mechanics
  are 41,981,144 setup and 41,981,144 normal-restoration swaps (2.553691 each
  per instruction call), 675,554 attributable defensive copies (0.041094 per
  instruction call), 266 unclassified swaps and 13,603,218 unclassified
  copies, with zero degraded attributions. Returns comprise 4,517,546 local
  moves, 8,591,365 non-local copies, 2,028,919 void, 1,301,568 immediate and 22
  terminal placements. The bounded portfolio observes 210 successful dynamic
  selections and no signals/native calls; deterministic dual-VM fixtures
  cover native calls, method/factory dispatch, frame failure domains, normal
  restoration, multi-frame signal unwind and interrupted-native restoration.
  All 541 recursive checksums and independent raw-profile reconciliations pass;
  the broad Debug suite passes 1844/1844. NR-06 should use the path/arity and
  5.107382 combined swaps-per-instruction-call evidence; NR-12 should keep the
  675,554 attributable argument copies distinct from the 8,591,365 measured
  `RET_REG` non-local copies and from the unclassified copy population.

### NR-06/NR-07 work notes

- 2026-07-16: Started the approved compiler fast-path PoC batch at clean
  `develop` commit `6e52ef872`. The shared worklist records separate design
  selection and proof gates. NR-06 will first compare typed Boolean/integer/
  float argument copies in the unchanged contiguous window with the current
  setup/restoration swap pair; every reference, optional and non-scalar case
  fails closed. NR-07 will first lower optimized direct integer conditions to
  existing branch opcodes while preserving no-opt, type/cleanup fallbacks and
  authored source steps. Iteration is restricted to target-only `rxc` builds
  and focused fixtures before either production Release verdict.
- 2026-07-17: Revalidated the in-progress batch after the unrelated
  datatype-source hot-fix advanced `develop` to `5e5e3b397`; no batch file or
  compiler surface overlapped the hot-fix.
- 2026-07-17: The shared PoC found 30 NR-06 typed-scalar copy sites and 283
  NR-07 direct-condition sites across the 11 optimized portfolio sources.
  NR-06 was isolated for the mandatory first Release verdict. Permute x50 was
  neutral under `rxbvm` (-0.252%) and materially noisy/adverse under `rxvm`
  (+3.369% median, with high-side outliers), so NR-06 is not yet an accepted
  performance win and remains provisional/revertable. NR-07's validated PoC
  lowering remains held out pending direction. Evidence:
  `evidence/2026-07-17-nr-06-first-release-verdict/`.
- 2026-07-17: Adrian rejected the NR-06 typed-scalar copy candidate, so that
  production lowering was removed. A separate static register-placement idea
  is now explicit in the worklist: order distinct named registers from
  call-site affinities and choose/reserve a compatible contiguous window with
  exact symbol occupants. A uniform final permutation cannot reduce swaps, but
  this preassignment form does not merge live ranges and need not wait for the
  NR-08 destructive-placement proof. It still requires focused allocation,
  result/count-slot, repeated-source, reference and signal-unwind validation.
- 2026-07-17: Restored the validated NR-07 lowering byte-for-byte, passed the
  focused structural and dual-VM fixture, completed the ordinary profiling-off
  Release build, and ran canonical RexxCPS with one warmup plus three recorded
  serial samples per image/VM. Candidate median CPS regressed 11.139% in
  `rxvm` and 5.573% in `rxbvm`; reversed `rxbvm` ordering did not remove the
  regression. All 16 runs passed. NR-07 is not an accepted win and stops at
  the decision gate. Evidence:
  `evidence/2026-07-17-nr-07-first-release-verdict/`.
- 2026-07-17: Re-ran NR-07 with original, direct-only and full specialist-loop
  images in five round-interleaved samples per image/VM. The earlier large
  direct-only regression did not reproduce, but full loops versus direct-only
  remained neutral/adverse at -0.099% CPS in `rxvm` and -0.293% in `rxbvm`.
  Disassembly showed the direct `BCTP` emission duplicated an existing RXAS
  fold. Adrian rejected and removed the specialist extension because no
  practical performance improvement justified the added compiler complexity.
  Evidence: `evidence/2026-07-17-nr-07-loop-release-verdict/`. NR-06 now moves
  to the approved affinity-guided register/window-placement PoC.
- 2026-07-17: The bounded NR-06 affinity PoC preassigned distinct local
  symbols into lexical call groups and reused a call window only when occupied
  slots matched exactly and all other slots were free. The exact 11-source
  Release comparison removed 58/423 static swaps with five extra summed local
  slots; RexxCPS removed 44/121 swaps and reduced its local sum by one. The
  profiling-off full Release build and 25/25 focused Release checks passed.
  Six order-balanced interleaved RexxCPS rounds then split: paired median CPS
  was -1.928% in `rxvm` and +1.676% in `rxbvm`, amid much larger host drift.
  No reliable practical win is demonstrated, so the production candidate
  remains provisional at the first verdict gate. Evidence:
  `evidence/2026-07-17-nr-06-register-affinity-verdict/`.
- 2026-07-17: Follow-up causal audit verified the exact distinct binaries and
  RXAS/link path. RexxCPS's 44 static removals are all in the unexecuted trace
  handler, so its baseline and candidate both execute 484,376 swaps. List,
  Richards, Storage and Base64 do remove 248,362 executed swaps at the retained
  argv. A direct 100,000,000-iteration profiling-off Release diagnostic prices
  one swap at 0.434 ns in `rxvm` and 0.706 ns in `rxbvm`, implying only
  0.000108%--0.040474% end-to-end opportunity. Storage also demonstrates that
  numbering can change later RXAS selection: moving its loop counter to `r1`
  makes the fixed `INC1` rule pre-empt 40 existing `BCTP` folds. The practical
  product benefit is therefore very small, but Adrian selected the bounded
  affinity implementation for retention because it verifiably removes static
  and executed swaps without adding runtime instructions. The rejected NR-07
  direct-condition lowering remains removed; exact patches, fixtures, profiles
  and timings are retained. The constant/by-value flag and branch backlog
  remains a separate item.
- 2026-07-17: Completed the approved NR-06/NR-07 batch closeout. Retained NR-06
  passed 26/26 focused selections, including the linked-runtime fixture, in
  both Debug and ordinary profiling-off Release.
  Seven intentional optimized golden changes removed another 86 static swaps;
  all matching runnable fixtures passed and the focused acceptance set passed
  16/16. The full Debug build then passed and final high-parallel Debug CTest
  passed 1,849/1,849. NR-06 is complete; NR-07 is rejected and removed. No
  sanitizer, package/install, cross-platform or additional timing work was
  added beyond the approved shortest closeout path.

### NUMERIC-01 work notes

- 2026-07-17: Completed the approved native numeric-surface and benchmark type
  audit from clean `develop` at `e23d44e58`. Added native `int`/`float`
  siblings for the appropriate `ABS`, `MIN`, `MAX`, `SIGN`, `TRUNC` and
  `FORMAT` surfaces without changing existing decimal contracts, VM opcodes,
  RXBIN or ABI. Corrected Bounce's integer `ABS` calls and float formatting in
  the evidence tool; all other standard cREXX benchmarks are decimal-free,
  while Mandelbrot's binary64 work is intentional.
- Adrian accepted candidate C. Canonical and opaque RexxCPS are now versioned
  2.2d: genuine Classic arithmetic is decimal under explicit digits 9,
  semantically integral paths stay integer, and timer/rate work is a disclosed
  binary64 adaptation using `floattrunc`/`floatformat`. Historical 2.2c and
  A/B candidate evidence remains separate.

| Verdict | Accepted result |
| --- | --- |
| Focused correctness | 17/17 numeric and 9/9 benchmark/tool/RexxCPS Release checks pass |
| Classic fidelity | Opaque result `1|69|1.22694` matches retained Classic evidence; authored procedures select digits 9 |
| Generated purity | Final optimized/no-opt canonical and opaque RXAS has zero `ftod`, `itod` and `ftoi`; remaining conversions are semantic |
| First Release comparison | C versus A mean observation: +1.621% `rxvm`, +1.069% `rxbvm`; C versus B neutral/noisy and not claimed as a BIF speedup |
| Accepted exact-hash baseline | Median 1,145,721 CPS / 8.74 s on `rxvm`; 1,114,685 CPS / 8.99 s on `rxbvm`, three recorded runs each |
| Broad validation | Full Debug build and CTest `--parallel 30`: 1,851/1,851 pass |

Evidence: `evidence/2026-07-17-numeric-01-first-release-verdict/`. The final
canonical source SHA-256 is
`2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6`.

### NR-08/NR-09 work notes

- 2026-07-17: Started the ordered combined design/PoC session at clean
  `develop` commit `e748621d1`, exactly equal to `origin/develop`. The resumable
  control plane is `NR-08-09-WORKLIST.md`. NR-08 is active; NR-09 remains
  queued until the NR-08 PoC result is frozen so removed lifecycle sequences
  cannot be promoted as fusion candidates.
- The status quo distinguishes the operations mechanically: `NULL` is a
  whole-value clear and proven kill, while `ENDLIFE` preserves ordinary
  contents but invalidates reference identity and remains a read/write,
  reference-release, lifetime-end, may-throw opaque barrier. The first bounded
  design therefore considers only a symbol-local, fail-closed `ENDLIFE` gate;
  `NULL` and copies remain unchanged without their own dataflow proofs.
- The current compiler already excludes exposed/argument/receiver/factory,
  reference-target, inline-generated and TRACE-generated locals from scoped
  register recycling, but lifetime emission is broader and emits `endlife` for
  every ordinary general-register local in a recyclable scope. Exact current
  canonical/opaque, optimized/unoptimized RexxCPS 2.2d inventory and dynamic
  counts are the next gate; historical NR-05 RexxCPS 2.2c counts are not a
  current baseline.
- 2026-07-17: Froze the NR-08 bounded candidate. Gate A omits `ENDLIFE` only
  for exact eligible scalar locals and leaves `NULL`, copies, effects and ISA
  unchanged. Focused final reference/structural checks are 26/26 and stem
  checks 3/3; byte-identical reference controls retain cleanup. Canonical
  dynamic `ENDLIFE` falls from 1,301,489/1,301,497 to 1,516 no-opt and from
  1,379,697/1,379,705 to 5,616 opt on `rxvm`/`rxbvm`. The recommendation is
  provisional advance only; imported-library NULL/copy DCE cascade must be
  audited if selected for the formal Release verdict.
- 2026-07-17: Froze the NR-09 pipeline PoC. The separate Level B processor
  keeps three exact revisions distinct, mechanically joins all 641 opcode
  effects, emits 11,332 ledger rows, and selects 76 stable identities from the
  global/per-workload bounded slice. Repeated `rxvm`/`rxbvm` processing is
  byte-identical and exact representative source counts reconcile. Statuses
  are 9,928 reviewed, 1,404 unreviewed, 9,558 unsafe, 322 subsumed and 48
  candidate. This advances the evidence pipeline only; no fusion/opcode is
  authorized before Adrian selects one stable family and its semantic proof.
- 2026-07-17: Adrian expanded NR-09 before selecting NR-08 for its formal
  verdict. NR-09 must implement mechanically obvious exact-operand rules and
  use the ledger to retain/prioritize the remainder; it is not complete at the
  mining pipeline. The first explicit queue is (1) use existing `NUMSCI` or
  `NUMENG` instead of five compatible constant procedure-entry `SETNUM*`
  instructions, (2) direct `LOAD_REG_INT | ICOPY_REG_REG` destination lowering
  with dead-temporary proof, and (3) exact identity-copy/dead-load/same-pair
  swap cases that actually occur. The architectural comparison must include a
  generalized synthesized numeric-context instruction and installing a
  non-inherited procedure default during frame activation, not just fusing
  adjacent existing opcodes. Canonical ISA/RXBIN or architectural selection
  remains an explicit Adrian decision. Full scope and proof gates are in
  `NR-08-09-WORKLIST.md`; work resumes after accepted NR-08 closeout.
- 2026-07-17: NR-08 passed the mandatory first ordinary profiling-off Release
  verdict and stopped. The full Release build and focused 4/4 CTest pass. Using
  the retained exact-source NUMERIC-01 accepted baseline, canonical optimized
  RexxCPS median CPS improves from 1,145,721 to 1,195,649 on `rxvm` (+4.358%)
  and from 1,114,685 to 1,180,487 on `rxbvm` (+5.903%); median process elapsed
  falls 4.119% and 5.673%, respectively. All eight candidate runs pass and the
  candidate ranges do not overlap the retained baseline ranges. The required
  library audit exposed a counting-script defect, not a generated-code DCE
  cascade: label-aware replay changes `ENDLIFE` 8,006 to 151 across 263/629
  procedures while `NULL` stays 4,293, copies 8,478, unlink 1,758 and reference
  ops 7. Evidence:
  `evidence/2026-07-17-nr-08-first-release-verdict/`.
- 2026-07-17: Adrian accepted NR-08 and approved the shortest closeout. The
  complete Debug build passed. Initial broad CTest exposed 71 expected-output
  mismatches; exact audit found only 170 removed scalar `ENDLIFE`s and no other
  changed lines. After the documented golden refresh, the failed selection
  passed 71/71 and final Debug CTest passed 1,851/1,851 in 217.35 seconds. No
  sanitizer, package/install, cross-platform or extra timing campaign was
  added. NR-08 is complete and NR-09 is the next queued activity.
- 2026-07-17: Resumed NR-09 at clean `develop` commit `7b93bef73`, exactly
  equal to `origin/develop`. The existing ordinary Release product hashes
  exactly match the accepted NR-08 candidate, establishing an exact
  post-NR-08 rather than NR-05/pre-NR-08 baseline. Canonical optimized
  RexxCPS has 25 static procedure-entry setters and its retained exact-image
  profiles execute 542,500 setters on each VM. The worklist now compares five
  designs. Rule 1 selects compiler-owned existing `NUMSCI`/`NUMENG` emission
  with a mandatory digits >= 5 compatibility gate; RXAS peepholing is rejected
  for this rule, while generalized opcode, procedure-default and private-image
  synthesis remain deferred design comparisons requiring their stated gates.
  Evidence starts at
  `evidence/2026-07-17-nr-09-numctx-first-release-verdict/`.
- 2026-07-17: Froze Rule 1 after focused Debug 12/12 and focused Release 1/1
  passed. Canonical optimized RexxCPS changes 25 static setters to five
  `NUMSCI` operations and 542,500 dynamic setup instructions to 108,508 on
  each VM (-433,992). RXAS shrinks 300 bytes, standalone RXBIN grows 8 bytes,
  and the linked library shrinks 808 bytes; retained library setup sites fall
  3,123 to 651. Against the valid accepted NR-08 baseline, median CPS changes
  +0.627% on `rxvm` and +0.246% on `rxbvm`, with elapsed changes -0.597% and
  -0.236%. Both ranges overlap, so the verdict is neutral-to-slightly-positive,
  not material. The candidate remains provisional/revertable and NR-09 stops
  for Adrian before broad validation, cleanup or Rule 2. Evidence:
  `evidence/2026-07-17-nr-09-numctx-first-release-verdict/`.
- 2026-07-18: Adrian accepted NR-09 Rule 1 as a small improvement. Exact
  pre-update audit proved 222 compiler goldens contained only 517 replacements
  of five setters by `NUMSCI`; four RXPA tests retained the same signal/source
  and moved only their expected bytecode address from 15 to 9. The failed
  selection passed 227/227 and final Debug CTest passed 1,852/1,852 in 205.57
  seconds. Rule 1 is retained and fully closed out.
- 2026-07-18: Adrian clarified that NR-09 low risk means RXAS-provable bounded
  dataflow (Class 1) or rxc-known compiler temporaries with irrelevant
  intermediate effects (Class 2), and that adding large instructions is the
  aim. The 76 selected patterns classify as 20 Class 1, 51 Class 2 and five
  deferred. Four Class 1 compare/branch mappings remain rejected by NR-07,
  leaving 67 active mappings across 12 coherent families. They will be
  designed and implemented as one attributable batch with one combined first
  Release verdict; overlapping RXSEQ windows are not additive.
- 2026-07-18: Class 1 ownership is RXAS-backed, not RXAS-exclusive. Every
  Class 1 mapping gets an RXAS peephole backstop. rxc may emit the large
  instruction directly when one AST-node emission naturally owns the whole
  mapping; mappings that would require coordination across merely adjacent AST
  nodes stay expanded in rxc and are collected by RXAS.
- 2026-07-18: The exact 76-ID disposition is retained in
  `NR-09-MAPPING-REGISTER.md`: 67 active, four prior NR-07 rejections and five
  deferred controls. Adrian selected one ordered batch: first add the large
  canonical instructions and direct tests, then RXAS Class 1 backstops, then
  simple single-AST rxc Class 1 emission plus rxc-owned Class 2 lowering. After
  focused proof, run one combined Release verdict and stop; full QA follows an
  accepted verdict under the programme gate.
- 2026-07-18: Froze the complete 67-mapping implementation after focused
  correctness and ran its combined first Release verdict. The 22 generated
  images lose 2,561 instructions (-19.851%); canonical matched profiles lose
  1,525,282/1,525,312 dispatches (-8.595%), but median CPS regresses 2.233% on
  `rxvm` and 2.289% on `rxbvm`. New VMs running exact retained old RXBINs are
  neutral (-0.060%/-0.069% CPS), so this originally appeared specific to the
  fused product rather than the larger VM core or arbitrary-operand decode.
  This cross-session interpretation is superseded by the 2026-07-19 rebaseline
  below. Stopped before broad QA, golden refresh or production-batch commit. Evidence:
  `evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/`.
- 2026-07-19: Added a rerunnable all-form profile report and decision ledger.
  It covers all 60 forms, observes 28 in the canonical profile and models
  1,525,298 retired dispatches, within +16/-14 of the exact VM count drops.
  Most observed forms have an instrumented saving signal; `SETTPCALL` is the
  first material possible-slowdown candidate at 56,968 calls and estimated
  +15.026%/+6.676% transition-aware cost. These profiler-biased estimates only
  prioritize the required ordinary-Release per-form review; they are not
  product verdicts. The ledger separately flags temporary-register-passing and
  independent-effect forms for a clear-advantage test.
- 2026-07-19: Completed the focused `SETTPCALL_REG_FUNC_REG_REG_INT` review.
  RXBIN/load/runtime inspection rejects a generic five-operand decode cost;
  the fused code segment is one eight-byte cell smaller and both VMs use fixed
  operand accesses. The former +15.026%/+6.676% signal compared this macro's
  calls with a population-mixed global `CALL` average and is not a valid
  classification. A balanced 16-offset ordinary-Release sweep (64 samples and
  5,000,000 calls per sample per VM) finds fused savings of 0.129 ns/call
  (-0.458%) in `rxvm` and 0.418 ns/call (-1.507%) in `rxbvm`. Individual
  `rxvm` layouts vary because the one-cell compaction shifts the post-call
  runtime-image address. A specialised call-body trial was slower in both VMs
  and was reverted; no production VM change survives. The standard report now
  marks call-bearing forms `exact-cell-required`, and the ledger retains
  `SETTPCALL` as coherent and implementation-reviewed.
- 2026-07-19: Reconstructed accepted commit `847e62f04` and reran the complete
  batch in a drift-controlled three-cell campaign: accepted VM/image (A),
  current VM/accepted image (B), and current VM/fused image (C). All 78 runs
  pass; 12 recorded rounds use every A/B/C permutation twice on each VM. The
  original -2.233%/-2.289% result does not reproduce. Complete-product paired
  median CPS is +0.586%/+0.671%, with elapsed direction agreeing and 95%
  intervals crossing zero. Infrastructure B/A is neutral at -0.152%/+0.022%;
  fusion C/B is slightly positive at +0.617%/+0.805%. The old candidate
  medians reproduce within +0.165%/-0.120%, while the old accepted baseline
  was 2.577%/2.364% faster than its rerun. The apparent slowdown was therefore
  unmatched-session baseline drift, not a reproduced VM or fused-product
  regression. Corrected verdict: neutral-to-slightly-positive; stop for Adrian
  before broad QA or production commit. Evidence:
  `evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/rebaseline/`.
- 2026-07-19: Completed the fact-based balanced review of all 60 provisional
  forms. The 22 optimized/no-opt images execute 76,122,675 combined
  instructions and exercise 33 forms. Thirty-two new profiling-off Release
  cells plus the retained `SETTPCALL` cell give every exercised form a dual-VM
  result; each new form/VM result contains 192 post-warm-up paired comparisons
  across all 16 eight-byte runtime-image positions and three fresh processes.
  The scorecard proposes 30 outright removals, two clean replacements and 28
  retained forms. All six caller/trace-temporary forms are removed or replaced:
  `FDIVSUB` and wide `ILOADSETUNLINKN` alone have double-digit evidence strong
  enough to justify redesign without their intermediate side effects. Swap/call
  retention is based on current evidence and explicitly discounted after the
  wider calling convention reduces swap demand. No production edit has been
  made; stopped for Adrian's approval. Evidence:
  `evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/balanced-review/`.
- 2026-07-19: Corrected the per-form review after Adrian challenged the
  no-usage inference. Both the source RXSEQ list and candidate census are
  post-RXAS, but runtime RXSEQ adjacency is not proof of static adjacency and
  an all-enabled zero can be caused by mapping order. Replaying the current
  combiner plus optimizing RXAS over the accepted 22-image pre-batch compiler
  RXAS recovers 501,000 `FMULTICOPY`, 5,569,610
  `LINKSETATTRSLINKADD`, 1,606,900 `SETLINKILOAD`, 1,652,680
  `UNLINKLINKATTR1` and 1,218 `LINKSETATTRS` executions. The last two remain
  valid overlaps with stronger retained forms. Alignment-balanced ordinary-
  Release head-to-head cells show the first three alternatives faster in both
  VMs; removing the remaining two-register `ITOF` from the selected arithmetic
  chain is slower in both. Revised proposal: 26 outright removals, two clean
  replacements and 32 retained forms. Production remains frozen for revised
  approval. Evidence: `evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/balanced-review/overlap-review/`.
- 2026-07-19: Adrian approved the corrected production pruning and design-only
  replacements. Removed 26 source-visible forms while preserving their numeric
  opcode slots as fail-safe reserved entries; selected the measured
  `FMULTICOPY`, `LINKSETATTRSLINKADD` and `SETLINKILOAD` paths and narrowed
  two-register `ITOF` to the winning arithmetic chain. Target-scoped Debug
  runtime regeneration and the focused 9/9 selection pass, as does the
  standalone example on both VMs. Both new Debug VMs also run the retained
  accepted old RexxCPS RXBIN and library successfully. Result-only `FDIVSUB`
  and compact TRACE-correct `ILOADSETUNLINKN` are recorded but not implemented.
  The corrected implementation is frozen for the mandatory ordinary-Release
  verdict before broad QA or commit.
- 2026-07-19: Completed the mandatory corrected-batch ordinary-Release verdict
  with equal-length accepted/candidate RXAS paths after retaining a first
  unequal-path pilot as non-decisive. All six warmups and 72 recorded samples
  pass. Corrected product C/B paired median CPS is +0.376%/+0.836%; complete
  C/A is +0.262%/+0.937%, with elapsed direction agreeing. The `rxbvm` C/B
  interval is wholly positive; both complete-product intervals cross zero.
  Verdict: no regression, neutral-to-slightly-positive. Stopped for Adrian
  before broad QA or commit. Evidence:
  `evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/prunedrun1/`.
- 2026-07-19: Completed accepted-batch QA. The broad compiler run exposed and
  fixed two semantic-boundary defects before golden refresh: an AST-side
  integer-alias shortcut could fuse a string copy in C2d/X2d, and native
  signal/unwind restore did not recognize the retained fused call forms. Alias
  cleanup fusion now occurs only over actual final typed instructions, with
  unit-proved TRACE retargeting; fused calls are included in cold restore.
  Full Debug CTest passes 1,864/1,864. The supported full Apple ASan run has no
  sanitizer finding; its one failure was a shared fixed-filename test race,
  corrected with a resource lock and a 2/2 parallel ASan rerun. Apple ASan
  rejects leak detection as unsupported. A 112-file isolated install executes
  the shipped example and both installed VMs pass against the exact accepted
  old RXBIN/library. The final same-session 78/78 Release refresh records
  complete-product paired median CPS +1.385%/+2.868%; `rxvm` uncertainty still
  crosses zero while `rxbvm` is wholly positive. Evidence:
  `evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/qa-closeout/`
  and `finalrun01/`. Production-batch commit remains pending.
- 2026-07-20: Audited final NR-09 documentation against the live assembler and
  corrected the remaining drift. The human reference now has exact coverage
  for 574 forms/367 mnemonics, including one standard section and validated
  example for each of the 25 large/fused mnemonics. All 373 tagged examples
  assemble; 328 execute and 45 are intentionally assemble-only. The compiler
  guide now requires matching actual final typed instructions rather than AST
  provenance, the VM guide includes all three fused calls in native cold
  unwind, and the mapping register records the final 34-form surface and broad
  QA state.
- 2026-07-20: Adrian marked NR-09 complete. The benchmark median index now
  records the final corrected-product Cell C medians of 1,211,556 CPS (`rxvm`)
  and 1,208,420.5 CPS (`rxbvm`) from the 12-round final QA refresh. There is no
  remaining NR-09 implementation, evidence, documentation or QA gate; the two
  rejected side-effect-preserving replacements remain separately recorded as
  future design ideas.

### NR-04A work notes

- 2026-07-16: Paused before commit and before declaring the first
  production-runtime performance change complete. The implemented eager
  runtime-only per-module kind index proves the target path is real: normalized
  `contract_kind` and `contract_implements` record visits fall by more than
  99%, and the existing interface-registry benchmark improves strongly on exact
  retained and stripped images. However, the first alternating portfolio pass
  and repeats also exposed unrelated stripped JSON and small/noisy lifecycle
  regressions. The programme had required evidence before hot-path changes but
  had not explicitly required an alternatives list and comparative lightweight
  prototypes before choosing a production design. `performance/AGENTS.md` now
  adds that gate. Treat the eager implementation and its retained bundle as
  candidate A. The option set now also includes link-produced runtime-critical
  tables, constant-pool seeds, split runtime/debug metadata, and hybrid seed plus
  legacy/direct/native fallback; clean serialized forms cross the RXBIN decision
  boundary and therefore remain design/PoC work until Adrian approves adoption.
  Compare viable candidates on the same images, file and retained memory size,
  link/load/teardown, late-load/plugin behavior, and both VM modes before
  selection or further production closeout.
- 2026-07-16: The full consumer audit showed that a generic kind index is not
  the final performance abstraction. Execution/procedure/source paths are
  already position- or head-addressed; the genuinely dynamic surface is the
  callable/type/class/interface/member/factory relationship model. Candidate
  A-prime isolated an unrelated code-layout regression, and candidate C proved
  that only the semantic kinds need retention while still performing 2,400,024
  implements-record visits for 100,001 negative relationship queries. The
  decision packet proposes a dense type universe, assignability and dispatch
  tables, factory buckets, per-site polymorphic caches, linker/RXAS seeds, and a
  generation-based late-load overlay, with future inheritance edges supported
  structurally but no inheritance semantics selected. Evidence is summarized in
  `evidence/2026-07-16-nr-04a-kind-index/options/MEASUREMENTS.md`; architecture
  options are in `NR-04A-RUNTIME-TYPE-DISPATCH-DESIGN.md`.
- 2026-07-16: Adrian selected T6 and approved an atomic RXBIN 007 break with no
  006 compatibility path. RXAS will emit a text-backed, dense-ID module graph;
  RXLINK will merge/reassign/reindex it into one sealed image graph and may use
  it during selection/validation; RXVM/RXBVM will borrow that seed and add a
  generation-published late/native overlay. Parameter/return type text lives on
  type nodes, signatures and hot relationships use local IDs, and built-ins
  such as `.float` are normal special nodes. A compiled common binutils library
  owns format/graph structure and fast traversal while language policy owns
  inheritance, assignability, override/default, and provider rules. The
  canonical design is `docs/ai-context/RXBIN_007_SEMANTIC_GRAPH.md`.
- 2026-07-16: Completed T6 milestone 1 without starting the performance-profile
  phase. RXAS and RXLINK now write only the fixed-width little-endian 007
  container; RXLINK rebuilds one sealed semantic graph and rewrites graph-bearing
  operands to linked IDs. The dedicated `rxbin` library owns checked format,
  signature, graph, and index code; `platform` remains OS-specific. RXDAS
  validates graph/container errors, both VMs consume numeric type/member/factory
  identities, and native/package surfaces—including installed `crexx`, `crxc`,
  and `ccomp`—link `librxbin.a`. A final 1,182-step Debug rebuild and all
  1,846 CTests passed; an installed-prefix `crexx -native` proof built and ran
  `explicit_header`. The next gate is measurement-led selection of physical
  layouts, site caches, and the late/native overlay, followed by Release,
  portfolio, sanitizer, and cross-platform closeout.
- 2026-07-16: Tightened the programme ordering after the first selected
  production performance change: once correctness is green, take an early
  end-to-end measurement before follow-on layout/cache prototypes or loose-end
  closeout, and stop to replan if the result is neutral, negative, or exposes a
  material regression. The T6/007 baseline is therefore the next decision gate;
  graph-layout harness refinement remains paused. Before that gate, correct the
  split-read `MTIME` defect and separate the explicitly noncanonical RexxCPS
  CTest smoke count from the no-argument canonical baseline.
- 2026-07-16: Completed that early gate and stopped. `MTIME`/`XTIME "U"` now
  use one coherent clock snapshot; RexxCPS rejects positional overrides,
  labels `--smoke-count` noncanonical, prints provenance, and the formal runner
  requires `contract=canonical-default`. Focused Debug build/tests are green.
  Audited retained evidence contains no 86,400-second or 6/12-CPS defect
  signature, so no 006 rerun is required. Against retained evidence, 007
  `rxvm` interface lookup is 81.7% slower retained and 76.9% slower stripped,
  its timed method/factory regions are 71.0-95.8% slower, canonical RexxCPS is
  14.0% lower, and the measured linked images are 3.24-4.64x larger. Raw 007
  evidence is in
  `evidence/2026-07-16-nr-04a-rxbin-007-early-baseline/`. No harness,
  layout/cache/overlay PoC, portfolio rerun, or closeout proceeds before
  Adrian's direction.
- 2026-07-16: At Adrian's direction, completed a holistic implementation and
  size review without changing the production representation. A standalone
  production-library harness shows exact type support at 0.9-1.2 ns but
  positive/negative transitive support at 32-73 ns because each query allocates
  and breadth-first-walks the graph. A scratch precomputed bit test and direct
  bound-target load both measure about 0.9 ns, proving the desired hot shape.
  Current graph dispatch/factory primitives are 3-5 ns, while RXVM still scans
  modules/procedures to bind portable targets on every selection. The T6 fields also enlarge every
  `value` from 256 to 272 bytes. Section attribution shows loss of 006 pool
  compression is the main whole-file expansion; all-`META_FUNC` graph scope and
  repeated semantic text remain material additional defects. The review in
  `NR-04A-RXBIN-007-IMPLEMENTATION-REVIEW.md` proposes one context-canonical
  type pointer, precomputed assignability bits, bound dispatch rows, direct
  factory buckets, three `TYPE_CONST`/descriptor ownership layouts, and R0-R4
  repair paths. Replacement PoCs are stopped for Adrian's selection.
- 2026-07-16: Adrian selected the C3-style/R1 rescue comparator: preserve the
  frozen 007 seed and current integration, materialize the process-optimal view
  inside `rxbin`, replace the four-field object type collection with one stable
  descriptor pointer, baseline immediately, and leave VM target binding for a
  later decision. Production descriptor support/dispatch now measures about
  0.92-1.07 ns; `value` is 248 bytes rather than 272. The exact-image `rxvm`
  interface delta is reduced to +5.3% retained/+7.0% stripped versus pre-007,
  while canonical RexxCPS reaches 1,129,206 CPS (+31.7% versus the retained
  clean O3 comparator). The remaining interface method delta is consistent
  with repeated `runtime_graph_procedure()` scans. Focused Debug validation is
  81/81 green; raw evidence is in
  `evidence/2026-07-16-nr-04a-c3-candidate/`. Work stops before choosing the
  callable-ID-to-`proc_runtime *` binding shape.
- 2026-07-16: Adrian selected one-time dense callable binding plus caches. The
  VM now resolves portable graph callables only while rebuilding a semantic
  generation, stores bound factory/provider rows, uses a two-way method-site
  cache, and caches factory buckets or the direct single-provider/no-match
  target. The first integrated factory result remained unexpectedly slow
  because RXAS/RXLINK had treated a valid short return type (`.lookup24`) as
  different from its canonical declared type and encoded two real
  `srcfprocsel` sites with an orphan providerless factory ID. The original
  harness selected a known-valid bucket and therefore measured approximately
  1 ns without auditing executable operands. Its new operand audit found both
  bad sites; semantic factory-signature remapping fixes them and the rebuilt
  image reports zero providerless sites. Final profiling-off `rxvm` medians are
  48.965 ms process, 37,044 us method and 7,905 us factory-region retained,
  improvements of 76.05%, 18.75% and 94.83% against exact pre-007 evidence.
  `SRCMETHODSEL` and `SRCFPROCSEL` both average about 14 ns in attribution
  profiles versus 21 ns/448 ns in 006. Focused Debug validation is 82/82 and a
  final canonical RexxCPS smoke reports 1,132,602 CPS. The 006-to-current audit
  found no remaining graph penalty on generic values (`sizeof(value)` is 248
  versus 256) and no other changed ordinary handler explaining a regression.
  Runtime performance passes this focused gate. Serialized size remains an
  independent defect: the interface images are still 4.474x/4.638x larger,
  chiefly because 007 stopped compressing the canonical pool and separately
  adds an approximately 26-KiB graph. Evidence and the stop boundary are in
  `evidence/2026-07-16-nr-04a-bound-cache/`. Iterate size candidates first in
  the standalone `rxbin`/RXAS/RXLINK/harness loop while preserving the selected
  hot view; integrated broad validation and final NR-04A reports follow only
  after a size candidate wins.
- 2026-07-16: Completed the approved disposable physical-layout PoCs against
  freshly assembled standalone and linked RXAS/RXLINK images. Restoring 006
  LZSS to canonical sections is the dominant win but leaves the interface image
  above 006. A linkable compact-full graph seed plus compressed non-graph
  sections reaches 31,880 bytes interface and 259,144 bytes RexxCPS; a terminal
  dynamic form reaches 31,624/254,832. Dynamic reconstruction takes 267 us/3.349
  ms (full 279 us/3.913 ms) and retains the control-cost hot view. A
  minimal link-resolved runtime seed using constant-pool text references plus
  inline fallback, direct relationship/declaration facts, dynamic procedure
  references, assignability, dispatch and factory/provider buckets reaches
  29,384/247,848 bytes and materializes in 5 us/28 us. Its type, dispatch and
  factory primitives remain at approximately 0.9 ns control cost; direct
  relationship buckets are 0.86-0.90 ns and indexed type-name search is 15-26
  ns. Keeping the current full seed beside it is a measured duplication-heavy
  upper bound of 33,340/262,762. The resolved-only result is
  3.61% above the retained interface 006 control and 4.50% below the RexxCPS
  control. No production format was changed. Evidence is in
  `evidence/2026-07-16-nr-04a-rxbin-size-options/`; stop for Adrian to select a
  single compact general graph, terminal resolved RXLINK output, or a separate
  resolved runtime plus narrower tooling/policy residual before integration and
  final reports.
- 2026-07-16: Adrian selected the contained production option: keep equivalent,
  complete and re-linkable RXAS/RXLINK 007 images, and compress every finished
  section independently through shared `rxbin` when it is smaller. Production
  sizes match the PoC exactly: 37,458 bytes retained interface and 273,858
  bytes retained RexxCPS, down 70.50%/68.52% from fresh uncompressed 007.
  Re-linking the interface output is byte-identical; both VMs pass. Median
  complete load rises from 79 to 106 us while hot graph access stays at about
  1 ns. Same-cell interface timing is within noise and canonical RexxCPS is
  1,120,626/1,135,200 CPS for `rxvm`/`rxbvm`. The obsolete layout PoCs are
  removed; selected evidence is in
  `evidence/2026-07-16-nr-04a-rxbin-007-compression/`.
- 2026-07-16: Closed NR-04A after correcting cross-image factory binding.
  Process-local graph buckets now aggregate signature-compatible providers from
  the validated global registry after linking, preserving higher-scoring
  providers in separately loaded 007 images without changing the hot selection
  shape. All six ADDRESS integration fixtures and the LLM fixture under
  `rxbvm` pass; the final Debug rebuild and 1,846/1,846 CTests pass. Adrian
  removed append-only-overlay, alternate-seed, sanitizer, cross-platform, and
  additional portfolio work from this activity's required closeout.

### NR-10/NR-11 work notes

- 2026-07-20 equal-work calibration found that the approved common work counts
  leave NetRexx at 0.050-0.272 seconds while the slowest cells are already
  2.330-15.569 seconds. Scaling the fastest cell to one second would breach the
  30-second cap for multiple workloads. Formal capture is paused before any
  retained samples pending an explicit normalized-throughput contract; see
  `NR-10-11-WORKLIST.md`.
- 2026-07-20 Adrian approved the bounded normalized-throughput exception:
  per-cell integer work targeting 3-10 seconds, two calibration points within
  5%, process-inclusive `work / elapsed`, and the unchanged formal sample and
  aggregation rules.
- 2026-07-20: Superseded that exception for the NR-10 common baseline after
  finding that the version-1 NetRexx ports used `options binary`. Corrected
  `options nobinary decimal`/`Rexx` ports qualify one equal argument per
  workload: fastest pilot medians are 1.062-1.182 seconds and slowest are
  1.614-16.061 seconds. The default HotSpot JIT remains canonical and fair.

- 2026-07-20: Started the combined resumable control plane at
  `NR-10-11-WORKLIST.md` from clean `develop` commit `1596d7c8c`, three local
  commits ahead of `origin/develop`. The NR-02 audit found 498 retained files,
  172 sample rows and 41/41 verified generated-form checksums. Its provenance,
  equivalence, opaque-input, ooRexx trace and NetRexx generated-form evidence
  remains reusable, but the bundles explicitly contain small non-formal pilots
  from older dirty cREXX states and do not provide formal power, uncertainty,
  RSS, complete artifact or native-C-ceiling evidence. A current bounded
  same-host campaign is required for factual NR-10 completion. The worklist
  records an evidence-backed NR-11 policy proposal and stops for Adrian before
  any threshold becomes normative or any formal run begins. Adrian confirmed
  that any new or changed performance tooling must be cREXX Level B, not
  Python; the Level B authoring guide is therefore a mandatory pre-edit gate.
  The final NR-10 evidence must also be compact: one consolidated bundle,
  existing NR-02 forensics reused by reference, and no repository retention of
  calibration, superseded scratch runs or reproducible duplicate build output.
- 2026-07-20: Adrian approved the policy proposal as drafted. The normative
  authority is `PERFORMANCE-GOVERNANCE.md`; mandatory future-agent rules are in
  `AGENTS.md`, operational navigation is in `README.md`, portfolio/comparability
  ownership is in `portfolio/cross-runtime-plan.md`, and the publication shape
  is `templates/performance-scorecard.md`. The approved initial common
  aggregate is Sieve, Permute, Bounce, Richards and Base64. Formal capture and
  a template-proving scorecard remain before NR-10/NR-11 completion.
- 2026-07-20: Closed NR-10 and NR-11 on the corrected Apple M5/macOS formal
  bundle at `evidence/2026-07-20-nr-10-formal-baseline/`. The canonical common
  and lifecycle paths retain 270 equal-work timing, 70 equal-work RSS and 160
  decimal-lifecycle rows with zero correctness failures; 24 labelled native-C
  control rows also pass. Required noise appends retain every extreme. The N=5
  equal-work geometric means are 0.883021/0.327772 for `rxvm` versus
  ooRexx/decimal NetRexx and 0.872707/0.323944 for `rxbvm`. The original
  0.006220/0.006149 NetRexx ratios are withdrawn as binary/JVM controls. The
  bundle inventories 101 artifacts, reuses NR-02 forensics by reference and
  removes reproducible C binaries plus the superseded unequal-work decimal
  capture. The Level B inventory writes and verifies 70 recursive checksums;
  final Release benchmark validation is 35/35.

## Cross-runtime portfolio execution order

The target is to run every approved Tier A workload on CREXX, ooRexx and
NetRexx. Regina runs RexxCPS only. Use explicit `not comparable` entries rather
than invented or semantically misleading ports. The detailed matrix and
coverage gaps are in
[`portfolio/cross-runtime-plan.md`](portfolio/cross-runtime-plan.md).

1. **NR-01 — complete coverage and port inventory.** Expand the seed portfolio,
   identify the source/port needed for CREXX, ooRexx and NetRexx, and classify
   each cell as canonical, equivalent port, disclosed adaptation, control or
   not comparable. Regina needs only the RexxCPS lane; Java/C remain selected
   controls rather than a mandatory full matrix.
2. **NR-02 — port, qualify and pilot-run comparisons.** Complete for the
   approved portfolio. Preserve the explicit `not comparable` and disclosed-
   adaptation decisions when transferring cells to NR-10/NR-11.
3. **NR-10 — retain formal comparative baselines.** Do not wait for the final
   workload before measuring: add same-host raw results implementation by
   implementation whenever a workload clears NR-02, while keeping incomplete
   matrices visibly incomplete.
4. **NR-11 — publish a scorecard.** Aggregate only the common qualified
   CREXX/ooRexx/NetRexx workload set; keep Regina's RexxCPS result and other
   non-common workloads visible but outside that portfolio aggregate.

## P1 bounded production register

| ID | Activity | Status | Current note / boundary |
| --- | --- | --- | --- |
| NR-12 | Extend read-only by-value and return copy coalescing | deferred | RXAS inspection is complete: never-written real formals already alias `aN`; unconditional overwrite exposes DSE, but conditional/loop isolation needs the forthcoming flow analysis. No implementation is selected; see `NR-12-21-WORKLIST.md`. |
| NR-13 | Redundant numeric-context setup elimination | complete | Accepted NR-09 Rule 1 satisfies the NR-13 exit criterion: the compiler uses existing `NUMSCI`/`NUMENG` only for an identical fully constant non-inherited effective context, focused cross-procedure/plugin coverage passes in both modes and VMs, and dynamic setup fell from 542,500 to 108,508 per VM. General full-context and procedure-owned-default experiments remain optional NR-23/NR-24 work, not unfinished NR-13 scope. |
| NR-14 | Static/frozen `PARSE` lowering fast path | complete | Adrian accepted the frozen hybrid and the +249,179 B/+20.579% compiler-exit artifact trade-off. The three exact opcodes remain preferred, `parsewords3` chains eligible longer odd word templates, compact opcode 410 `parseplan` covers remaining mechanically frozen plans, logging/TRACE/INTO fall back, and regex is untouched. Generic elapsed is -90.757%/-90.744%, exact elapsed -97.262%/-97.423%, and RexxCPS CPS +45.249%/+45.499%; capped byte-identical Richards is neutral. Closeout passed focused Debug/ASan 15/15, full Debug 1,876/1,876, and isolated installed/native-package proof. See `NR-14-WORKLIST.md` and `evidence/2026-07-21-nr-14-hybrid-first-release-verdict/`. |
| NR-15 | General stem default/reset fast path | queued | Preserve generation/default/drop/tail semantics. |
| NR-16 | TRACE-off and same-ADDRESS-environment fast paths | queued | Preserve hooks, signals, mode changes and host callbacks. |
| NR-17 | Link-time direct provider/call resolution | queued | Preserve late-load and plugin fallback. |
| NR-18 | Safe RXAS rule harvest | queued | Continue beyond the completed NR-09 batch only with opcode effects/liveness proof and generated rule tests, retaining rejected/deferred ledger entries. |
| NR-19 | Optional C LTO/PGO/code-layout experiment | queued | Optional build feature; adopt only with repeatable supported-platform evidence. |
| NR-20 | Value/frame allocation counters and targeted pooling | queued | Gather counters first; preserve ownership and sanitizer gates. |
| NR-26 | Typed semantic flow analysis in `rxc` | complete | Adrian accepted the F1/F2/P1 panel on the correctness-plus-instructions gate. Final focused 8/8 and broad Debug 1877/1877 pass; the exact 19-image census remains 50,965 to 50,924 instructions (`copy -11`, `icopy -30`). The corrected RexxCPS artifact passes a narrow same-session drift control with no 3% guard hit. No RXAS annotations, ISA, RXBIN or ABI change. See `NR-26-WORKLIST.md`. |
| NR-27 | Whole-procedure machine flow analysis in RXAS | complete | Adrian accepted the annotation-free RXAS panel on the mathematical-correctness plus instruction gate: 46,469 -> 45,476 instructions (-993) and Richards executes 59,880 fewer instructions per VM. Formal profiling-off Release timing is no-regression but mostly neutral/noisy: five-workload median-throughput geometric mean -0.552%/+0.323% (`rxvm`/`rxbvm`), with only Richards/`rxbvm` clearly favorable at -0.238% paired elapsed. No 1% aggregate or 3% workload guard is hit. Full Debug build and CTest 1,885/1,885 pass; see `NR-27-WORKLIST.md`. |

## P1 architecture-neutral prototypes

These are time-boxed evidence activities, not production commitments.

| ID | Prototype | Status | Adoption question |
| --- | --- | --- | --- |
| NR-21 | Mapped-call descriptor | complete | Adrian accepted permanent `CALL1...CALL4` with enforced RXBIN 007 feature bit 0 after the first Release verdict improved List about 6% and Permute 3-4% in both VMs, with neutral Richards, no JSON control regression and smaller images. The descriptor alternative was rejected; complete Debug build, focused 17/17 and final broad CTest 1,871/1,871 pass. NR-12 remains deferred. See `NR-12-21-WORKLIST.md`. |
| NR-22 | Compact runtime execution stream | queued | Does it improve total portfolio time on at least two architectures with canonical RXBIN unchanged? |
| NR-23 | Runtime quickening/superinstructions | queued | Use the completed NR-09 evidence when comparing private-runtime synthesis with canonical new opcodes; can dequickening, signals, TRACE/debug, late load and both VM modes remain correct? |
| NR-24 | Profile-selected fusion | queued | Extend the completed NR-09 evidence only where RXSEQ-selected semantic units repeat across the portfolio without code-size explosion and beat compiler lowering or procedure defaults. |
| NR-25 | Value hot/cold split or pool allocator | queued | Do cache/allocation counters justify the ownership and complexity cost? |

## Idea ledger

### NR-27 work notes

- 2026-07-22: Started from clean synchronized `develop` at `25cbca679`, an
  accepted one-commit successor to the requested `4ab5f3d8d` baseline. The
  intervening commit is the completed NR-26 typed semantic-flow panel; NR-27
  remains independent, consumes only ordinary RXAS plus canonical opcode
  effects and adds no compiler annotations or private contract.
- 2026-07-22: Adrian replaced the per-slice first-Release timing stop with an
  activity-specific panel-first gate. Complete and review the bounded NR-27
  transformation panel using mathematical semantic correctness and instruction
  reduction as the production-candidate gates. Only after the panel is complete
  run the consolidated profiling-off Release baseline under the normal formal
  sampling and regression policy. Negative/rejected candidates remain in the
  panel record; instruction reduction is proof of removed machine work, not a
  wall-clock claim.
- 2026-07-22: Selected a hybrid procedure-stream design. Keep the existing
  20-item local peephole semantics unchanged, retire its stable output into a
  dynamically sized internal per-procedure stream, and construct blocks, CFG
  edges and bitset dataflow over that stream before ordinary RXBIN emission.
  This is the smallest representation that preserves old rule boundaries while
  supporting hand-written and compiler-generated RXAS equally. Sparse def-use
  chains are derived only where needed; SSA is deferred because it adds rename,
  metadata and lowering complexity without being required by the initial panel.
- 2026-07-22: Froze the completed panel on the mathematical-correctness and
  instruction-reduction gate. Accepted bounded forms are typed `icopy`/`fcopy`
  and strict-string-use `scopy` propagation, identity full copy, exact repeated
  int/bitwise-float load, `null`/context-free `itof`, complete-CFG reachability,
  and the typed-copy compare subset. `dcopy`, nonidentity full copy, broader
  loads/conversions and conversion-to-compare remain deferred. P3 dead-result
  deletion was removed and rejected because a nominal numeric write can release
  hidden reference/native-payload state. Unknown or jump-table indirect control
  flow now disables all NR-27 rewrites for that procedure.
- 2026-07-22: Final pre-timing evidence is exact. The 19-image optimized-source
  census is 46,469 -> 45,476 (-993) across nine changed images with no growth:
  979 unreachable instructions and 14 typed copies over 154 procedures. The
  retained pre-NR-27 versus candidate Richards image executes
  9,179,035 -> 9,119,155 instructions (-59,880, -0.652356%) identically in
  `rxvm` and `rxbvm`, with matching PASS/queue/hold results. An isolated
  hand-RXAS panel executes 28 -> 22 instructions in each VM. The panel is now
  frozen for the consolidated ordinary profiling-off Release verdict.
- 2026-07-22: The consolidated ordinary profiling-off Release verdict is
  retained under `evidence/2026-07-22-nr-27-panel-verdict/`. All 708 executions
  pass. Governed expansion reaches 34 paired rounds for Sieve, Permute, Bounce
  and Base64 and 36 for Richards. The equal-weight five-workload median-
  throughput geometric mean is -0.552%/+0.323% (`rxvm`/`rxbvm`), within the 1%
  guard; no workload paired median hits the 3% guard. Richards/`rxbvm` is the
  only clear lane (-0.238% median elapsed, mean 95% interval -0.705% to
  -0.097%); every other interval crosses zero. NR-27 is at the mandatory first-
  Release decision stop, provisional and uncommitted, without a release-wide
  speedup claim.
- 2026-07-22: Adrian accepted the panel and requested local commit followed by
  a separate opportunity review. The unchanged closeout passes a full Debug
  build and 1,885/1,885 CTests in 145.65 seconds; evidence checksums and
  `git diff --check` pass. NR-27 is complete without a release-wide speedup
  claim. No push was requested.

### NR-12 / bounded NR-21 work notes

- 2026-07-20: Started the ordered call-convention investigation from clean
  `develop` commit `5626d6b87`. The retained NR-05 CSV reproduces 14,959,323
  arity-zero-through-four calls out of 16,439,420 (90.996659% overall,
  87.792028% optimized and 92.868653% unoptimized). JSON deliberately remains
  the high-arity fallback/non-regression case. The resumable audit, design,
  guarded-PoC, protected-input and inlining gates are in
  `NR-12-21-WORKLIST.md`. No public opcode, serialized RXBIN, ABI or
  architectural change is selected by starting this work.
- 2026-07-20: The default-off fixed-arity direct-bytecode PoC passed the
  opt/no-opt two-VM portfolio, focused arity/repeated/overlap/status/reference/
  signal tests, effects metadata and assembler/linker/disassembler round-trip.
  It preserves the ordinary callee `a1...aN` contract and status flags while
  removing eligible call-window marshalling. The 11-source static portfolio
  falls 262 optimized and 333 no-opt executable instructions with identical
  `.locals`, register ceilings, copy counts and non-marshalling opcode choices.
  A bounded Release capture is clearly positive for List and Permute in both
  VMs and neutral/noisy elsewhere; this is design-selection evidence, not the
  mandatory production verdict. NR-12 implementation is deferred to flow
  analysis, and inline formal/result scaffolding remains a separate follow-on.
- 2026-07-20: Adrian approved the fixed direct-bytecode design and explicit
  RXBIN compatibility policy. The provisional production implementation uses
  `CALL1...CALL4` at IDs 401-404 and enforced
  `RXBIN007_FEATURE_FIXED_CALLS`; zero-feature RXBIN 007 remains readable and
  unsupported/missing features fail closed. NR-06 affinity, dynamic/native
  calls, NR-12 and inline lowering remain out of scope. The implementation is
  frozen after minimum focused correctness for the mandatory ordinary-Release
  verdict.
- 2026-07-20: The frozen ordinary profiling-off Release comparison passed
  focused Debug CTest 17/17 and the pre-timing smoke matrix 16/16. Across 12
  recorded paired rounds, List improved 6.015%/5.784% and Permute
  3.849%/3.224% in `rxvm`/`rxbvm`; Richards was neutral and the high-arity JSON
  control improved 0.904%/1.983%. All candidate linked images were smaller and
  every timing cell declined a rerun. First production verdict: **ACCEPT**.
  Exact evidence is retained in
  `evidence/2026-07-20-nr-21-first-release-verdict/`. Work stops here pending
  Adrian's direction; NR-12 remains deferred for flow analysis.
- 2026-07-20: Adrian accepted the first production verdict and approved the
  shortest QA/documentation closeout plus a local commit. The complete Debug
  product and focused 17/17 set passed. Broad CTest first exposed 96 expected
  compiler golden mismatches; the documented refresh changed only fixed-call
  lowering, standalone status setup and four consequent unfused link forms,
  with no `.locals`, metadata, source/TRACE or unrelated opcode drift. Final
  high-parallel Debug CTest passed 1,871/1,871. Compiler, RXAS, RXBIN and VM
  documentation now agree on selection, feature enforcement, expected status
  flags and the ordinary callee `a1...aN` view. NR-21 is complete; NR-12 stays
  deferred for flow analysis. QA evidence is under
  `evidence/2026-07-20-nr-21-first-release-verdict/qa-closeout/`.

### NR-14 work notes

- 2026-07-20: Started the static/frozen PARSE investigation from clean local
  `develop` commit `8424587f2`, intentionally two commits ahead of
  `origin/develop`. `NR-14-WORKLIST.md` records the semantic inventory,
  retained-evidence audit, machine-level ceiling and guarded A-E comparison.
  Adrian explicitly authorized runtime-assist RXAS/RXBIN/linker/disassembler
  and both-VM PoCs and productionization of the fastest validated NR-14 design;
  generic `parseExec` remains the complete unsupported/dynamic fallback. No
  production design is selected by starting the work.
- 2026-07-20: The A-E comparison selected D, three exact direct-result opcodes
  with RXBIN 007 feature bit 1 and fail-closed compiler eligibility. Focused
  Debug and candidate Release checks pass 11/11. In 12 paired profiling-off
  Release rounds, canonical RexxCPS native CPS improved by 45.965%/45.826% and
  focused frozen-PARSE elapsed fell by 97.248%/97.445%, favorable in every pair
  on both VMs. The byte-identical Richards control is neutral/noisy on `rxvm`
  at the 36-pair cap and slightly favorable on `rxbvm`; sub-5 ms lifecycle is
  noisy/inconclusive, while RSS is slightly lower. First verdict recommends
  **ACCEPT**, but the 131,483-byte/10.859% `rxcexits.rxbin` increase crosses the
  artifact guard and needs Adrian's explicit trade-off decision. Evidence is
  `evidence/2026-07-20-nr-14-first-release-verdict/`. Work stops here with the
  implementation provisional; no broad closeout, commit or push has run.
- 2026-07-21: Adrian required generic mechanically frozen coverage while
  retaining benchmark-fast exact paths. Implemented the hybrid with opcode 410
  `parseplan`, a compact portable descriptor/reusable vector, and exact
  `parsewords3` chaining where faster; regex remains untouched. Six balanced
  PoC rounds select prepared over `parseExec` and exact chaining over prepared
  for the eligible odd-word class. Focused Debug and Release pass 13/13. The
  new first Release verdict records -90.757%/-90.744% generic elapsed,
  -97.262%/-97.423% exact elapsed and +45.249%/+45.499% RexxCPS CPS, all 12/12
  favorable; byte-identical Richards is neutral at 36 pairs. ACCEPT is
  recommended, pending explicit approval of the +249,179 B/+20.579%
  `rxcexits.rxbin` increase. Evidence:
  `evidence/2026-07-21-nr-14-hybrid-first-release-verdict/`.
- 2026-07-21: Adrian accepted the hybrid verdict and explicitly approved the
  compiler-exit/RXAS artifact trade-off, full closeout and a local commit, with
  no push. Disposable PoCs were removed and the four instructions documented.
  Focused Debug including RXAS reference examples passes 15/15; the full Debug
  suite passes 1,876/1,876. The complete supported Apple ASan build passes and
  the same focused surface passes 15/15; Apple ASan rejects leak detection, so
  LSan is not claimed. The ordinary Release product rebuilt, isolated install
  placed 131 files, installed native `hello` packaging executed successfully,
  and installed `rxvm`/`rxbvm` both pass the generic frozen-PARSE workload.
  NR-14 is complete; `qa-closeout/` records the final gate.

### NR-26 work notes

- 2026-07-21: Started NR-26 from clean `develop` commit `4ab5f3d8d`, exactly
  equal to `origin/develop`. The design comparison selected a procedure-local
  CFG overlay over the final typed AST: direct AST annotations do not provide
  a reusable join model, while a new lowering IR or SSA would add machinery
  not justified by the first transformations. The initial bounded production
  batch will share one must-write-before-read proof between scalar default
  initialization removal and NR-12 scalar by-value entry-copy removal. The
  layer will retain separate source-symbol, compiler-symbol/temporary and
  eventual physical-register identities, reject uncertainty only for the
  affected value/site, and emit ordinary self-contained RXAS. The resumable
  design, proof and mandatory first-Release stop are in `NR-26-WORKLIST.md`.
- 2026-07-21: The first F1/F2 Release result was directionally favorable but
  noisy/inconclusive at the governed 36-pair cap. Adrian selected REVISE and
  directed NR-26 to build a stronger transformation panel under correctness
  plus exact instruction-avoidance gates, deferring the next formal performance
  sweep until the panel is complete. Discovery now prioritizes the retained
  compact `ILOADSETUNLINKN` opportunity, compiler-owned dead/copy scaffolding
  and flow-sensitive constant/copy propagation; result-only `FDIVSUB` remains
  a separate ISA/RXBIN semantic decision.
- 2026-07-21: Stronger panel slice P1 now passes the construction gate. Exact
  small-scalar must-copy propagation, dead-copy fixed point and guarded
  incoming-slot sharing avoid 41 instructions across the bounded 19 optimized
  images: 35 in five benchmark workloads and six in three tool selftests, with
  the complete mnemonic delta limited to `copy -11` and `icopy -30`. Focused
  optimized/no-opt and both-VM correctness pass, as do the 11 changed or
  retargeted Release images in both VMs. Compact `ILOADSETUNLINKN` is deferred
  because its loaded operand is presently required by register-backed literal
  TRACE after alias unlink; immediate constants have the same observation
  boundary, and `FDIVSUB` still needs an ISA decision. Counted-loop count-copy
  reuse exposes only four static setup copies and is not selected for this
  panel because it adds a new destructive loop-state proof surface. F1, F2 and
  P1/F3 are now the frozen three-transformation no-architecture panel; the one
  combined formal performance sweep is next. Exact construction evidence is under
  `evidence/2026-07-21-nr-26-panel-construction/`.
- 2026-07-21: The frozen three-transformation panel's combined ordinary
  profiling-off Release sweep completed at the governed 36-pair cap over the
  five instruction-changing benchmarks plus canonical RexxCPS, under both
  current VMs. All 888 executions pass. Permute/rxbvm is clearly favorable
  (paired median -0.662%, mean 95% interval -2.438% to -0.375%); all other
  intervals are noisy/inconclusive, none is wholly unfavorable, no 3%
  per-workload guard is hit, and every candidate linked image is smaller.
  ACCEPT is recommended on Adrian's selected correctness-plus-instructions
  gate without making a release-wide speedup claim. NR-26 remains frozen,
  provisional and uncommitted at the mandatory decision stop. Evidence:
  `evidence/2026-07-21-nr-26-panel-first-release-verdict/`.
- 2026-07-22: Adrian accepted the panel and authorized the shortest local
  gate-to-commit closeout. Broad validation exposed and corrected two
  fail-closed boundaries before landing: counted-loop traversal is now kept
  within exact CFG block ownership, and generated semantic `VAR_REFERENCE`
  selector nodes are always treated as real reads. Focused regressions cover a
  backedge kill and opaque jump dispatch; the final focused set passes 8/8 and
  full Debug CTest passes 1877/1877. All 19 optimized source-RXAS images retain
  the exact 41-instruction reduction. A 24-byte RexxCPS linked-image change
  from the corrected standard library was controlled in a same-session
  old/corrected run under both VMs: all 52 executions pass, both paired
  intervals cross zero and no 3% guard is hit. Final evidence is under
  `evidence/2026-07-22-nr-26-closeout/`; NR-26 is complete with no new
  release-wide speed claim.

Ideas remain here until assessed, even when rejected. An idea can inform more
than one `NR-*` activity and does not alter their priority by itself.

### IDEA-PARSE-01 - Compiled immutable PARSE execution

- Status: complete; accepted hybrid under NR-14
- Related activities: NR-14, NR-22, NR-23, NR-24
- Hypothesis: mechanically eligible frozen PARSE templates can execute close to
  direct string-scan/assignment cost when the runtime consumes a compact
  immutable plan or exact hot primitive, avoiding plan-text interpretation,
  name lookup, generic descriptor traversal and avoidable result allocation.
- Variants: compiler expansion into existing operations; specialized
  `parseExec`; a canonical compiled-plan opcode; direct explicit-target
  instruction forms; and linker/load-time private preparation compared with
  serialized RXBIN assistance.
- Semantic risks: source/modifier evaluation order, literal and positional
  template edge cases, empty/unmatched fields, repeated targets, source/target
  aliasing, dynamic patterns, Unicode/string behavior, error ordering, TRACE,
  source coordinates, late load and dual-VM parity.
- Evidence needed: supported/fallback population, exact machine-level ceilings,
  focused optimized/no-opt dual-VM semantics, assembler/linker/disassembler and
  compatibility proof where applicable, paired RexxCPS plus another PARSE
  workload, unrelated control, startup/load, allocation/RSS and artifact cost.
- Resumable control plane: `performance/NR-14-WORKLIST.md`.

### IDEA-NR09-NUMCTX-01 — Procedure-entry numeric-context synthesis

- Status: NR-13 complete through accepted existing `NUMSCI`/`NUMENG` lowering;
  wider variants remain optional architecture work under NR-23 and NR-24
- Related activities: NR-09, NR-13, NR-23, NR-24
- Hypothesis: repeated procedure-entry numeric setup should be performed once
  as a semantic unit, or installed as the procedure's frame-entry default,
  instead of dispatching five independent setters on every activation.
- Current leverage: `rxc` emits up to five constant `SETNUM*` instructions for
  non-inherited values, while RXBIN 007 and both VMs already implement
  `NUMSCI`/`NUMENG` for the common fuzz-zero scientific/engineering cases. A
  child frame first copies the caller numeric context and the prologue then
  overwrites declared settings; each setter also resynchronizes the decimal
  plugin.
- Variants: emit existing `NUMSCI`/`NUMENG`; add a full five-field synthesized
  operation; install non-inherited context from procedure/runtime metadata at
  frame activation; synthesize only in the private runtime image while keeping
  canonical RXBIN unchanged.
- Semantic risks: explicit `INHERITED`, invalid-value signal ordering, decimal
  plugin ownership/synchronization, recycled frames, inline compatibility,
  source-step/TRACE coordinates, late loading and both VM modes.
- NR-13 evidence: accepted Rule 1 proves identical-effective-context
  eligibility, inherited/nonzero-fuzz/small-digits fallback, cross-procedure
  behavior, both optimization modes, both VMs, all three decimal-plugin modes,
  and an exact dynamic setup reduction from 542,500 to 108,508 per VM. Its
  broad closeout passed 1,852/1,852; the current post-NR-21 suite passes
  1,871/1,871.
- Remaining decision gate: a canonical full-context opcode, RXBIN metadata
  contract, procedure-owned frame default or private-runtime synthesis is a
  separate NR-23/NR-24 architecture choice. It requires comparative evidence
  and Adrian's selection but does not keep NR-13 open.
- 2026-07-20 closure review: the historical NR-13 exit criterion requires
  identical effective context, cross-procedure numeric tests and dynamic-count
  reduction. Accepted NR-09 Rule 1 supplies all three, so Adrian's requested
  review marks NR-13 complete without another implementation or timing run.

### IDEA-META-01 — Link-produced runtime-critical metadata table

- Status: subsumed by selected T6/RXBIN 007 semantic graph
- Related activities: NR-04A, Strand 8 link/load work
- Hypothesis: `rxlink` already traverses and rewrites every selected metadata
  record, so it can emit compact per-module runtime kind spans/offsets/ordinals
  more cheaply than making every VM rediscover them at load.
- Semantic boundary: preserve the canonical metadata chain and its ordering,
  duplicates, first-match behavior, source/TRACE diagnostics, and generic
  introspection. Shared-pool modules need distinct per-module descriptors.
- Compatibility boundary: format 006 has no extension directory or seed pointer
  and rejects unknown record types/flags. RXBIN 007 is now approved as an atomic
  break; old images are rebuilt, while late-load and native definitions enter
  the common T6 overlay.
- Evidence needed: linked file-size delta, link time, load-to-first-result,
  runtime allocation/retained bytes, exact hot lookup cells, stripped unrelated
  cells, both VM modes, 006 rejection/rebuild coverage, rxdas/tool support, and
  native/late overlay behavior.

### IDEA-META-02 — Constant-pool seed and hybrid metadata index

- Status: rejected as the primary shape; separate 007 graph sections selected
- Related activities: NR-04A, IDEA-META-01
- Hypothesis: store a compact index payload with the constant-pool lifetime and
  let a module point directly to it, avoiding a runtime copy/allocation; use a
  runtime-built fallback only where no seed exists.
- Design variants: a clean new header pointer/new constant type; assembler seed
  rewritten by `rxlink`; link-only seed; or a 006-compatible reserved metadata
  convention. The last variant is suspect because it leaks a hidden reserved
  record into canonical introspection and still needs discovery.
- Evidence needed: fixed-width/cross-platform encoding, validation cost,
  compressed/shared-pool size, zero-copy feasibility after decompression,
  fallback equivalence, and the same performance/lifecycle matrix as
  IDEA-META-01.

### IDEA-DISPATCH-01 — Runtime type universe and polymorphic dispatch tables

- Status: selected as T6 under NR-04A; milestone 1 implemented, physical-layout
  and overlay/cache PoCs pending
- Related activities: NR-04A, IDEA-META-01, IDEA-META-02, Strand 8 link/load work
- Hypothesis: a linker-seeded type universe with dense TypeId/MemberId identities,
  assignability data, class/interface dispatch rows, factory provider buckets,
  and per-instruction-site late-binding caches will outperform generic metadata
  indexing while providing one coherent foundation for polymorphism and future
  inheritance.
- Affected surfaces: RXAS/RXLINK semantic validation, RXBIN runtime-data
  serialization, loader binding, object runtime type identity, ISTYPE/ASSERTTYPE,
  dynamic method/factory selector opcodes, RXVML, native/late load, profiling,
  and both VM modes.
- Semantic risks: current multi-interface/default/factory scoring and tie-break
  rules; duplicate/module ordering; future inheritance/override/default-conflict
  rules; generation and publication during late/native load; and complete 006
  artifact rejection/rebuild coverage.
- Evidence needed: direct assignability, monomorphic/polymorphic/megamorphic
  dispatch, factory bucket/match, seed size/load/bind, late-load invalidation,
  retained memory, and full exact portfolio results. Separate required user
  `match` execution from avoidable lookup/descriptor overhead.
- Design record: `performance/NR-04A-RUNTIME-TYPE-DISPATCH-DESIGN.md`.

### IDEA-CALL-01 — Protected by-value input and call-flag simplification

- Status: assessed; implementation deferred pending flow analysis
- Related activities: NR-05, NR-06, NR-12
- Hypothesis: strengthen the compiler invariant so a by-value formal proved
  read-only is never the destination of compiler-generated writes and always
  reuses the incoming argument register on every supported call/inlining path.
  With that invariant explicit, measure whether compiler call signalling and
  defensive-copy branches can be narrowed, and whether any uses of
  `REGTP_VAL` / `REGTP_NOTSYM` can be removed or replaced for non-optional,
  read-only calls.
- Existing foundation: `mark_const_args()` in `compiler/rxcp_opt.c` already
  sets `is_const_arg` for provably read-only by-value formals in optimized and
  no-opt builds. `compiler/rxcpemit.c` already avoids their defensive copy, and
  inlining has matching read-only/writable handling.
- Inspection result: real never-written formals already bind directly to the
  incoming `aN`. Write-before-read has a genuine dead eager copy; read-then-write
  only moves it; conditional/loop writes require path-correct binding and join
  facts. Adrian selected postponement until the forthcoming flow analysis.
- Important distinction: `REGTP_VAL` also signals that an optional argument was
  supplied, while `REGTP_NOTSYM` lets a writable large-value formal reuse a
  temporary not backed by a caller-visible symbol. The current flags therefore
  cannot be removed wholesale merely because read-only formals alias safely.
- Semantic boundary: plain `ARG` remains caller-visible pass by value;
  `ARG expose` remains pass by reference. The assessment must include scalar,
  string, binary, array, object and explicit reference values; optional/default
  arguments; repeated actuals; mutation/rebinding; inlining; returns; signals;
  recursion; dynamic/native calls; and both optimized/no-opt output.
- Evidence needed: call census by formal kind; generated RXAS counts for
  `SETTP`, copy/swap and flag tests by static site; proof that every read-only
  formal path has no write; paired portfolio timings; and a written ABI/signal
  unwind impact assessment before any flag-contract change.
- Key pointers:
  `compiler/docs/emitter_architecture.md` section 4.1,
  `docs/books/crexx_language_reference/procedures_and_arguments.md`,
  `docs/ai-context/RXVM_INTERPRETER.md` copy/move/call sections,
  `compiler/rxcp_opt.c` (`mark_const_args`),
  `compiler/rxcpemit.c` (`ARG` emission),
  `compiler/rxcp_emit_expr.c` (call marshalling),
  `binutils/include/rxflags.h`, and
  `compiler/tests/rexx_src/inline_test_byvalue_arg_reuse.crexx`.
- Decision gate: first assess as a compiler-only copy/signalling simplification
  preserving the current ABI. Any language, public ABI or signal-unwind change
  requires Adrian's explicit approval.

### IDEA-NUMERIC-01 — Native typed Level B numeric siblings and RexxCPS type fidelity

- Status: complete; promoted to the P0 no-regrets register as NUMERIC-01
- Related activities: NR-01, NR-02, NR-10, NR-13
- Hypothesis: native `.int` and `.float` siblings for the appropriate decimal
  value and formatting BIFs can remove avoidable decimal crossings, while an
  explicit 9-digit Classic numeric context and a semantic type audit can make
  the cREXX RexxCPS port more faithful. Correctness, Classic fidelity and
  performance remain three independent verdicts.
- Affected surfaces: the public `rxfnsb` numeric surface and documentation,
  focused optimized/unoptimized library tests, canonical and opaque RexxCPS
  sources, generated RXAS/RXBIN inspection, and exact-hash benchmark evidence.
- Semantic risks: public naming without polymorphism; homogeneous variadics;
  signed-integer minimum absolute value; float signed zero, NaN and infinity;
  `TRUNC`/`FORMAT` text contracts; and preserving genuine Classic decimal
  arithmetic under `NUMERIC DIGITS 9` instead of selecting a faster type.
- Evidence needed: approved signature/semantics matrix; isolated A/B/C/D
  candidate cells; source/image hashes; numeric-context, opcode, conversion and
  call counts; focused correctness on both VMs; and the mandatory profiling-off
  Release verdict with benchmark-native CPS kept separate from process time.
- Resumable control plane: `performance/NUMERIC-01-WORKLIST.md`.
- First Release evidence:
  `performance/evidence/2026-07-17-numeric-01-first-release-verdict/`. Focused
  correctness and Classic fidelity pass; C removes avoidable decimal BIF
  crossings. B/C versus A has a small positive mean on both VMs but overlapping
  ranges; C versus B is neutral/noisy because those calls sit outside the
  default timed clause loop. The native surface remains independently useful.
- Decision: Adrian accepted C. RexxCPS 2.2d, the native Level B surface and the
  standard-benchmark purity fixes completed focused, exact-hash Release and
  full Debug closeout; positive and neutral findings remain separate above.

## Architecture selection gate backlog

The gate remains unstarted. It will consume completed evidence from the
portfolio, compiler/semantic improvements, cross-platform VM matrix and
bounded prototypes. It must record selected and rejected paths, compatibility
effects, remaining performance outliers and the next bounded response.
