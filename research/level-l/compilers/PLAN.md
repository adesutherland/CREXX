# cREXX compilers built with Level L

The compiler programme is experimental and implemented in Level B. STEP01–06
and STEP07A are published; the bounded SAY radix-literal repair is published.
The repository reorganisation is implemented and locally verified in main
`develop`, with publication pending; [the owning plan](../PLAN.md#repository-architecture-and-cleanup)
contains its RA acceptance criteria. This does not close full C, B or G coverage.

Classic instruction completion takes priority. Review each instruction against
its own semantics, diagnostics and RXAS quality. Preserve current G behaviour;
B/G share the `typed/` implementation. Proper B/self-hosting (STEP07B) and
INTERPRET (STEP08) remain deferred. See the
[instruction ledger](doc/LEVELC-INSTRUCTION-REVIEW.md),
[STEP07A contracts](doc/STEP07A-CONTRACTS.md), [architecture](doc/architecture.md)
and [user guide](../../../langtools/doc/user-guide.md).

## Current delivery and validation boundary

| Stage | Delivered scope | Remaining boundary |
|---|---|---|
| STEP01 | Agreed C/G PoC and IR/semantic contracts | Full-language contracts remain separately gated. |
| STEP02 | Checked extensible IR, semantic services and writer | General replacement and resource criteria remain open. |
| STEP03 | C/G source-to-IR frontends and diagnostics | Coverage is the declared PoC. |
| STEP04 | Source-to-RXAS services and commands; essential provenance fixes | Full clause/TRACE semantics and wider allocation optimisation remain open. |
| STEP05 | Generic propagation, admitted folding, pruning and emitter cleanup | No formal performance verdict is required for Level L research. |
| STEP06 | End-to-end PoC coverage, installed use and build-order repairs | Local qualification is not full platform/replacement qualification. |
| STEP07A | Exact constants, explicit arguments/presence, checked callable contracts | Does not implement proper B or complete Classic. |
| C-first | Instruction-by-instruction work; SAY radix literals repaired | Whole SAY, host output/conditions/TRACE, remaining instructions and BIFs stay open. |
| STEP07B/08/09/10 | Deferred/planned | Proper B, INTERPRET, complete qualification, then explicit replacement approval. |

Before reorganisation, the published baseline was
`06a548321662e6dc4e91899363eb12831b4d952c`. Its retained conclusions include
STEP07A's 99-command matrix, the 40-command SAY regression and earlier PoC,
provenance, independent Level L and bootstrap coverage. These are historical
local results; the changed source/build boundaries are covered by the focused checks in
the RA plan. Raw run logs, hashes and receipts have been deleted. Permanent
regression inputs and readable examples remain under `tests/` and `examples/`.
CI monitoring remains stopped. Publication is a separate instruction.

## Vision and intended outcome

Build new Level C and Level G compilers in cREXX using Level L. Each frontend
recognises its own language and preserves its semantics, then produces a common
intermediate representation (IR). Shared analyses and optimisations transform
that IR before the owning C or typed selector emits RXAS through shared Level L
writer and target utilities. The existing assembler, linker and
VM execute the result.

The result must also be economical and intelligible at the IR/RXAS boundary:
simple source should not acquire unnecessary setup, misleading source maps or
accidental host paths. Necessary Classic semantics remain explicit, but the
existing Classic compiler's expansion is not a quality target. Source fidelity,
structural code quality and measured replacement budgets accompany functional
correctness throughout development, rather than being postponed until final
replacement (AC16).

These compilers are intended to replace the production C-language compiler
routes when their complete replacement criteria pass. The first PoC covers C
and G; the complete destination also includes the proper Level B compiler,
written in Level B and built through Level L. Until qualified, all three remain
separate, explicitly selected tools beside the existing compilers. The retained
C-language Level B compiler supplies the initial seed. It does not remain the
normal B compiler after replacement. Retiring the native seed itself is outside
this programme; its re2c and other native build prerequisites remain explicit.

Adrian clarified this bootstrap destination during STEP01 review on 15 October
2026: native B seed, Level L, proper B, rebuild/verify B and L, then downstream
G and C. That remains the final self-hosted build order. On 16 October Adrian
reordered development delivery: complete C using the existing native B seed
before implementing STEP07B. C's compiler host depends on Level B and Level L,
independently of G. C functionality does not require the new B compiler to exist.

Level L remains a general language-engineering toolkit. Developing both C and G
drives its requirements: Classic dynamic state, numeric context and conditions
must coexist with G types, references, classes and extensions. The extensible
IR, its schemas, compact storage and views belong in Level L,
along with reusable tree, graph, analysis, transformation and emission services.
Compiler libraries define C/G operation families, language semantics and cREXX
calling conventions. Existing settings, calculator and RexxScript consumers continue to exercise the generic services.

The first delivery is a genuine two-frontend compiling PoC. It must execute
programs from both languages through the same IR, optimisations and cREXX RXAS
backend. A recogniser, an IR dump, or delegation to the old compiler does not
complete that delivery. Full language migration and production replacement are
later phases with separate gates; they remain part of the intended end state.

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
  compiler algorithms and new performance tooling stay in cREXX. No new C
  runtime, VM or linker implementation is authorised by this plan. If a required
  semantic operation cannot be expressed through the existing platform, retain
  a reproducer and bring the missing contract back for a decision.


## Delivery phases and initial language coverage

### Phase A: two-frontend compiling PoC

The initial paired corpus covers literals, scalar assignment and reads,
arithmetic/comparisons, IF/ELSE, a pre-test loop, procedures/calls/returns and
SAY. G inputs use explicit types in the admitted profile; Classic inputs use
RexxValue/pool semantics and their configured numeric rules. Each frontend has
a Level L grammar and semantic mapper; neither translates into source for the
old compiler. Pin precise forms and exclusions in STEP01 before implementation.

Include cases where the languages agree and cases where they must differ:
numeric representation/conversion, literal spelling, eager versus skipped
operands, variable state, argument/result handling and diagnostics. Use one
supported runtime-library call path so calling/import contracts are exercised,
not only standalone arithmetic. Choose parser backends from the current
supported portfolio and justify any composite stages from actual grammar needs.

Complete source -> IR -> optimised IR -> RXAS -> linked execution for both
languages. Provide unoptimised and optimised stage dumps and a small independent
cREXX IR evaluator for the admitted operations, alongside existing compiler
and reference-language controls. The evaluator is a test oracle, not a runtime
fallback or a second INTERPRET implementation.

### Phase B: language and product coverage

Grow the same pipeline towards full C/G replacement. The coverage matrix must
include every currently supported source feature, import/metadata contract,
diagnostic and tooling entry of the routes being replaced. Track new upstream
features while the implementations coexist. Do not redefine replacement as
parity only on the original PoC subset.

The active delivery sequence now completes C first, through the C-first
milestone below, while retaining current G regressions and independent Level L
consumers. G expansion and proper B/self-hosting under STEP07B are deferred
until after that milestone. Their full coverage remains required for eventual
combined replacement. B and G may share typed syntax and semantic code where
their contracts agree; the B build must not require generated G services or
the G compiler. The final self-hosted product build order begins with B and L.

For G this includes classes/interfaces, references and ownership, arrays,
numeric modes, imports/providers, initialisers, task/parallel and extension
contracts where supported. For C it includes the whole-instruction/BIF matrix,
PARSE, stems/exposure, numeric state, calls/arguments, labels/SIGNAL/conditions,
TRACE, ADDRESS, Unicode, streams and host contracts. Reuse the shared runtime
algorithms through explicit ABI calls rather than implementing a second set of
Classic BIF semantics in the compiler. Existing compiler bugs are recorded and
resolved against the language contract, not copied merely to match output.

### Phase C: reusable compilation and INTERPRET

STEP08 is part of C completion, with its mechanism review brought forward
before late C closeout. The phase number does not require waiting for STEP07B
or full G coverage. Its semantic and lifetime gates remain unchanged.

Build the drivers over a cREXX request/result compile service from the start:
exact source length, owned diagnostics/artifacts, explicit imports/options,
work/storage limits, failure cleanup and no CLI process exit in the service.
Normal module compilation and later interpreted-group mode share the frontend,
semantic lowering, IR, optimiser and RXAS backend.

Before execution of generated Classic code, prove the activation contract:
current pool and replacement of that pool by PROCEDURE; arguments/presence and
RESULT; NUMERIC/ADDRESS/TRACE and condition state; local routine/label access;
RETURN/EXIT/SIGNAL; handler suspension and resumption; nested INTERPRET; and
authored/generated diagnostic origins. A fresh ordinary procedure call or a
completion enum alone does not establish these semantics.

The existing VM exposes loading but no public individual-module unloading.
Select and prove bounded generated-code ownership, retention and reclamation;
a bounded lookup cache with unbounded resident modules is insufficient.
Code may be cached only under an explicit semantic/dependency key and must not
retain an expired caller activation. Keep standalone compiler/assembler stages
available for constrained hosts. A generated-code mechanism that requires new
C/VM support is a decision gate with an unmet criterion, not permission to
violate the cREXX-only implementation requirement.

### Phase D: production replacement

The C-first milestone qualifies a complete, explicitly selected C tool and
service independently of new B self-hosting. It does not close the combined
B/C/G replacement criteria or authorise a default-routing change. STEP09/10
retain those obligations and the explicit replacement decision.

Switch default compiler routing only after the full coverage, bootstrap,
platform, lifecycle and delivery criteria pass and Adrian approves the switch.
Retain the documented C-language bootstrap tool and required existing runtime,
assembler/linker/VM tools. Remove obsolete production compiler routes, build
entries and documentation together after proving there is no hidden dependency
on them. Preserve a tested rollback path for the routing transition.

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

Current architecture/reference pages describe what is implemented. Proposed
shapes remain visibly proposed in this plan or review material until accepted.
Use tested fixture dumps/excerpts where practical so examples cannot silently
drift from code. Keep chronology and qualification receipts out of the enduring
use/reference explanation. No empty documentation skeleton counts as evidence.

## Bootstrap and dependency direction

```text
native tools, including retained C-language B seed B0
    -> B libraries and complete Level L built by B0
    -> proper cREXX B compiler B1, built by B0 using Level L-generated frontend
    -> rebuild B libraries, Level L and proper B using B1 -> B2
    -> repeat the same closure using B2 -> B3; compare B2/B3 stage products
    -> qualified B + Level L foundation
         -> new G compiler and G runtime/library products
         -> new C compiler and its B-only host services
```

The [architecture](doc/architecture.md#bootstrap-native-seed-to-self-hosted-b-and-l)
defines stage inputs, output comparisons and host versus target dependencies.
The existing Level L L0/L1/L2 reader bootstrap runs within the appropriate build
stage; its numbers are distinct from B1/B2/B3 compiler generations. Rebuilding L
with proper B establishes that the foundation no longer needs the native B
compiler. Repeating the same closure checks convergence; it is a fixed
qualification procedure, not a build loop that runs indefinitely.

All compiler infrastructure is Level B. Neither Level L, proper B nor the C
compiler host may require G. Existing full Classic runtime packaging currently
depends on G; this target-program dependency is recorded separately. Removing
that dependency requires a reviewed runtime/package split and is not established
merely by writing the C compiler in B. The C/G PoC may use the existing runtime
packages while the full closure is developed.

Test cold from pinned sources without installed new compiler products. Later
self-hosting stages must run with native B/C/G compiler services unavailable,
while retaining the required assembler/linker/VM tools. Keep exact stage import
roots and manifests, including libraries and generated grammar modules; renaming
a PATH entry alone does not prove absence of a linked native backend. Use
deterministic products, opt/no-opt controls and independent semantic regressions.
Convergent output demonstrates reproducibility, not compiler correctness by
itself. Routine incremental builds reuse the qualified foundation; they need not
repeat cold multi-generation qualification for unrelated edits.

## Numbered acceptance criteria

All implementation criteria below are **open**. Phase A evidence closes its
declared PoC portions only; Phase B–D obligations remain visible.

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
| **LLCOMP-STEP07B — deferred until after C completion** | Implement the proper B frontend/profile over the shared IR/backend; inventory full B coverage and the self-hosting source closure. Build B1 from the native seed, rebuild B/L/B to B2 and B3, and qualify convergence, semantic behaviour and independence from G/native compiler services. | AC01/02/04–10/12–15 | Shared backend and required typed/metadata coverage from STEP04/07; review the exact B coverage and stage manifest before implementation. Required before combined STEP09, not before the C-first milestone. |
| **LLCOMP-STEP08 — deferred INTERPRET** | Prove and implement the shared compile-service/Classic interpreted-group activation and continuation route, with bounded generated-code ownership. | AC05/09/11–13 | Adrian deferred this mechanism review and implementation; runtime integration follows the needed STEP07 activation/control contracts. **INTERPRET mechanism gate**, existing LC criteria and no-C constraint remain binding. |
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
The subsequent C-first decision below supersedes the earlier balanced C/G
expansion sequence. Classic coverage and STEP08 now lead; STEP07B is deferred.
Its full self-hosting obligations and the INTERPRET mechanism gate are retained.


## C-first delivery milestone

**Priority and plan approved, 16 October 2026.** Adrian chose to complete the
new Level C compiler before STEP07B, and explicitly retained RXAS quality and
efficiency as acceptance. Native C output may be faulty or inefficient. This
Adrian subsequently directed instruction-by-instruction review, starting with
SAY in the existing LC-I order, and leaving INTERPRET until later. This
supersedes the earlier parallel/early INTERPRET mechanism review; no INTERPRET
implementation or mechanism investigation is part of the current pass. This
section is the authoritative sequence within STEP07/08, not a second Classic
language specification. The [Classic worklist](../../../docs/planning/release-1/levelc-compatibility-worklist.md)
and its [reference obligations](../../../docs/planning/release-1/levelc-reference-obligations.md)
continue to own language conformance and approved exceptions.

### Vision and intended outcome

Deliver a complete, explicitly selected Level C compiler and reusable compile
service through Level L, with correct Classic behaviour, useful diagnostics and
economical, explainable RXAS. All new compiler algorithms remain Level B;
grammars and reusable services use Level L. The existing native B seed continues
to build this implementation. New source compilation never delegates to native
compiler services. Existing runtime algorithms are reused through checked ABI
contracts, rather than copied into a second Classic runtime.

Complete C includes the agreed instruction/BIF surface, variables and pools,
numeric/string/Unicode rules, calls, host services, conditions, TRACE and
INTERPRET. Existing native support alone is neither the complete scope nor its
proof: its missing behaviour remains visible and its defects must not become
the new specification. A parked item is still open unless Adrian has explicitly
approved an exclusion. Constrained-host and platform applicability must be
recorded and qualified; desktop-only proof does not close them.

G retains its currently qualified behaviour. Shared changes run appropriate G
regressions and independent non-C/G Level L tests; they do not require a new G
feature in every C increment. Proper B and further G expansion resume after C
completion. This changes scheduling, not LLCOMP-AC02/10 or the final B/L build
order. The whole-programme AC01–16 and STEP09/10 stay open until their full
scope passes. A later default switch still needs its own approval and rollback.

### Checkable acceptance criteria

All **CFC-AC01–08 are open**. STEP07A supplies retained foundation evidence.
Instruction reviews advance this crosswalk incrementally; a reviewed subset or
a deferred shared dependency does not establish whole-instruction completion.
INTERPRET remains an eventual requirement, with its work explicitly deferred.

1. **CFC-AC01 — complete scope crosswalk.** Map every applicable Classic
   instruction, BIF, LC reference obligation and approved extension to new
   grammar/AST/IR/runtime paths, positive/error tests and qualification evidence.
   Record current support, missing implementation, missing proof, approved
   exclusions and unresolved decisions separately. Reconcile upstream changes;
   no required row may disappear or inherit a native compiler's pass by name.
   Serves LLCOMP-AC03/08/10/15.
2. **CFC-AC02 — Classic semantics and independent expectations.** Complete
   variables/stems/compound names, DROP/EXPOSE, PARSE/ARG, expressions, numeric
   context, structured control, routine/label/call resolution and applicable
   BIF contracts. Tests state expected values, side effects, errors and order
   independently of native output, including boundaries and invalid inputs.
   Native discrepancies retain a minimal reproducer, modes, expected contract,
   classification and disposition; unresolved semantic ambiguity remains open.
   Serves LLCOMP-AC05/08/10.
3. **CFC-AC03 — activation, conditions and source behaviour.** Verify local
   labels, SIGNAL/CALL conditions, trap state, RETURN/EXIT, source identities,
   clause/TRACE observations and diagnostic origins through calls and failures.
   Include Unicode/CRLF, syntax and semantic messages/carets, generated source,
   optimised/unoptimised paths and preserved prior output on refusal.
   Serves LLCOMP-AC05/07/09/10.
4. **CFC-AC04 — host and delivery contracts.** Qualify the agreed ADDRESS and
   implicit-command, input/output/stream/queue, invocation, external-call and
   exposed host-state routes, including ownership, conditions, failure and
   resource limits. Preserve the approved static signed B/G call boundary and
   other exact scope decisions; new general loading/interoperation is a review
   gate. Missing platform facilities or host APIs remain named open rows.
   Serves LLCOMP-AC08/10/12.
5. **CFC-AC05 — INTERPRET.** Complete STEP08 against LC-I-25 / LC-87-01–05:
   live caller state, nested groups, local labels/routines, transfers and
   resumption, exact source/provenance, and bounded generated-code retention
   and reclamation. Work is deferred until after the current instruction review
   pass. When resumed, first retain a small mechanism proof and review its exact
   contract before integration. Unsupported native platform needs are blockers
   for decision, not permission to add C compiler/VM code or omit the feature.
   Serves LLCOMP-AC05/09/11–13.
6. **CFC-AC06 — inspected RXAS quality and efficiency.** Each substantive
   lowering/emission/optimisation increment retains readable full RXAS for
   representative simple, control, call and failure cases, alongside relevant
   before/after IR and structural assertions. Record IR/assembler/host modes;
   use assembler-no-opt controls to expose compiler work. Inspect executable
   instructions, repeated setup, calls and helper work, copies, branches,
   allocations, register/frame needs and cleanup separately from metadata.
   Preserve truthful logical filenames, source/TRACE and error observations.
   Classify excess as a direct defect, proven semantic/ABI need or a named
   later optimisation with evidence and a closure test. Repair direct defects
   in the owning layer; an eventual pass or a worse native result is no excuse.
   Neither raw line counts nor smaller code alone prove speed or correctness.
   Formal performance testing remains outside ordinary Level L research;
   later production budgets remain under LLCOMP-AC13. Serves AC06/07/09/16.
7. **CFC-AC07 — reusable Level L and B-only implementation.** New generic
   tree/IR/analysis/writer services have independent non-C/G tests, documented
   ownership/invariants and resource/failure checks. Language facts remain in
   compiler extensions. Source/dependency audits, both retained B-host modes,
   appropriate G regression checks and installed requests with native compiler
   services unavailable prove the boundary. Serves LLCOMP-AC01/04/06/07/15;
   does not close new-B self-hosting AC02.
8. **CFC-AC08 — complete C qualification and handover.** Close every required
   crosswalk row with exact-input evidence; verify installed CLI/service,
   supported VMs/platforms and constrained-host applicability, packaging,
   lifecycle and documented errors. Reuse valid evidence and keep local,
   hosted, sanitizer and release gates distinct. Architecture, algorithms,
   user guide and executable source/IR/RXAS examples agree. Direct quality
   defects and required host/INTERPRET gaps prevent completion. Record retained
   optimisation limitations without claiming optimal code or full B/G
   replacement. Serves the C portions of LLCOMP-AC01–16; approval of routing
   changes remains separate.

### Numbered delivery sequence

These steps preserve the existing top-level IDs. Each implementation slice
must record its exact API/tree/IR decisions and worked example before edits;
already approved semantics need no repeat permission. New language decisions,
architecture shifts and public API contracts retain their existing gates.

| Step and current state | Work and acceptance | Dependencies / decision gate |
|---|---|---|
| **CFC-STEP01 — in progress** | Review each instruction in LC-I order, starting with SAY; incrementally reconcile the complete C crosswalk, pin independent expectations and inspect representative RXAS. CFC-AC01/02/06/08. | STEP07A foundation. Reuse applicable evidence; identify shared expression/BIF/host gaps explicitly. Finish the current instruction's review and disposition before starting the next. |
| **CFC-STEP02 — deferred by Adrian** | Later prove the STEP08 activation/control/source/lifetime route on a small INTERPRET example and nested/error case. CFC-AC03/05/07. | Resume after the current instruction review pass. Exact integration/API approval remains required. No early INTERPRET mechanism work. |
| **CFC-STEP03 — open** | Complete variable/pool/stem/exposure, PARSE/ARG, expressions/numeric state, structured control and routine/call families, reusing runtime services and improving Level L where reusable. CFC-AC02/06/07. | STEP01 instruction review; implement each agreed instruction and its necessary shared services, retaining errors, actual RXAS and structural checks in every slice. |
| **CFC-STEP04 — open** | Complete labels, conditions/SIGNAL/TRACE, diagnostic provenance and agreed host/I/O/stream/queue/call contracts. CFC-AC03/04/06/07. | Needed activation/pool contracts from STEP03; exact host gaps remain visible instead of emulated silently. |
| **CFC-STEP05 — open** | Implement and integrate STEP08 INTERPRET with nested activation, transfer/resumption and bounded code lifetime. CFC-AC05/06/07. | Accepted STEP02 mechanism plus required STEP03/04 semantics. No fallback to native source compilation. |
| **CFC-STEP06 — open** | Close remaining crosswalk rows and the complete C delivery evidence, documentation and emitted-code review. CFC-AC01–08. | Required STEP03–05 coverage; focused evidence first, remaining full/platform gates once on final inputs. Separate C milestone; no STEP07B prerequisite or automatic default switch. |
| **CFC-STEP07 — deferred** | Resume STEP07B and remaining G coverage, then combined STEP09/10 qualification and routing review. | Complete C milestone; retain all existing B/G/self-hosting criteria and decision gates. |

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

1. **CFC-SAY-AC01 (reviewed):** classify both SAY forms, expression/order/error,
   Unicode/host and source/TRACE obligations, with implementation, evidence and

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
| C-first delivery / STEP07 and STEP08 | Priority approved 16 October; CFC-AC01–08 and CFC-STEP01–07 recorded above. Complete C leads; current G coverage maintained. Instruction-by-instruction review is active, starting with SAY. INTERPRET and its mechanism review are deferred; no new C completion claimed. |
| STEP07B / proper B and further G expansion | Deferred until after the C-first milestone. AC02/self-hosting and remaining G criteria stay open and retain their IDs. |
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
