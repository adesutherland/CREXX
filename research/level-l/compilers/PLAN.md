# Level C delivery through Level L

**Active scope: Level C only. Architecture and delivery sequence approved by
Adrian, 19 October 2026; implementation started.** The existing native B compiler remains
the build seed. New B/G features, B self-hosting and combined replacement are
outside this delivery; retained consumers remain regression obligations.
INTERPRET is the final implementation stage. Its architecture is reviewed now,
not deferred until a design incompatible with it has already been built.

Source review began on synchronized `develop`
`016e6a65466c65a5c722b96452d95025acb463b2`. During review, local and remote
advanced together to `e5c066b87945dc697df6c771fc5a56a20962c772`, a sanitizer
scheduling/documentation change. Its diff leaves reviewed compiler/runtime
implementation unchanged; preserve the new structured-flow serial scheduling.
All 112 pre-existing active files remained byte-identical. Neither revision
identity is a fresh product, hosted or release qualification. The reorganisation and
subsequent preparation repairs are published. CI monitoring remains paused.

## Vision and intended outcome

Deliver a complete, explicitly selected Level C compiler and reusable compile
service implemented in B using Level L. Complete the agreed Classic instruction,
BIF, Unicode, host, diagnostics and lifecycle contracts, ending with INTERPRET.
Alongside it, deliver independently proven generic Level L services, shared B
Classic libraries, and justified RXAS/VM support for economical C execution.
Keep these four owners separate and document their interfaces and failure paths.
Native C is an inspected comparison control; its bugs, overhead and unfinished
contracts cannot define acceptance of the new implementation.

The [active C-only milestone](#c-first-delivery-milestone) below owns sequence
and acceptance. Read it with the [approved architecture](doc/architecture.md),
[requirements crosswalk](doc/levelc-requirements.md) and
[subagent work packages](doc/agent-work-packages.md). Those documents detail one
plan; they do not introduce competing acceptance authorities. Product routing
and retirement of native C require a separate explicit decision after delivery.

## Existing foundation and evidence boundary

STEP01–06 and STEP07A provide the published C/G PoC, generic IR, owned services,
provenance/optimisation foundations, callable metadata and delivery fixtures.
The bounded SAY radix-literal repair is published; complete SAY remains open.
Retained implementation records below describe their exact local scope. The
historical `06a548321662e6dc4e91899363eb12831b4d952c` checkpoint included the
99-command STEP07A and 40-command SAY checks; later layout/preparation proofs
belong to the parent plan. None establishes complete new C or current hosted
success. Reuse unchanged qualified inputs; do not retain raw run output.

### BIF implementation reuse

The existing Classic libraries already provide the BIF bodies. BF01–09 are
new-compiler binding and verification packages, not a library reimplementation
programme. Preserve unchanged bodies and valid direct-library qualification;
reuse existing independent fixtures through the new compiler. Its current
LENGTH/ARG/DIGITS subset must grow to the existing catalogue, with correct
presence, context, ABI, errors and source behavior. Change a library only for
a reproduced defect, a named unfinished contract or approved Unicode ownership
extraction. The design review found no new BIF-body defect.

## C-first delivery milestone

**C-only architecture and implementation approved, 19 October 2026.** This
supersedes the 16 October ordering only where explicit: B/G expansion is out
of this delivery, INTERPRET architecture is designed now, and its implementation
is last. Existing language exceptions remain binding. The
[Classic worklist](../../../docs/planning/release-1/levelc-compatibility-worklist.md)
continues to own language decisions; a native closure never closes new C.

### Checkable acceptance criteria

All CFC criteria remain **open** for the complete new compiler. Existing
foundation evidence may close individual sub-obligations only.

| ID | Observable pass condition and proof |
|---|---|
| **CFC-AC01** | Complete crosswalk of LC-I-01–25, all 95 named BIF entries, LC-REF-001–075, extensions and approved exceptions. Each row links implementation, independent positive/error expectations and qualification; no omitted or inherited native passes. |
| **CFC-AC02** | Expressions, numeric/string/Unicode values, symbols/stems/pools/EXPOSE, PARSE/ARG, control and calls implement the agreed contract. Test state, order, errors and limits, including defects exposed by native comparisons; each instruction has its own disposition. |
| **CFC-AC03** | Activation/labels, SIGNAL/CALL conditions, RETURN/EXIT, clause clocks, source/TRACE and diagnostic origins survive normal/abrupt/nested paths, Unicode/CRLF/NUL and optimisation. Prior output survives failed compilation. Source identities are logical and stable. |
| **CFC-AC04** | Host invocation/completion, callback-window pool access, ADDRESS/commands, streams/queues, clocks/HALT and signed external calls have positive/error/ownership proof on their declared platforms. Configuration and missing host facilities stay open unless explicitly excluded. |
| **CFC-AC05** | INTERPRET satisfies LC-87-01–05 using the same B/L compiler, live caller state, real resumption/transfers, nested groups and bounded resident-code reclamation. Tests include trapped failures, all-pinned exhaustion, stale handles and teardown; no native/RexxScript fallback. |
| **CFC-AC06** | Representative full RXAS and helper paths have checked structural expectations for simple expressions, loops, calls, errors and dynamic execution. Compiler/RXAS modes are distinct. Direct defects are fixed; necessary cost and later opportunities have owners. Preserve ABI, source, TRACE and numeric observations; static counts are not speed claims. |
| **CFC-AC07** | Generic L services have independent non-C/G consumers, checked structure/effects/facts, bounded failure and documented algorithms. C/BIF/ISA policy stays in owning extensions; source/import/install audits prove no native or typed frontend in the C host closure. |
| **CFC-AC08** | Every required C row closes on qualified inputs, with CLI/service/installed/platform proof, documentation and runnable walkthroughs. Distinguish local, hosted, guest/SDK, sanitizer and release results. Complete C is separate from default replacement and B/G delivery. |
| **CFC-AC09** | B Classic runtime has one implementation of values, pool aliases, activation/context, BIFs and host adapters. C target closure uses the shared B Unicode foundation independently of G aggregation; native C, typed Classic mode and RexxScript retain their contracts. Public metadata/doc tags and consumer regressions prove compatibility. |
| **CFC-AC10** | Any RXAS/VM change has an approved operation/storage contract, exact error/alias/lifetime/source semantics, encoding/disassembly/loader tests and both-engine/platform proof. Alternatives are compared; production performance edits pass the first Release verdict gate. No native source-language implementation is introduced. |
| **CFC-AC11** | Compile/request/result and runtime activation/code lifetimes are isolated, reusable and bounded. Verify exact lengths, atomic failure, stale/foreign references, interruption, repeated/concurrent contexts, declared budgets and recovery without partially published output. |
| **CFC-AC12** | Delivery/build manifests separate native seed, L tools/SDK, C host, C runtime and optional dynamic-compile support. Cold/incremental/installed/relocated builds have one producer and correct freshness; static applications have no compiler dependency. Platform capability rows and source/user/API documentation match actual products. |

AC01–08 preserve their earlier meanings; AC09–12 make runtime/ISA/resource and
product boundaries separately checkable. Explicitly approved compatibility
exceptions are positive documented contracts, not permission to omit unfinished
host/source/reference obligations. Formal performance tests are not a research
publication gate; production primitive edits and later replacement budgets
retain repository performance governance.

### Numbered delivery sequence

The CLC sequence is **approved for implementation**. STEP01 and bounded
foundations of STEP02/03 are active; later steps remain open. Each step updates architecture, schemas/algorithms and user guidance
with its implementation. Generic L changes include independent tests; shared
changes include existing G/native-C/RexxScript tests where affected.

| Step | Deliverable and decisive checks | Criteria / dependencies |
|---|---|---|
| **CLC-STEP01 — contracts and executable baseline** | Freeze the requirements/exception ledger, source/ABI/platform manifests and independent expectations. Audit current tests against code; establish C-only dependency controls and representative RXAS shapes. No assumption that native is correct. | AC01/06/07/08/12; design approval. |
| **CLC-STEP02 — generic L foundations** | Reuse existing llflow/llir services; extend registered effect/control contracts for exceptional and resumable edges, clause observations, revision-bound analyses, generic liveness and checked rewrites. Retain block-local values/slots; no gratuitous SSA conversion. Positive/adversarial independent L fixtures and G regressions. | AC07/10/11; STEP01; precise public API/schema approval. Deliver foundations incrementally against real consumers. |
| **CLC-STEP03 — Classic runtime and ABI** | Reconcile shared B activation/context/pool services, direct operation entries, BIF presence/error ABI and one-body label-entry model. Extract the existing B Unicode algorithms into a separately selectable B foundation for C/G, retaining G facade/API and independent L bootstrap; no algorithm/table duplication. Prove recursion, restoration, exposure, exceptions and build closures. | AC02/03/09/11/12; STEP01 and required STEP02 contracts; exact ABI gate. |
| **CLC-STEP04 — source and expression completion** | Complete contextual lexing, Unicode symbols, literals/continuation/comments, AST/source maps, all operators and numeric/context/error rules; bind existing value/Unicode BIF entries in BF01–04 and reuse their expected-result fixtures. Validate unreachable syntax before optimisation. Test independent expectations, both modes and RXAS cost. | AC01–03/06/07/09/11; STEP02/03. |
| **CLC-STEP05 — instruction reviews 01–10** | SAY, DROP, assignment, NOP, OPTIONS, IF, SELECT, DO, LEAVE, ITERATE, individually. Support all source forms/errors, pool/loop effects and clause anchors; normal/strict timing distinctions remain explicit. | AC01–03/06/07; STEP04. Each row gets its own implementation/review packet; dependent host/TRACE proof stays open. |
| **CLC-STEP06 — instruction reviews 11–19** | ARG, PROCEDURE, CALL, RETURN, EXIT, PULL, PUSH, QUEUE, PARSE individually. Integrate one PARSE executor, shared label/fall-through activation, exact argument presence, static signed providers and BF05/06/08. | AC01–04/06/09/11; STEP03–05. ARG may introduce PARSE infrastructure before PARSE's full instruction review. |
| **CLC-STEP07 — instruction reviews 20–24** | ADDRESS, implicit commands, NUMERIC, SIGNAL, TRACE individually. Finish BF07/09 compiler integration, command/callback state, delayed/immediate traps and existing clock/source/error context integration. Inspect authored locations and executable overhead. | AC01–04/06/09; STEP06. NUMERIC/conditions needed earlier use STEP03/04 contracts; full instruction closure occurs here. |
| **CLC-STEP08 — host and remaining contract completeness** | Close named remaining reference/host configuration and BIF integration rows, preserving completed library work; length-aware invocation/completion and pool callback-window APIs, real HALT/clock/error producers and named stream/queue/platform adapters. No silent desktop-only completion. | AC01–04/09/11/12; STEP04–07; new public host APIs require exact contract approval. |
| **CLC-STEP09 — target quality and justified infrastructure** | Finish C-OPT-01–06 and allocation/call-window proofs; compare existing instructions, library/selector fixes and any native primitive. Measure concrete production candidates before selection. Add only approved primitives and qualify both engines/consumers. | AC06/07/10/11; applies incrementally after STEP03, full closure after STEP08. Performance verdict gate for production edits. No INTERPRET mechanism implementation here. |
| **CLC-STEP10 — static C acceptance** | Close LC-I-01–24, all BIFs and static reference rows; installed/cold/freshness manifests, platform applicability, source/IR/RXAS walkthrough and independent coordinator review. Run required grouped checks once on final inputs. | AC01–04/06–12 static portions; STEP01–09. This is a bounded static-C milestone, not complete C. |
| **CLC-STEP11 — INTERPRET and final C acceptance** | Implement and qualify the dynamic group mechanism last, then close all complete-C criteria, docs and installed/platform proof. Substeps below include the remaining code-owner/continuation infrastructure. | All AC01–12; static milestone and exact mechanism approval. No B/G expansion or default switch follows automatically. |

STEP02/03 may be developed in parallel on disjoint ownership after interfaces
are frozen. Instruction compiler edits are serial in LC-I order; workers can
prepare independent expected tests, runtime services and independent reviews in
parallel. Do not declare a whole instruction closed while its listed host,
TRACE, condition or shared-service proofs remain open.

#### Final stage substeps

- **CLC-STEP11a:** prove normal/nested/SYNTAX/CALL-handler-resume/SIGNAL cases
  with live activation and owned continuations; compare the proposed image-group
  approach with existing loader limits. Review exact ABI, cleanup and quotas.
- **CLC-STEP11b:** implement approved generic ephemeral image ownership and safe
  entry/reclamation in VM/linker/assembler interfaces as needed. Prove pins,
  stale handles, all-pinned exhaustion, failure rollback and both engines.
- **CLC-STEP11c:** connect interpreted-group mode of the same C service, compile/
  assemble/load, live pool/arguments/configuration, caller routine dispatch and
  continuation spills. No second parser/evaluator or native compiler fallback.
- **CLC-STEP11d:** close all LC-87 semantics including PROCEDURE eligibility,
  outer loop restrictions, RETURN/EXIT/SIGNAL, delayed handlers, nested groups,
  source/TRACE, caches/quotas, host/platform and installed use. Complete the final
  independent C review and required qualification on exact inputs.

These substeps are one final implementation stage. Host/platform limitations
that prevent required dynamic execution remain explicit blockers for decision;
a file-stage compiler alone does not close INTERPRET.

### Existing step-ID reconciliation

| Retained ID | Current disposition |
|---|---|
| CFC-STEP01 | Instruction review and crosswalk continue through CLC-STEP01/04–08; bounded SAY evidence retained below. |
| CFC-STEP02 | Mechanism architecture proposed here; executable proof moves to final CLC-STEP11a. |
| CFC-STEP03 | Semantic work is CLC-STEP03–06. |
| CFC-STEP04 | Conditions/source/host work is CLC-STEP07/08. |
| CFC-STEP05 | INTERPRET integration is CLC-STEP11. |
| CFC-STEP06 | Static checkpoint CLC-STEP10; complete C checkpoint CLC-STEP11d. |
| CFC-STEP07 | B/G expansion is outside this delivery; no automatic resumption. |
| LLCOMP-STEP07B/09/10 | Wider B/G/self-hosting/replacement criteria remain unverified, outside this C-only completion claim. |

### Approval and decision gates

Adrian approved the four-owner architecture, C-only sequence and GPT-6.1 Sol
Extra High worker model on 19 October 2026. Review exact changed public L/Classic APIs,
serialized schemas, host services and VM/ISA contracts in their owning step.
Reusing existing approved semantics needs no repeated approval. Any proposed
language deviation must identify its user-visible effect and receive a separate
decision. Default-routing changes, native-C retirement and release qualification
are not authorised by approving research implementation.

### Design-review validation

Inspected compiler/runtime sources at `016e6a65466c65a5c722b96452d95025acb463b2`
are unchanged by the reconciled `e5c066b87945dc697df6c771fc5a56a20962c772` head.
The completed design review changed documentation only. In-memory Python checks confirm 25
instruction rows, all 95 native catalogue names exactly once in BIF work groups,
all 75 reference IDs and retention of prior acceptance/step IDs. Local document
links/anchors and `git diff --check` pass; all 112 pre-existing active files are
byte-identical. No product/test qualification was rerun for this proposal.
The active implementation below establishes executable proof incrementally.

## Active first implementation increment

**First bounded foundation increment implemented and locally qualified.
Complete C remains open.** Base:
`e5c066b87945dc697df6c771fc5a56a20962c772`. Adrian approved implementation
with GPT-6.1 Sol Extra High subagents, existing BIF/test reuse, clean ownership
and INTERPRET last. The coordinator owns integration and independent checks.
Adrian authorised committing this baseline locally on 19 October; publication
and resumed CI monitoring are outside that instruction. The existing qualified
source/test/build inputs are unchanged, so their results below are reused.

| Increment | Acceptance and work | Current status |
|---|---|---|
| **CLC-STEP01a** | Reuse current SAY fixtures/expected bytes in a C-only staged host with native/typed compilers absent; execute generated static images in a separate compiler-free runtime. Inspect existing RXAS structural checks. CFC-AC06/07/12 slices. | Implemented and locally qualified: 32 existing C checks, both modes and VM engines; C-only registration is outside the mixed C/G gate. Complete C dependency/platform acceptance remains open. |
| **CLC-STEP02a** | Replace repeated whole-graph predecessor scans in existing IR definite-assignment analysis with private bounded predecessor/worklist infrastructure. Preserve API/schema, unsupported controls and atomic revision-bound facts. Independent irreducible/multiple-root/limit fixture plus existing IR/service regressions. CFC-AC07/11 slices. | Implemented and locally qualified: indexed predecessors, bounded work queue, checked lattice storage/work, atomic facts, independent runtime and RXAS shape checks. Exceptional/resumable effects and liveness allocation remain open. |
| **CLC-STEP03a** | Extract unchanged B Unicode executors and pinned data from G into optional `rxunicode-core.rxbin`; generators in `tools/unicode`. Retain private ABI, G facade and existing conformance/shape fixtures. Share canonical property data with L without runtime bootstrap dependency. CFC-AC09/12 slices. | Implemented and locally qualified: single B producer, unchanged algorithms/G facade, independent B runtime, canonical data, recovery/no-op and install checks. Exact public B facade and C adapter migration remain open. |

### Local qualification and findings

Inputs are the base above plus this increment's source, test and build changes.
Three GPT-6.1 Sol Extra High workers supplied disjoint implementations; the
coordinator reviewed and integrated their code, inspected generated RXAS and
ran the following focused checks. Existing BIF bodies and expected results were
reused. No native compiler/runtime instruction or public API was changed.

- `cmake --build cmake-build-debug --parallel 10 --target stage-product`
  passes without compiler warnings. This includes the integrated B Unicode
  producer and its G consumer.
- `llcompiler_c_only_measure` passes the existing 32 C SAY commands in an
  isolated compiler host and a separate runtime, with empty `PATH` and VM
  autoload disabled. Exact imports include the existing `classlib`; the old
  test's ambient autoload assumption is removed. Debug took about 19 seconds;
  maintained Apple ASan/Make took about 55 seconds. These scheduling observations
  support serial CTest registration with a 900-second hang limit, not a speed
  claim. `ctest --test-dir cmake-build-debug -R '^llcompiler_c_only_'`
  passes all four mode/VM variants (about 17 seconds each).
- Focused Debug CTests pass for independent L IR/services/flow/optimisation,
  the retained mixed C/G SAY cases, direct B Unicode runtime, all five existing
  Unicode RXAS checks, existing G Unicode conformance/contract and native C
  Unicode/label consumers. After the final flow copy correction, all 23 selected
  L, mixed SAY, RXAS and repository hygiene/cleanup checks pass; unchanged
  Unicode semantic inputs reuse their preceding passes.
- `tools/asan-run.sh --phase build` with targets `llcompiler_c_only_measure`,
  `ts_llirflow_opt`, `ts_llirflow_noopt`, `ts_llirservices_opt`,
  `ts_llirservices_noopt` and `ts_unicode_core`, eight build jobs and
  `--build-leaks off`, passes. The cold C-only Make path exposed a missing
  cross-directory `llbinmeta` producer prerequisite. Research `ll_compile`
  now maps exact shared image paths to their producers while retaining file
  freshness dependencies. The same preparation then passes.
- `tools/asan-run.sh --phase ctest --regex
  '^(ts_llir(flow|services)_|ts_unicode_core_|levell_ir_flow_rxas$)'
  --test-jobs 8 --leaks off` passes all 13 tests on the maintained Apple ASan
  build. No sanitizer finding was observed. Apple LSan is unavailable; this is
  focused ASan evidence, not a full platform sanitizer closure. Refreshing native
  prerequisites emitted existing SDK `sprintf` deprecation warnings; the ASan
  refresh is not claimed warning-free.
- Existing `rxlang_bootstrap_build_order` passes its real-declaration Make
  clean/missing-L2/changed-input/no-op fixture. Actual Debug/Ninja Unicode
  builds restore a removed member RXAS, generated source and final core image
  byte-identically; the following builds preserve timestamps. Actual maintained
  ASan/Make restores the member and then remains a no-op. Recovering the core
  exposed a repeated G builder invocation when its link retained identical
  bytes; the G CMake rule now records successful output completion. Two
  following Make requests run no G controller and preserve bytes/timestamps.
- `python3 -B scripts/tests/check_install_inventory.py --build
  cmake-build-debug --expect-bytecode rxunicode-core` passes fresh/repeated
  installation into a prefix containing spaces, checks 391 files and retained
  exclusions, and verifies exact core bytes. A separate core component install
  also preserves those bytes. This does not establish installed new-C closure.

RXAS review found that two prior `bincopy` calls cloned the full B×S lattice
through a by-value argument before copying one S-byte row. Both callers now
size the scratch row to S and use the existing target-sized `bcopy` operation;
no BIF body or VM edit was needed. `levell_ir_flow_rxas` checks this shape from
metadata-resolved registers and rejects whole-lattice copies. Remaining seed
compiler object/trace/service overhead is described in the algorithm guide;
correctness and structural checks do not establish a formal performance result.

This increment leaves full CFC-AC01–12 open. Next work is the remaining STEP01
contract/dependency ledger and precise STEP02/03 public effect/runtime/Unicode
interfaces, followed by expression and existing-BIF integration in STEP04.
Private Unicode packaging does not yet remove C's existing G facade dependency.
INTERPRET implementation remains STEP11, last. No full hosted, cross-platform,
release or replacement readiness is claimed; CI monitoring remains paused.
Final `git diff --check` and reviewed document links pass. All 112 unrelated
active files remain byte-identical and uncommitted. Task-created diagnostic
logs and temporary fixtures were deleted; maintained regression inputs and
build products remain in their owning locations.

## Authority and scope

This is the authoritative implementation plan for the new compiler subtree.
The [Level L programme plan](../PLAN.md) delegates this
work here and retains the wider language-tooling and AST acceptance criteria.
The [compiler separation proposal](../../../compiler/docs/compiler_frontend_split_proposal.md)
supplies useful semantic and ABI requirements. Its native-library decomposition
and proposed C-backend adapter are not the implementation route selected here.

The [Classic compatibility worklist](../../../docs/planning/release-1/levelc-compatibility-worklist.md)
continues to own Classic conformance, including `LC-I-25` / `LC-87-01–05`
INTERPRET. This plan implements and tests relevant requirements without closing
those entries by declaration. Product priorities and release qualification stay
with the repository roadmap, release plan and AGENTS.md.

### Required implementation boundary: cREXX, no C compiler implementation

- All new compiler logic is cREXX: scanning/parsing, language checks, imports,
  IR construction/verification, analysis, optimisation, register assignment,
  source maps and RXAS emission. Write implementation modules in Level B
  `.crexx`; write grammars as `.rxl` or `.crexx` with `options levell`.
- Use the existing Level L generators and their cREXX/RXAS output. Do not add C
  parser actions, native compiler plugins, or another-language implementation
  hidden behind a cREXX facade. No new compiler dependency on re2c or Lemon.
- The new compiler's source-to-RXAS execution path must not call, link, load or
  delegate semantic work to `rxc`, `rxc-c`, `rxclib`, `rxcc_lib`, the C lowerer,
  optimiser, register allocator, emitter or native compiler metadata reader.
  Port needed metadata-reading logic into cREXX against the published contract.
- The existing C-language Level B compiler may compile the compiler and its
  libraries during bootstrap. Old compilers may run independently as comparison
  controls in tests. Neither role is a dependency when the new compiler compiles
  an input programme after bootstrap.
- Existing VM/runtime primitives and libraries remain the execution platform;
  existing RXAS/RXLINK tools may process the emitted RXAS after compilation.
  Their implementation language is outside this compiler rewrite. These roles
  must be visible in the dependency audit, rather than described as a C-free
  operating environment.
- Build configuration and test orchestration may use existing repository tools;
  compiler algorithms and new performance tooling stay in cREXX. The proposed
  C-only architecture includes separately owned native RXAS/VM enhancements.
  It does not authorise their implementation before design approval; exact
  public API/ISA, lifetime and performance gates remain binding. Native changes
  must implement execution/storage primitives, never hidden source compilation.


## Earlier programme scope and retained IDs

The earlier Phase A delivered a C/G PoC; Phase B sought full B/C/G coverage,
Phase C reusable compilation/INTERPRET and Phase D combined replacement.
Adrian narrowed active delivery to C on 19 October. The LLCOMP criteria and
steps below preserve earlier contracts and stable references; their B/G-only
portions are **outside the current delivery**, not completed or automatically
scheduled next. C portions are implemented through CFC/CLC below. No new B
compiler, full G migration or combined default switch gates C completion.

## Architecture documentation alongside implementation

Adrian explicitly requires purposeful tree/structure design and parallel
documentation of architecture and processing algorithms. Each implementation
increment includes the corresponding design, reference and worked explanation;
documentation is part of its acceptance, not a later cleanup phase.

Before implementing a representation, explain the question it answers and the
information it retains. Review its shape with a small real programme. Avoid
carrying a source-parser node into later stages merely because it is already
available, and avoid adding a new representation without a distinct purpose.

| Representation | Design to document and verify |
|---|---|
| Source, tokens and parse results | Exact source identity/encoding, token ranges, trivia, selected derivation, ambiguity/recovery status and ownership. |
| C and G language ASTs | Nodes that express the respective language constructs; explicit fields, optional/list counts, source spelling and semantic attachments. |
| Common IR | Operation/result types, functions, blocks, control edges, value uses, state/effect dependencies and the stage at which language-specific operations are lowered. |
| Symbols, types and analysis tables | Binding identity, scope, aliases, sparse defaults, ownership, fact validity and invalidation after transformation. |
| RXAS preparation and output | Instruction/operand representation, virtual and assigned registers, call windows, cleanup, labels, metadata and source-map chunks. |

For every node/record family, publish a schema table: purpose, fields and types,
required/optional/list cardinality, explicit/default values, owned children versus
borrowed references, identity and lifetime, allowed cycles/sharing, invariants,
and the validator that checks them. Explain which information a stage adds,
preserves or deliberately removes. Version cross-stage contracts deliberately;
do not overload one field with different undocumented meanings in later passes.

Maintain these documents under `compilers/doc/` as their corresponding features
are implemented:

- `architecture.md`: component responsibilities, dependencies, bootstrap,
  service boundaries and full source-to-execution processing order.
- `trees-and-ir.md`: readable schemas and diagrams for each tree, graph and
  supporting structure, including lifetimes and verified invariants.
- `algorithms.md`: the chosen algorithms and why they fit, their inputs/outputs,
  pre/postconditions, worklist or traversal order, stopping rules, time/storage
  bounds, failure modes and evidence for stated correctness properties.
- `pipeline-walkthrough.md`: one paired C/G example followed through tokens,
  language AST, common IR, analysis facts, before/after optimisations, register
  assignment and actual RXAS. Link the runnable fixtures and retain intentional
  semantic differences. Include an invalid-input example with source diagnosis.
- `design-decisions.md`: concise reasons and tradeoffs behind selected
  representations and algorithms, with links to any unresolved review gates.

The algorithm account covers lexical/parser and composite choices, binding and
type checking, graph construction, effect analysis, fixed-point scheduling,
rewriting, instruction selection, register assignment and output construction
as those stages arrive. Distinguish a proved bound, an assumed semantic premise
and a measured observation. Explain terms before using them; a diagram or node
dump supplements the explanation rather than replacing it.

As-built reference pages describe implemented services. The architecture page
now explicitly describes the proposed target, with a separately labelled source
baseline. Proposed records are not public APIs until their contract gate passes.
Use tested fixture dumps/excerpts where practical so examples cannot silently
drift from code. Keep chronology and qualification receipts out of the enduring
use/reference explanation. No empty documentation skeleton counts as evidence.

## Bootstrap and dependency direction

Active order: retained native B seed and native tools → B runtime and Level L
SDK/tools → new C compiler → C applications. C source requests use no native
source compiler/backend. Keep compiler-host, target-runtime and optional runtime-
compilation manifests separate. C-only host products must not link/generate the
G frontend. The current rxfnsc package's G Unicode dependency needs the reviewed
package split in CLC-STEP03; no duplicate Unicode implementation is proposed.

Existing L0/L1/L2 bootstrap remains required. Earlier B1/B2/B3 self-hosting and
full G closure remain LLCOMP-AC02/STEP07B outside this delivery, without any
claim of convergence. Cold and installed C checks use explicit import roots and
native source-compiler services unavailable after bootstrap. Separate RXAS and
RXLINK stages remain supported, including constrained-host applicability.

## Numbered acceptance criteria

These are **retained whole-programme criteria**, not the active C-only
completion checklist. Their unverified portions stay open; B/G expansion and
self-hosting portions are out of the current delivery. CFC-AC01–12 below define
this delivery without claiming the wider programme complete.

1. **LLCOMP-AC01 — cREXX-only compiler and separation.** All compiler stages are
   cREXX/Level L under the layout above, with generic services in Level L. Both
   tools compile inputs with the old compilers/backends unavailable after
   bootstrap. Verify source/build/dependency audits and installed execution;
   existing default tools and unrelated consumers retain their behaviour.
2. **LLCOMP-AC02 — Acyclic bootstrap and proper B.** A cold native B seed builds
   L and the proper cREXX B compiler. That compiler rebuilds the B libraries, L
   and itself without native compiler services or G; the next identical stage
   agrees under the documented deterministic comparison. Retain L0/L1/L2 reader
   parity, B2/B3 closure comparisons, opt/no-opt controls, semantic regressions
   and transitive dependency manifests. Both new C/G compiler hosts build from
   qualified B/L with no G prerequisite. Separately inventory target runtime
   packages and every retained native tool. Phase A establishes only the
   seed-built C/G PoC closure; STEP07B closes proper B/self-hosting obligations.
3. **LLCOMP-AC03 — Two real frontends.** C and G each parse and validate the
   agreed Phase A corpus through Level L and the same IR/backend. Unsupported
   forms receive located errors without fallback. Record corpus and feature
   coverage; full C/G coverage remains open until Phase B is complete.
4. **LLCOMP-AC04 — Checked common IR.** Validate types, operands/results,
   function/block membership, control targets, value uses and selected lifetime
   rules. Reject stale/foreign references and invalid graph shapes. Test deep,
   wide and cyclic structures, deterministic dumps, exact budgets and atomic
   failure. Verify SSA-specific rules if that representation is selected.
   Under the proposed numeric design, retain text/binary round trips, checked
   schema decoding and direct handle-based access without text in pass loops.
   Verify namespaced extension admission and a non-C/G operation-family fixture
   without compiler-module imports; test conservative handling of unknown
   semantics and refusal of incomplete control contracts.
5. **LLCOMP-AC05 — Effects and semantic distinctions.** Both language mappings
   preserve their numeric, logical, variable, call, failure and trace contracts.
   Unknown operations block unjustified transformations. Independent positive
   and adversarial tests demonstrate eager/skipped operands, mutable numeric
   context and state invalidation; broader control contracts remain open until
   their operations are admitted.
6. **LLCOMP-AC06 — Shared verified optimisations.** Shared passes make observable
   IR improvements for both frontends before RXAS emission. Retain pass-level
   arguments, operation/work bounds, changed-IR evidence and opt/no-opt semantic
   comparisons. Use assembler no-opt controls to isolate these effects. Failed
   passes publish no partial successful output.
7. **LLCOMP-AC07 — cREXX RXAS backend.** Instruction selection, register
   assignment and source-mapped emission are implemented in cREXX. Emitted RXAS
   assembles, links and runs on both concrete VMs where supported. Verify exact
   constants/quoting, branch targets, call windows, value lifetimes and metadata
   for each admitted profile. Test emitter copying/streaming and failure bounds.
8. **LLCOMP-AC08 — Imports and ABI.** A cREXX contract reader and emitter preserve
   the existing module/type/member/callable/provider/source metadata needed by
   the admitted profile. Exercise separately built libraries, B/G providers,
   linking, signatures and malformed imports. Full metadata conformance is a
   replacement gate; no hidden native compiler reader is permitted.
9. **LLCOMP-AC09 — Diagnostics and source fidelity.** Grammar, source, semantic,
   IR and emission errors name the operation and original location. Preserve
   Unicode, CRLF, exact source lengths, literal spelling and generated origins.
   Recovered input cannot produce a successful executable. Test diagnostic
   anchors before/after optimisation and retain defined TRACE observations.
10. **LLCOMP-AC10 — Full language migration.** Every feature and delivery
    contract of the B/C/G routes being replaced has a current implementation/test
    disposition. Existing supported behaviour is preserved unless Adrian
    approves a specific change. Reconcile the B contract, Classic worklist and G reference;
    a PoC subset does not satisfy this criterion.
11. **LLCOMP-AC11 — Compile service and INTERPRET.** Exact-length cREXX request
    APIs own results and cleanup. Prove reuse/reentrancy requirements and the
    Phase C activation, transfer, resumption and bounded-code-lifetime contract
    against `LC-87-01–05`. Test nested use and every admitted failure path;
    an unresolved platform capability remains an open gate.
12. **LLCOMP-AC12 — Product and platform delivery.** Separate tools build and
    install without collisions; combined/library consumers, wrappers, both VMs
    and optimisation modes work on the supported platform matrix. Include
    constrained/mainframe applicability and separate-stage operation in the
    migration inventory. Preserve current product builds and generic Level L
    consumers. State which platforms are locally versus hosted verified.
13. **LLCOMP-AC13 — Resource and performance evidence.** Retain representative
    compile latency, peak storage/copying, emitted code size and execution
    measurements for C and G, with exact inputs and an existing-compiler
    baseline. Include numeric arena layout, indexes, facts and codec overhead;
    compare construction/traversal/rewrite costs before choosing packing.
    Record semantic/resource bounds separately from timings. Agree
    production replacement budgets from measurements before the final gate;
    no arbitrary hidden limit or unmeasured speed claim establishes success.
14. **LLCOMP-AC14 — Usability and replacement.** The manual explains grammar,
    AST, IR, passes and RXAS through paired examples; APIs and dumps explain
    failed contracts in plain language. All required earlier criteria are
    verified before an explicitly approved default switch. Verify installation,
    bootstrap-only retention, retirement of obsolete routes and rollback.
15. **LLCOMP-AC15 — Purposeful structures and concurrent documentation.** Each
    implemented stage has current architecture, schema and algorithm accounts
    covering the contracts above. Every representation has a stated purpose and
    reviewed example; validators correspond to its documented invariants. The
    paired C/G walkthrough agrees with runnable stage dumps and emitted RXAS.
    Verify documentation/code cross-references and example regeneration in each
    increment. No increment is complete with its design explanation deferred.
16. **LLCOMP-AC16 — Emitted-code quality.** Retain paired C/G examples and
    equivalent existing B/G/C compiler controls, with compiler/assembler modes
    explicit. Verify truthful source ranges, clause/provenance identities and
    stable logical filenames; no accidental absolute host paths or needless
    unchanged source/diagnostic-state emission. Simple examples must not create
    unused default operands or reserve call storage without a target need.
    Account for executable operations, helper work, calls, copies, branches,
    frame/register requirements and numeric/activation setup separately from
    metadata. Preserve required ABI, ownership, errors and TRACE observations.
    Record and close direct defects before quality acceptance; assign proven
    optimisation opportunities to named passes with semantic and structural
    regressions. Existing compiler inefficiency does not excuse new excess.
    AC13 still owns representative timing/storage budgets and replacement
    measurements; static shape counts do not establish a speed claim.

## Numbered implementation steps and gates

| Step | Work and evidence | Criteria | Dependencies / gate |
|---|---|---|---|
| **LLCOMP-STEP01** | Inventory C/G source/ABI/tooling coverage and bootstrap dependencies; pin paired PoC forms and baselines. Present the smallest grammar, IR dump, graph/value model, numeric storage/view proposal, effect contract and emitter API. | AC01–05/08–15 | Plan approval first. Review new public syntax/API and the exact PoC boundary before implementing them. |
| **LLCOMP-STEP02** | Implement reusable checked graph/value/effect and emission foundations in Level B, including the extensible IR, approved storage and view/codec contracts; register C/G operation/type extensions in `common`. Preserve AST consumers. | AC01/02/04/05/07/09/13 | STEP01 surface gate. Focused malformed/lifecycle/budget regressions. |
| **LLCOMP-STEP03** | Build both Level L grammars and semantic mappers for the pinned scalar/control/call profile, with explicit language differences and readable IR. | AC03–05/08/09 | STEP02; neither frontend may delegate to the old compiler. |
| **LLCOMP-STEP04** | Complete unoptimised cREXX RXAS selection, simple register assignment, ABI calls and separate driver/service entries. Execute both languages end to end. Close the direct source-map, construction and emission findings from the emission-quality review. | AC01/03/07–09/12/16 | STEP03; retain old compiler controls as separate test processes only. Functional implementation published; CP essentials approved and completed locally. Wider quality follow-through remains open. |
| **LLCOMP-STEP05** | Add shared temporary propagation, admitted constant evaluation, branch simplification and safe unused-operation elimination. Verify against the independent IR evaluator and emitted execution, including actual code-shape improvements for both languages. | AC04–07/09/13/16 | STEP04; pass contracts reviewed before enabling each transformation. Do not use optimisation to conceal direct construction/emission defects. |
| **LLCOMP-STEP06** | Qualify and document the two-frontend PoC: bootstrap, installed tools, both VMs/modes, failure cases, explicit resource bounds and semantic comparisons. | Phase A portions of AC01–09/12–16 | STEP05. **PoC review gate:** assess usefulness and complexity; approve the next coverage increments. This does not authorise production replacement. |
| **LLCOMP-STEP07** | Complete Classic coverage first through the C-first milestone: instructions/BIFs, pools, calls, runtime/host contracts, diagnostics and tooling. Maintain current G coverage; its remaining classes/references, ownership, extensions and other full-language obligations stay open for later. | AC03–10/12–16 | STEP06/07A foundations; keep the complete coverage matrix current and generic services independent of either language. No new B prerequisite for C delivery. |
| **LLCOMP-STEP07B — outside current C-only delivery** | Implement the proper B frontend/profile over the shared IR/backend; inventory full B coverage and the self-hosting source closure. Build B1 from the native seed, rebuild B/L/B to B2 and B3, and qualify convergence, semantic behaviour and independence from G/native compiler services. | AC01/02/04–10/12–15 | Shared backend and required typed/metadata coverage from STEP04/07; review the exact B coverage and stage manifest before implementation. Required before combined STEP09, not before the C-first milestone. |
| **LLCOMP-STEP08 — CLC-STEP11, final implementation stage** | Prove and implement the shared compile-service/Classic interpreted-group activation and continuation route, with bounded generated-code ownership. | AC05/09/11–13 | The 19 October design request brings architecture review into this proposal; mechanism implementation follows static-C acceptance in CLC-STEP11. **INTERPRET mechanism gate**, existing LC criteria and B/L-only source-compiler constraint remain binding; separately reviewed native execution/storage support is permitted by the proposed design. |
| **LLCOMP-STEP09** | Close full B/C/G replacement coverage, installed/platform qualification, constrained-host/bootstrap requirements and representative performance budgets. | AC01–16 | STEP07/07B/08; reuse unchanged evidence and keep release/sanitizer gates distinct from ordinary develop publication. |
| **LLCOMP-STEP10** | Switch default B/C/G routes, retain the explicit native B seed, and retire superseded production compiler paths/documentation. | AC01/02/10/12/14 | STEP09 plus **Adrian's explicit replacement approval** and a verified rollback procedure. |

**AC15 and AC16 apply throughout the programme.** STEP01 presents the architecture and purposeful
tree/graph designs for review. STEP02 documents storage, ownership, schemas and
verifiers; STEP03 documents each frontend and language mapping; STEP04 documents
emission, registers and ABI; STEP05 documents analyses and transformation
arguments. STEP06 verifies the complete walkthrough. STEP07–10 update these
accounts in the same increment as every new capability or replacement change.

Adrian approved the [current plan](PLAN.md) and authorised STEP02
on 15 October 2026: Level B-only IR foundations and semantic extension API,
regressions, architecture documentation and user guide. STEP03–06 retain their
subsequent implementation and named pass/PoC gates.
STEP07–10 preserve the full destination and later decisions; bounded approval
does not remove those outcomes.


1. **IR2-AC01:** Versioned family/type/operation registration, frozen schemas,
   explicit unknown effects, and checked service dispatch. Duplicate, foreign,
   incomplete and invalid registrations have stable diagnostics.
2. **IR2-AC02:** Numeric graph storage and named construction handles support
   functions, blocks, typed slots, arguments, constants, extension operations,
   branches and returns. Reject invalid ownership, stale handles, types,
   use order, signatures and terminators; demonstrate loops and joins.
3. **IR2-AC03:** Read-only structured views, deterministic named dumps and
   lossless text/binary loading preserve exact constants and source origins.
   Corrupt, truncated, incompatible and over-budget input publishes no module.
4. **IR2-AC04:** Semantic verifier/evaluator/lowering contracts have checked
   success, keep, invalid and limit outcomes. An independent non-C/G consumer
   and initial compiler family registrations exercise those contracts. No
   callback is obtained from an input file; unknown effects remain conservative.
5. **IR2-AC05:** Sparse revision-bound facts and checked replacement foundations
   preserve the source module on failure. A generic analysis/consumer exercises
   the graph without depending on C/G semantics.
6. **IR2-AC06:** A bounded chunk emitter supports text, lines, indentation and
   sections with explicit failure; callers can emit without quadratic whole-
   output concatenation. Target instruction selection remains STEP04.
7. **IR2-AC07:** Compare fixed-width record and column storage using a Level B
   harness; retain costs and bounded measurements. Compile/link/run focused
   regressions in opt/no-opt and both VMs, check the B-only dependency closure,
   product integration and relevant existing AST regressions. Update architecture
   and the enduring user guide with runnable examples and exact API contracts.

Implementation sequence:

| Step | Work | Acceptance |
|---|---|---|
| IR2-STEP01 | Implement schema and semantic service types; numerical storage and checked builders. | IR2-AC01/02/04 |
| IR2-STEP02 | Implement verifier, views, codecs, facts and atomic replacement/service coordination. | IR2-AC02–05 |
| IR2-STEP03 | Add emitter, independent example and initial compiler semantic registrations. | IR2-AC04–06 |
| IR2-STEP04 | Regressions, storage comparison, B-only build/consumer qualification, documentation. | IR2-AC01–07 |

Current state: **IR2-STEP01–04 complete; IR2-AC01–07 verified locally.**

1. **F3-AC01 — Two complete admitted grammars.** Generated Level L DFA/LR(1)
   frontends recognise the POC-01–10 source corpus, including Classic contextual
   names and whitespace concatenation, nearest-IF ELSE binding, routine forms,
   loop transfers and calls. Preserve exact origins, spelling and ordered AST
   children. Evidence: positive/negative grammar fixtures and schema validation.
2. **F3-AC02 — Explicit language mappings.** G binding/types/defaults and
   short-circuit evaluation, and C pools/unset names, eager operands, argument
   presence/RESULT and NUMERIC context produce distinct checked operations.
   Extend compiler-owned families for the full admitted profile, including SAY,
   with documented types/effects/failures. Evidence: tree/IR checks for the six
   controls plus focused semantic edge cases; no guessed context or G-shaped
   Classic AST. Runtime execution equivalence remains a STEP04–06 gate.
3. **F3-AC03 — Checked calls and imported contracts.** Resolve admitted local
   routines, recursion and library/BIF calls from their actual contracts, with
   the required metadata reader implemented in Level B. Wrong/missing imports,
   signatures and argument forms produce located diagnostics. No native compiler
   reader or frontend is a source-compilation dependency. Evidence: dependency
   audit and valid/malformed contract fixtures; emitted ABI execution is STEP04.
4. **F3-AC04 — Atomic source-to-IR result.** Clean inputs yield schema-verified
   IR and readable dumps. Syntax/type/profile errors, recovered input and
   resource limits yield owned diagnostics without a successful module.
   Evidence: malformed/unsupported inputs, exact source spans, bounded failure
   tests and repeated independent requests. Explicitly cover syntax errors and
   message quality (Adrian, 15 October 2026): missing THEN/END, misplaced ELSE,
   malformed expressions/calls, unterminated strings/comments, invalid bytes,
   Unicode/CRLF coordinates and unsupported-profile distinctions. Use a reusable frontend service;
   complete installed compiler CLI/RXAS output remains STEP04.
5. **F3-AC05 — B-only delivery and explanation.** Both frontend hosts build
   with B/L imports only, run on both concrete VMs in the maintained modes, and
   retain existing AST/IR regression behaviour. Keep architecture, source-tree
   schemas, binding/lowering algorithms, user guide and paired generated stage
   dumps current. Evidence: product build, focused regressions, dependency
   manifests and documentation/fixture cross-checks.

Implementation sequence, serving LLCOMP-AC03–05/08/09/15:

| Step | Work | Acceptance / dependency |
|---|---|---|
| F3-STEP01 | Implement both grammars and purpose-built source trees; demonstrate grammar admission and diagnostics. | F3-AC01/04; review approval first. |
| F3-STEP02 | Implement G binding/types and Classic pool/routine/context tables; extend semantic families and build expression/control IR, including SAY. | F3-AC02/04; after F3-STEP01. |
| F3-STEP03 | Complete local/recursive calls, argument and RESULT handling, and the required Level B import-contract reader/mapping. | F3-AC02–04; after the corresponding F3-STEP02 contracts. |
| F3-STEP04 | Qualify all POC rows at the frontend boundary; record generated examples and update architecture/algorithms/manual. | F3-AC01–05; all earlier steps. |


### Emission-quality follow-through

**Vision:** preserve the working C/G path while making its source maps truthful
and its baseline output proportionate to the source. Establish structural
regressions now and use both frontends to drive later shared IR improvements.
The original functional qualification established execution, not a code-quality
pass. The review's small controls are not representative timing measurements.

1. **EQ-AC01 — Source identity and fidelity (Phase A identity/ranges verified; full clause/TRACE gate open).** Recompiling the same
   logical source under two physical roots preserves emitted source identity
   and deterministic output. Test CLI and service, colliding basenames and
   explicit source mapping, Unicode/CRLF/multiline spans, derived/generated
   origins and multiple expressions in one clause. Generated setup must not
   masquerade as exact authored text. Preserve required TRACE/error mappings.
2. **EQ-AC02 — Source-state emission (verified for the admitted profile).** Structural checks reject redundant
   unchanged `.srcstep` records and Classic location loads within a straight
   path. Branch entries, loops, calls and failing/skipped operands still report
   the correct authored location. Generated entry handling has a documented,
   meaningful fallback. Do not globally deduplicate address-bearing records.
3. **EQ-AC03 — Proportionate baseline construction (verified for the admitted profile in STEP05).** `SAY expression`
   and assignments construct only their selected operand/default. Empty `SAY`
   retains its behaviour. Call windows reflect actual selected target calls,
   with no fixed eight-argument floor in a call-free body; entry-to-next-label
   jumps are absent where fall-through is equivalent. Stable distinct value
   registers remain permitted pending the separately reviewed allocator work.
4. **EQ-AC04 — Explain and improve real target costs (open).** Paired C/G and
   native B/G/C controls retain instruction/directive/call/frame counts and
   representative RXAS, plus expected execution and error results. Explain
   argument flags/cleanup and numeric snapshots. Inventory work in target
   helpers; reducing caller text alone cannot close a cost finding. Review the
   Classic target contract before changing it. Shared passes must demonstrate
   named redundant operations removed without changing observable semantics.
5. **EQ-AC05 — Durable qualification (verified for direct Phase A repairs; wider gates open).** Add permanent structural and
   semantic regressions for repaired findings, both assembler modes and concrete
   VMs where affected, with source metadata retained. Keep examples, architecture
   and user guide current. Reuse unchanged qualification; do not claim runtime
   speed, full TRACE conformance or hosted success from these bounded checks.

| Step | Criteria / dependencies | State and owner |
|---|---|---|
| EQ-STEP01: inspect STEP04 RXAS, compare native B/G/C, retain evidence and classify findings | AC16; identifies EQ-AC01–05 closure evidence | Complete as a review; [current plan](PLAN.md). |
| EQ-STEP02: repair source identity/provenance/grouping, redundant source-state emission, unused default construction and needless frame/entry scaffolding | EQ-AC01–03/05; review source-identity and clause-mapping surface before implementation | CP essentials and STEP05 unused-default/frame/fallthrough repairs verified locally. Real clause grouping remains a later reviewed gate. |
| EQ-STEP03: review Classic activation/numeric/helper lowering with paired typed controls | EQ-AC04; inspect actual helper work, error/lifetime obligations and INTERPRET compatibility | Bounded condition-provenance correction completed below; broader numeric/activation work remains open. No automatic native-C/runtime rewrite. |
| EQ-STEP04: execute approved shared passes and subsequently review liveness/register reuse | EQ-AC04/05; LLCOMP-STEP05 pass gates and EQ-STEP02 baseline | Shared passes verified in STEP05; liveness/register reuse remains open for review. Simple stable allocation is not claimed optimal. |
| EQ-STEP05: retain focused structural/semantic proof and regenerate the walkthrough with each repair | EQ-AC01–05; affected implementation and existing publication rules | Direct repairs have permanent STEP05 regressions; STEP06 adds the consolidated walkthrough/coverage. Wider semantic gates remain open. |

F4-AC07 is the bounded STEP04 quality gate covering EQ-AC01–03 and EQ-AC05 for
their direct repairs. EQ-AC04's broader pass/target costs remain open through
STEP05/06 and AC13/16. Any deliberately retained excess needs an explicit
reason and closure gate; passing against verbose native Classic output is not
sufficient.

### Essential metadata and condition-provenance correction

**Vision:** correct misleading source metadata, physical filenames and the
loss of the original fault during Classic error re-raising, using existing
Level B facilities and Level L emission. Keep the current diagnostic structure
and retain needed authored context. No new C/G syntax, condition framework,
source-map companion, IR format, VM API or stack mechanism is proposed.

The [current plan](PLAN.md) provides the exact surface,
mechanism and paired C/G error example. On 15 October 2026 Adrian authorised the
design/plan update, then explicitly narrowed its priority to essential
corrections rather than a wholesale redesign. Accordingly the proposal keeps
function-level Classic handlers and their necessary location state; new adapter
handler placement and a source-map companion are not part of this increment.
Adrian approved implementation of this bounded correction on 15 October 2026.
CP-STEP01–03 and CP-AC01–06 are verified for this bounded profile. The
[current plan](PLAN.md) retains actual product,
structural, error, service and example proof. Adrian subsequently authorised
commit/push to develop and switching the lightweight CI monitor on 15 October
2026. This publication reuses the unchanged qualified inputs; it includes the
emission-quality instructions, review, regressions, examples and documentation.
Wider redesign, the known STEP04 package dependency repair, hosted results and
full programme gates remain separate.

1. **CP-AC01 — Logical source names (verified locally).** Use the existing service filename
   argument and CLI: preserve deliberate logical relative names, convert absolute
   physical names to basename and retain `<source>` for memory input. The same
   logical input under two physical roots produces identical source identity
   and output. Test CLI/service, Unix/Windows path forms and deliberate distinct
   names for colliding basenames. No new CLI option is required.
2. **CP-AC02 — Honest source records (verified locally).** Verify complete single-line
   ranges, Unicode/CRLF, documented first-line multiline projection, truthful
   authored/generated/inherited/synthetic flags and no previous-routine mapping
   bleed. Use clause zero when no real authored clause association exists.
   Proper clause grouping remains open under EQ-AC01/AC09; this correction
   cannot claim full source/TRACE fidelity or close those broader criteria.
3. **CP-AC03 — Preserve fault and authored context (verified locally).** Keep the current
   function-level fatal handler and required location field. Format authored
   context on the error path and select literal `signalorigin` re-raises with
   the original bound event, preserving the original fault rather than resetting
   it to the formatting helper. Prove direct and deep-helper C/G errors, recursion,
   generated entry, branches/loops and a call followed by a different anchor.
4. **CP-AC04 — Preserve the admitted error contract (verified locally).** Retain signal
   name, message/error number, original module/address, failure timing and
   nonzero status. Audit current fatal string payloads; do not silently generalise
   to structured conditions. Prevent handler recursion and preserve ignored
   overflow/lost-digits policy. Full condition trapping, object-payload relay,
   embedded provenance fields and full backtraces remain later obligations.
5. **CP-AC05 — Proportionate diagnostic state (verified locally).** Remove redundant
   unchanged straight-path source records/location writes while preserving
   required updates at control-flow entries. Replace `generated Classic entry`
   with a truthful logical-file/routine-entry context, without an invented exact
   source line. Retain code-shape evidence and explain the remaining location
   field's role after VM unwinding. No claim that all such state has disappeared,
   and no unrelated numeric/ABI/register-allocation change.
6. **CP-AC06 — Focused proof and documentation (verified locally).** Add permanent
   structural/error regressions, including stripped metadata, untaken failures
   and independent invocations; run affected checks in both assembler modes
   and concrete VMs where supported, with normal/no-opt B-host controls where
   the changed handler code is affected. Reuse unchanged STEP04 qualification.
   Update architecture, examples and user guide when implemented. Full clause,
   TRACE, payload and INTERPRET lifetime obligations stay visibly open.

| Step | Criteria / dependencies | State and owner |
|---|---|---|
| CP-STEP01: prove origin-preserving relay in the existing fatal handler and audit admitted payloads | CP-AC03/04; approved | Complete; four fatal names preserve original fault/name/status, with a string-payload audit and recursive/helper/error evidence. No C/VM extension. |
| CP-STEP02: correct logical names, existing source records and entry context; deduplicate redundant state updates | CP-AC01/02/05; approved bounded correction | Complete; B frontend/driver/backend using the Level L writer, focused structural/Unicode/CRLF/identity regressions and retained code-shape comparison. No new map format or condition API. |
| CP-STEP03: integrate the relay, retain paired structural/error evidence and update examples/manual | CP-AC01–06; CP-STEP01 proof and CP-STEP02 mappings | Complete locally; product build, 31 CTests plus the retained fourth provenance profile, focused Apple ASan proof, reusable-package consumer and regenerated C/G examples. Publication/hosted gates remain separate. |

CP criteria contribute to EQ-AC01/02/05 and LLCOMP-AC09/15/16; they do not fully
close those wider gates. Real clause grouping, broader provenance and an
adapter-boundary alternative are named further work in the review. STEP05
subsequently verified EQ-AC03's unused defaults/frame layout and the approved
shared passes. EQ-AC04's broader numeric/helper specialisation and STEP08/AC11
INTERPRET requirements retain their existing scope.

1. **F5-AC01 — Clean unoptimised construction/emission (verified locally).** Explicit SAY
   and Classic assignment operands create no unused defaults; bare SAY and
   required variable defaults retain their meaning. Frames reserve only needed
   value, scratch, diagnostic and call-window registers, including hidden/cold
   calls and argument counts. Omit jumps to the next emitted block without
   losing anchors. Verify opt-off IR/RXAS shape, calls/recursion, register-budget
   boundaries and semantic/provenance regressions. Serves EQ-AC03/05 and AC16.
2. **F5-AC02 — Generic Level L services (verified locally).** Shared batch reconstruction,
   numeric definition/use/successor indexes, contract eligibility and pass
   driving live in Level L with no compiler imports or C/G-name tests. Preserve
   existing rewrite/lower interfaces and use one checked reconstruction per
   pass/batch rather than cloning the graph per fold. A non-C/G fixture uses the
   same services as both compilers. Inspect dependencies and retain pass-level
   tests and operation/storage bounds. Serves AC01/04/06/13/15.
3. **F5-AC03 — Explicit semantic admission (verified locally).** Register evaluators for
   actual frontend operations, not only the foundation demo names. Initially
   admit checked G int64 add/subtract/multiply/comparisons, exact int64-to-text
   and canonical Boolean conversion/truth; C truth admits only exact literal
   bytes `0` and `1`. Overflow, division, unknown conversions, mutable C NUMERIC
   context, pool reads and arbitrary calls retain runtime behaviour. Independent
   expected results and boundary/error cases verify each admitted rule; no
   generic pass infers semantics from a name. Serves AC05/06.
4. **F5-AC04 — Safe control and removal (verified locally).** Prove branch edges and remove
   unreachable blocks from all admitted function entries; preserve effects or
   failures in condition evaluation. Propagate only nonescaping immutable or
   uniquely defined block-local private temporaries. Remove reachable unused
   operations only with proved state/failure/continuation eligibility. Verify
   joins, loops, skipped/reached errors, short-circuiting, output, cleanup,
   unknown effects and invalidation at state/call boundaries. Serves AC05/06/09.
5. **F5-AC05 — Atomic ownership and provenance (verified locally).** Source graphs stay
   unchanged; operation/value/block mappings account for substitutions and
   removal. Fresh analyses/facts belong to the candidate revision. Verify exact
   constants, attributes, derived origins and every candidate before acceptance.
   `invalid`/`limit` exposes the original graph, never a partial successful
   transformation. Stale/foreign handles, refused services, budgets and repeated
   optimisation exercise failure safety, determinism and convergence. Serves
   AC04/06/09/13.
6. **F5-AC06 — Useful and independently checked output (verified locally).** Both languages
   show fewer unnecessary IR operations and RXAS instructions/calls on the
   paired examples. Compare original/optimised IR with an independent bounded
   reference evaluator, not the folding callback as oracle, and execute both
   outputs with assembler optimisation off first. Include appropriate existing
   B/G/C compiler controls; their inefficiencies are not acceptance targets.
   Record helpers, frames and metadata separately from executable counts.
   Keep source/error anchors and logical filenames truthful. Serves AC06/07/16.
7. **F5-AC07 — Controls, delivery and research qualification (verified locally).** The reviewed
   compiler `-n` switch and trailing service option select the cleaned baseline;
   frontend and emission graphs remain inspectable. Preserve existing request
   arguments, product/install selection and B-only bootstrap. Keep compiler,
   B-host and assembler optimisation modes distinct. Following Adrian's
   15 October clarification, formal performance testing and a separate Release
   verdict are not required for this research increment. Focused/normal suites,
   resource limits, emitted-code inspection, installed consumers, enduring
   architecture/user-guide updates and input-qualified validation summaries close the increment;
   hosted qualification remains separate. Serves AC02/12–16.
8. **F5-AC08 — Reusable source coordinates and emitter integrity (verified locally).** Move
   duplicated byte-offset/line/column lookup into a bounded request-owned Level
   L source index used by diagnostics and C/G emission. Preserve Unicode scalar
   columns, CRLF/CR/LF, EOF, multiline spans and malformed-byte fallback, with
   independent Level L tests. Retain the existing writer and prove chunk/section
   origins, error propagation, output limits and sink failures after graph
   remapping. Source presentation and RXAS control/ABI policy remain compiler
   responsibilities. This introduces no TRACE/condition redesign. Serves
   AC07/09/13/15 and EQ-AC05.

### Numbered implementation steps and gates

| Step | Work and criteria | Dependency / gate |
|---|---|---|
| **F5-STEP01 — complete** | Repair direct default/frame/fall-through defects with optimisation off; extract shared source-coordinate lookup. Retain opt-off shape and focused provenance/ABI/diagnostics tests. F5-AC01/08. | Design approved; all code Level B. The first cleanup verdict was accepted; later formal research measurement gates are waived. |
| **F5-STEP02 — complete** | Extend Level L's rebuild coordinator, indexes and bounded pass driver; prove with a non-C/G fixture and failure/lifecycle tests. Preserve existing service contracts. F5-AC02/04/05/08. | Approved pass/API contracts; generic work must not import the compiler. Integrate only verified candidates. |
| **F5-STEP03 — complete** | Register the admitted C/G evaluators and wire the shared pipeline, graph access and opt-off controls. Add the independent IR oracle and paired execution/code-shape cases. F5-AC03/06/07. | F5-STEP02; focused correctness and code-quality evidence. Unproved folds remain runtime operations. |
| **F5-STEP04 — complete** | Complete the relevant normal suite and installed/B-only checks, update architecture and user guide, retain exact-input evidence and acceptance status. F5-AC01–08. | Reuse unchanged valid evidence; measure aggregate scheduling before registering expanded nested tests. Develop publication authorised 15 October. STEP06 and full replacement gates remain open. |


### Vision and intended outcome

A reader can follow a small C and G program from its purposeful source tree,
through verified original/optimised IR, into readable RXAS and installed
execution. A single coverage/evidence matrix explains what each language
preserves, what Level L supplies, how failures are reported, and which platform
or full-replacement obligations remain open. Both compilers must improve the
shared services without making those services depend on Classic Rexx.

Reuse the qualified STEP02–05 tests and validation summaries after checking their input
identities. Add only missing decisive cases or repair a demonstrated defect.
Formal performance testing is not required under the standing Level L research
exception. Work/storage limits, useful output shape and observed pathological
cost still matter. Full TRACE/conditions, INTERPRET, SSA/register reuse,
inlining and broader language/ABI coverage remain their named later gates.

### Numbered acceptance criteria

1. **F6-AC01 — Profile and semantics (verified locally for the agreed PoC).** Map every POC-01–10 row to source,
   expected behaviour and evidence through AST, IR, optimised IR, RXAS and
   execution. Include agreement and intentional C/G differences: literal bytes,
   numeric context, variable defaults/unset reads, eager/skipped operands,
   calls/arguments and control flow. Reuse independent expected values and the
   non-C/G oracle; old compiler output alone is not the specification.
2. **F6-AC02 — Failure and ownership (verified locally for the agreed PoC).** The same matrix covers syntax,
   profile/type, malformed source/import, unknown-contract and budget failures,
   together with reached/skipped runtime errors, Unicode/CRLF origins,
   repeated requests and owned results. Failure preserves existing destination
   files and exposes no partial executable. Any discovered gap gains a focused
   permanent regression before closure.
3. **F6-AC03 — Emitted-code quality (verified locally for the agreed PoC).** Retain a few complete before/after
   examples and structural assertions: meaningful source names/spans, no unused
   default construction or gratuitous frame/call machinery, and required C ABI,
   numeric/activation/error work preserved. Separate executable counts, helpers,
   registers and metadata; classify remaining conservatism against named later
   passes. Static counts establish no execution-speed claim.
4. **F6-AC04 — Bootstrap and installed use (verified locally for the agreed PoC).** Retain the B/L-only compiler
   host closure, opt/no-opt standalone bootstrap and installed CLI/service
   evidence. Demonstrate compilation with old compiler commands unavailable to
   the new drivers. Inventory native seed/binutils, generated launchers and the
   separate Classic target-runtime dependencies. Proper B self-hosting remains
   STEP07B; it must not be claimed from this seed-built PoC.
5. **F6-AC05 — Modes, platforms and resource boundaries (verified locally for the agreed PoC).** Consolidate
   B-host, IR and assembler optimisation modes and both supported VMs, keeping
   each axis distinct. Match local/normal hosted evidence to exact inputs;
   record unavailable or untested platform cells explicitly, including
   constrained-host applicability. Close the Phase A supported-path checks
   without claiming full AC12 platform qualification. Exercise deterministic
   work/index/byte/register refusals; no formal timing campaign, deep matrix
   dispatch or new production-replacement gate is implied.
6. **F6-AC06 — Walkthrough and review decision (verified locally for the agreed PoC).** Update the enduring
   architecture/user guide with a reproducible paired example, actual tree/IR/
   RXAS views, failure variants and installed commands. Summarise improvements
   and gaps in Level L separately from C/G coverage. Present the resulting PoC
   assessment and proposed balanced STEP07 increments for approval. All
   full-replacement criteria remain open beyond their proved Phase A portions.

### Numbered implementation steps

| Step | Work and acceptance | Dependency / decision gate |
|---|---|---|
| **F6-STEP01 — complete** | Audit existing POC-01–10 coverage, exact input identities and dependency/mode/platform evidence; identify only missing proof. F6-AC01/04/05. | Approval of this bounded proposal; reuse STEP05 qualification. |
| **F6-STEP02 — complete** | Fill decisive semantic/failure/ownership/code-shape gaps with Level B consumers and shared non-C/G tests; fix demonstrated in-scope defects. F6-AC01–05. | F6-STEP01. New language/IR/architecture decisions require separate review. |
| **F6-STEP03 — complete** | Regenerate the paired walkthrough and update architecture, guide and criterion/evidence matrix. F6-AC03/06. | Qualified implementation inputs; do not repeat unchanged broad tests. |
| **F6-STEP04 — complete** | Present the complete Phase A assessment, remaining obligations and proposed STEP07 coverage increments. F6-AC01–06. | PoC review gate; no default compiler replacement or later implementation without approval. |


admitted C runtime-call contracts, with independent Level L checks. It contains
the paired `scale(7)` / `scale(7,3)` example, tree/IR responsibilities,
F7A-AC01–06 and F7A-STEP01–04. The [approved contract](doc/STEP07A-CONTRACTS.md) records the inventory and
exact public API; its decision gate is complete. Native
B/G/C controls passed 38 commands with 14 VM executions, including separate G
provider/consumer metadata and missing-required rejection. This is reference
evidence, not implementation qualification. The new qualification covers both
B-host modes and VMs, independent Level L, native/new module interoperability,
installed CLI/service requests and focused Apple ASan checks. This increment
is included in the develop publication authorised on 17 October; no hosted
STEP07A result is claimed.
The active C-only decision above supersedes the earlier balanced C/G
expansion sequence. Classic coverage and STEP08 now lead; STEP07B is deferred.
Its full self-hosting obligations and the INTERPRET mechanism gate are retained.


### Instruction review 01: SAY

**Vision.** Review the complete LC-I-01 / LC-AC-57 obligation through the new
compiler, rather than inheriting the native compiler's closure. Preserve one
expression evaluation and one ordered output operation, exact Unicode text and
NUL, useful errors and economical RXAS. Shared expression, activation, host and
TRACE gaps remain named dependencies; a small passing corpus does not close
whole SAY. The [instruction review](doc/LEVELC-INSTRUCTION-REVIEW.md) holds the
coverage ledger and findings, subordinate to this plan.

The first minimal defect is **CFC-SAY-DEF01**: `say '410042'x` prints `410042X`
in both IR modes. The lexical adapter inserts concatenation between the string
and suffix; the expected output is `A`, NUL, `B`, LF under the accepted fixed
ordinal bridge. This is a shared literal defect exposed by SAY. Existing
Classic syntax and ordinal semantics are already agreed; no new public Level L
API, grammar syntax, IR operation or runtime facility is proposed.

The [retained SAY validation](#say-validation) owns the bounded repair
criteria/results. Its successful subset does not close whole SAY.

## Qualification and documentation practice

- Compare each frontend with its own language semantics and existing compiler;
  do not require C and G to agree where their contracts differ. Supplement
  compiler comparisons with independently stated expected results and Classic
  references so a shared bug is not treated as proof.
- Record compiler optimisation, assembler optimisation and host compiler build
  mode separately. Retain exact source, generated grammar, runtime/library and
  tool identities for every claimed comparison.
- Use small permanent regressions and the relevant normal correctness suite.
  Publish substantive approved increments only after their required gates pass,
  following AGENTS.md. Normal hosted checks remain distinct from local evidence;
  no status-only publication or routine overnight matrix dispatch is required.
- Follow [performance governance](../../../performance/PERFORMANCE-GOVERNANCE.md)
  and the [performance instructions](../../../performance/AGENTS.md) for
  formal measurements when separately requested or required for later production
  replacement. The root `AGENTS.md` Level L research exception governs ordinary
  research increments; formal benchmarking is not their completion gate. Use the existing
  benchmark corpus where applicable; new performance tooling is Level B.
- Keep the Level L user guide an enduring use manual. Keep current plans,
  unresolved decisions and concise validation summaries here or in their
  owning maintained document; delete raw output and obsolete notes.
  New features enter the manual when implemented and verified.
- Update this plan after each approved increment: criterion/step status,
  evidence, remaining coverage, exact scope decisions and next gates. Read it
  together with AGENTS.md and the parent Level L plan after session takeover.

## Current acceptance record

| Item | State |
|---|---|
| Architecture direction: cREXX compilers, Level L-owned extensible numeric IR and parallel documentation | Agreed; detailed contracts presented in STEP01. |
| STEP01 | Reviewed and approved as the STEP02 foundation: scope, architecture, trees, IR/API, algorithms, dependency inventory and six source controls. [current plan](PLAN.md). |
| STEP02 / foundation implementation | IR2-AC01–07 verified locally; [current plan](PLAN.md). Approved develop publication includes baseline `d13a91c1b`; hosted outcomes reported separately. |
| STEP03 / frontend increment | F3-AC01–05 verified locally; [current plan](PLAN.md). Published as `bcfb068b7abe2f3a2b9a23232fdb4244cc8ffa67` with the CI preparation repair. Hosted results are separate. |
| STEP04 / executable backend | Published `bf74c72111fbd466d1f93c421e6d5dadbbedcc2c`; original [current plan](PLAN.md) retained. CP provenance and STEP05 construction/emitter cleanup close the direct Phase A defects with retained evidence; full clause/TRACE and wider target-cost work remain open under EQ/AC16. Hosted results are separate. |
| Essential metadata/condition correction | CP-AC01–06 and CP-STEP01–03 verified locally; [current plan](PLAN.md). Published as `f14d8a569a8ced9420fd27e8d6277db0617825db`. Existing diagnostic structure retained; original faults now survive the relay. Wider redesign and hosted outcomes remain separate. |
| STEP04 product-stage repair | PKG-AC01/02 verified locally; PKG-AC03 regression/docs/evidence complete and publication approved. [current plan](PLAN.md). The missing-producer repair was qualified. The approved reorganisation now selects these products through the explicit research stage/component; hosted delivery remains separate. |
| STEP05 / shared Level L optimisation | F5-AC01–08 and F5-STEP01–04 verified locally; [current plan](PLAN.md). Level B-only shared services, C/G integration, independent tests, code-quality examples, B-only bootstrap and installed delivery complete. Formal research performance tests are not required. Develop publication and lightweight CI-monitor handover authorised; hosted outcomes separate. |
| STEP06 / remaining Phase A | F6-AC01–06 and F6-STEP01–04 verified locally for the agreed C/G PoC; [current plan](PLAN.md), [coverage](doc/poc-coverage.md), [walkthrough](examples/step06/README.md). F6-PREP-01 repaired and locally qualified; published as `4eca26ac8335a22dd74f84250696ba68303c9958` with repair `7326287c5`. Wider replacement/platform gates remain open. |
| STEP07A / scalar calls and modules | Bounded increment and [exact API](doc/STEP07A-CONTRACTS.md) implemented and locally qualified: F7A-AC01–06 complete. Develop publication authorised 17 October 2026 with the bounded SAY repair; hosted results separate. No STEP07B or later coverage approval implied. |
| C-only delivery / STEP07 and STEP08 | Architecture and CLC-STEP01–11 approved 19 October. CLC-STEP01a/02a/03a implement the first bounded foundation increment, including shared B Unicode extraction. CFC-AC01–12 remain open for complete C; INTERPRET implementation is last. Exact new API/ISA gates remain binding. |
| STEP07B / proper B and further G expansion | Outside current delivery, not automatically resumed. AC02/self-hosting and remaining G criteria stay open with stable IDs. |
| STEP09/10 / combined full replacement | Planned destination; all full B/C/G coverage, delivery and explicit default-switch gates remain open. |
| LLCOMP-AC01–16 | Open for their full compiler scope. STEP02 establishes the bounded IR foundation portions; AC16 now explicitly requires emitted-code quality. |
| Existing Level L AST services | Published separately; see the parent plan's AST7 evidence. |
| Existing production compilers and bootstrap | Existing routing retained. Seed-built Level L now includes the IR modules; proper B remains STEP07B. |

## References

- [Level L AST contracts](../AST-CONTRACT.md)
- [Level L user guide](../../../langtools/doc/user-guide.md)
- [cREXX architecture](../../../docs/ai-context/CREXX_ARCHITECTURE.md)
- [Level B implementation guide](../../../docs/ai-context/CREXX_LEVELB_AUTHORING.md)
- [Classic architecture](../../../compiler/docs/levelc_working_architecture.md)
- [Metadata conformance](../../../compiler/docs/metadata_conformance.md)
- [RXAS](../../../docs/ai-context/RXAS_ASSEMBLER.md),
  [RXLINK](../../../docs/ai-context/RXLINK_LINKER.md) and
  [RXVM](../../../docs/ai-context/RXVM_INTERPRETER.md)



## Retained acceptance contracts

These accepted criteria and step IDs remain binding. Completed build/documentation
increments are historical; unverified language, platform and replacement gates
remain open. The reorganisation does not reopen unchanged semantic results or
claim a new platform pass. Current ownership is described above.

1. **QA3-AC01:** The optimizer-parity preparation graph contains all STEP02
   demo/contract and STEP03 frontend test producers for opt/no-opt modes.
   Verify the generated target graph and rebuild deliberately absent test images
   through `qa-prep-optimizer-parity`, without invoking their producers directly.
2. **QA3-AC02:** The eight STEP02 tests and 24 STEP03 tests pass on both VMs
   after preparation. Verify the product build and the unchanged retained STEP03
   source/artifact/example hashes; reuse broader unchanged qualification.
3. **QA3-AC03:** Commit only the repair and approved STEP03 paths, publish to
   `develop`, verify the remote SHA and switch the existing lightweight monitor
   to that SHA and its normal Build/CodeQL runs. Hosted success is reported
   separately from local qualification.

| Step | Work | Acceptance / dependency |
|---|---|---|
| QA3-STEP01 | Complete directory QA tier/preparation finalization using the existing repository helper. | QA3-AC01; approved repair scope. |
| QA3-STEP02 | Recreate absent test images via CI preparation, run focused tests and product build, record evidence. | QA3-AC01/02; after repair. |
| QA3-STEP03 | Publish the repair and STEP03, verify remote identity and retarget monitoring. | QA3-AC03; after local gates pass. |

The following PKG criteria describe the repaired historical packaging mechanism;
RA-AC07 now owns the approved research opt-in delivery boundary.

1. **PKG-AC01 — Complete product selection (verified locally).** The Level L product stage
   includes `llcompilers`, `llctarget`, `llrxc-c` and `llrxc-g`. After their
   generated delivery files are removed, `stage-product` alone recreates them.
2. **PKG-AC02 — Installed use (verified locally).** Install to a fresh temporary prefix;
   verify the four products, run the installed C/G pipeline examples and compile
   a B service consumer using only installed packages. Retain command evidence.
3. **PKG-AC03 — Durable delivery check and publication (local proof complete; approved publication).** Add a focused
   regression for product completeness, document the stage contract, retain
   exact inputs/results and publish the repair to develop. Switch the existing
   lightweight CI monitor; hosted platform/signing results remain separate.

| Step | Criteria / dependencies | State |
|---|---|---|
| PKG-STEP01: restore the four Level L compiler products to the stage selection | PKG-AC01; failed SDK logs and stage graph | Complete; all four producers were required by the then-current product stage; the research component now owns their selection. |
| PKG-STEP02: check missing-output rebuild, fresh install and installed consumers | PKG-AC01/02; PKG-STEP01 | Complete; old stage reproduced the omission, repaired stage rebuilt all four byte-identically, full install and 13 installed checks passed. |
| PKG-STEP03: retain regression/docs/evidence and publish with monitor handover | PKG-AC03; PKG-STEP02 | Regression/docs/evidence complete; approved develop publication and lightweight monitor handover. Hosted results reported separately. |

1. **F6-PREP-AC01 (verified locally):** a fresh focused Unix Makefiles graph using the real
   bootstrap CMake declarations reproduces duplicate execution before the fix
   and passes after it. Include missing-output, changed-input and no-op checks.
2. **F6-PREP-AC02 (verified locally):** rebuild the missing real L2 output with parallel
   preparation in normal Debug and maintained Apple ASan; verify the original
   preparation target succeeds, each producer executes once and qualified
   generated products retain their bytes. Reuse unchanged semantic evidence.
3. **F6-PREP-AC03 (qualification/docs complete; publication authorised):** update the validation summary, architecture/guide and open-finding
   disposition; publish only approved paths, verify remote `develop`, switch
   the existing lightweight monitor and present STEP07 for review only.

| Step | Acceptance and dependency |
|---|---|
| **F6-PREP-STEP01 — complete** | Reproduce with actual CMake declarations and record target/file ownership; AC01. |
| **F6-PREP-STEP02 — complete** | Add producer ordering, retain a permanent regression and qualify real parallel preparation; AC01/02, after reproducer. |
| **F6-PREP-STEP03 — publication authorised** | Reuse verified STEP06 evidence, document and publish repair plus STEP06, hand over monitoring and propose STEP07; AC03, after focused checks. |

### SAY validation

These are retained results for the published radix repair, not a new execution
run or whole-instruction verdict. The earlier foundation checkpoint and
instruction ledger delimit their scope.

1. **CFC-SAY-AC01 (reviewed):** classify both SAY forms, expression/order/error,
   Unicode/host and source/TRACE obligations, with implementation, evidence and
   remaining owner for each. Inspect the existing LC instruction contract.
2. **CFC-SAY-AC02 (verified locally):** recognise an adjacent single-symbol X/B suffix only
   for C string tokens. Preserve complete authored spelling/span in the existing
   String node; spaced suffixes and longer identifiers remain concatenation.
   Validate digit/group rules, empty and partial leading groups, and decode to
   U+0000–U+00FF constants at compile time. Prove all 256 ordinals, malformed
   inputs, no successful IR/output on refusal and unchanged G admission.
3. **CFC-SAY-AC03 (verified locally):** permanent C end-to-end cases cover empty, ordinary
   and ordinal text, numeric display, nested/procedure output ordering and
   expression failure before output. Check both IR modes and VMs, with RXAS
   assembled without optimisation; native controls do not define expectations.
4. **CFC-SAY-AC04 (reviewed; wider quality open):** inspect complete simple/error RXAS plus helper cost;
   assert no X/B pool read or runtime decoder for a literal, no accidental host
   source path, and one output per SAY. Classify remaining setup/register costs
   under existing owners rather than claim optimal code or hide costs in helpers.
5. **CFC-SAY-AC05 (verified for this repair):** keep implementation B-only and Level L-based; run
   focused C/G frontend checks and the product build, reuse unaffected Level L
   evidence, and update architecture/user documentation. Whole LC-I-01 remains
   open wherever host/TRACE/shared-language proof is missing.

| Step | Work / acceptance | Dependencies |
|---|---|---|
| **CFC-SAY-STEP01 — reviewed** | Review existing forms and source/IR/RXAS; retain the independent minimal reproduction and ledger. AC01/04. | Existing STEP07A products. |
| **CFC-SAY-STEP02 — repaired** | Merge the adjacent suffix at the C token-layout boundary; validate and decode using B ordinal helpers before creating the existing IR constant. Add positive/negative frontend regressions. AC02/05. | STEP01, existing approved Classic syntax/Unicode contract. |
| **CFC-SAY-STEP03 — qualified locally** | Focused end-to-end/order/error/shape tests, small isolated scheduling measurement if a nested test is registered, and product build; retain exact input evidence and documents. AC03–05. | STEP02. No broad or formal performance gate. |
| **CFC-SAY-STEP04 — disposition recorded** | Report SAY-specific results and every unclosed shared dependency before reviewing DROP (LC-I-02). AC01/05. | STEP03. INTERPRET remains deferred. |
