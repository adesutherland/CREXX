# cREXX performance roadmap

Last updated: 2026-07-16

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
| NR-01 | Reproducible benchmark portfolio and runner | in progress | Approved workload coverage, serial raw-sample retention and the separate lifecycle lane are implemented; remaining exit work is machine-capturable full provenance and NR-11 publication policy. |
| NR-02 | Portfolio equivalence and optimizer-resistance ledger | complete | All approved steady-state/runtime cells and the lifecycle lane have correctness, equivalence, generated-form and retained-pilot evidence or an explicit `not comparable` disposition. |
| NR-03 | Automated performance evidence bundle | complete | One Level B command retains exact-hash Release timing, schema-3 profile/allocation evidence, RXSEQ N=2/3/4, ranked reports and paired deltas; the verified proof bundle is `evidence/2026-07-15-nr-03-automated-proof/`. |
| NR-04 | Opcode effects inventory | complete | All 641 opcode slots have ordered machine-readable effects; 539 source opcodes are classified or explicitly conservative and RXAS consumes the fail-closed API without new transforms. |
| NR-04A | Runtime type/dispatch architecture and metadata scan evidence | complete | Complete re-linkable RXBIN 007 images, process-local graph views/bindings, site caches, and transparent section compression pass. Retained interface is 37,458 B and RexxCPS 273,858 B; hot graph access remains approximately 1 ns with no focused end-to-end regression. Cross-image factory providers are aggregated correctly, RXAS output re-links byte-for-byte, both VMs pass, and final Debug CTest is 1,846/1,846. Evidence: `evidence/2026-07-16-nr-04a-rxbin-007-compression/`. |
| NR-05 | Call-path census | complete | Schema-4 dynamic census plus exact 22-image dashboard retained at `evidence/2026-07-16-nr-05-call-census/`; focused fixtures cover native/dynamic/signal cold paths absent from the bounded portfolio. |
| NR-06 | Compiler call-window placement fast path | queued | Preserve the contiguous-window ABI and signal-unwind contract; schedule moves once and measure. |
| NR-07 | Direct compare-to-branch lowering | queued | Typed single-use Boolean results only; retain RXAS fallback and source/debug semantics. |
| NR-08 | Definite initialization and reference-lifetime facts | queued | Fail closed; prove dead `NULL`/`ENDLIFE` and copy removals with escape/reference tests. |
| NR-09 | RXSEQ candidate ledger | queued | Rank by workload, static site and module; record semantic status and measured disposition. |
| NR-10 | Cross-runtime forensic baselines | queued | Run the formal CREXX/ooRexx/NetRexx portfolio matrix incrementally as each NR-02 workload clears equivalence; include Regina for RexxCPS only and retain selected Java/C controls. |
| NR-11 | Performance governance and scorecard | queued | Agree Tier A/B, geometric-mean and outlier policies, regression budget and publication format. |

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
- Remaining exit work: make full build/machine provenance machine-capturable
  and define the publication scorecard, common subset and regression policy
  under NR-11. The approved coverage categories and separate lifecycle lane
  are now implemented.

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
| NR-12 | Extend read-only by-value and return copy coalescing | queued | No weakening of by-value isolation or reference semantics; see IDEA-CALL-01. |
| NR-13 | Redundant numeric-context setup elimination | queued | Effective procedure context must be identical. |
| NR-14 | Static/frozen `PARSE` lowering fast path | queued | General supported templates only; retain `parseExec` fallback. |
| NR-15 | General stem default/reset fast path | queued | Preserve generation/default/drop/tail semantics. |
| NR-16 | TRACE-off and same-ADDRESS-environment fast paths | queued | Preserve hooks, signals, mode changes and host callbacks. |
| NR-17 | Link-time direct provider/call resolution | queued | Preserve late-load and plugin fallback. |
| NR-18 | Safe RXAS rule harvest | queued | Require opcode effects/liveness proof and generated rule tests. |
| NR-19 | Optional C LTO/PGO/code-layout experiment | queued | Optional build feature; adopt only with repeatable supported-platform evidence. |
| NR-20 | Value/frame allocation counters and targeted pooling | queued | Gather counters first; preserve ownership and sanitizer gates. |

## P1 architecture-neutral prototypes

These are time-boxed evidence activities, not production commitments.

| ID | Prototype | Status | Adoption question |
| --- | --- | --- | --- |
| NR-21 | Mapped-call descriptor | queued | Does it materially beat compiler placement while preserving aliases and cold unwind? |
| NR-22 | Compact runtime execution stream | queued | Does it improve total portfolio time on at least two architectures with canonical RXBIN unchanged? |
| NR-23 | Runtime quickening/superinstructions | queued | Can dequickening, signals, TRACE/debug, late load and both VM modes remain correct? |
| NR-24 | Profile-selected fusion | queued | Do RXSEQ-selected fusions repeat across the portfolio without code-size explosion? |
| NR-25 | Value hot/cold split or pool allocator | queued | Do cache/allocation counters justify the ownership and complexity cost? |

## Idea ledger

Ideas remain here until assessed, even when rejected. An idea can inform more
than one `NR-*` activity and does not alter their priority by itself.

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

### IDEA-CALL-01 — Read-only by-value formal invariant and call-flag simplification

- Status: captured; assessment not started
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

## Architecture selection gate backlog

The gate remains unstarted. It will consume completed evidence from the
portfolio, compiler/semantic improvements, cross-platform VM matrix and
bounded prototypes. It must record selected and rejected paths, compatibility
effects, remaining performance outliers and the next bounded response.
