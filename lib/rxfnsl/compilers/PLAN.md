# cREXX compilers built with Level L

Status: STEP01 approved; STEP02 and STEP03 published. STEP04 C/G IR-to-RXAS
compilation published as `bf74c72111fbd466d1f93c421e6d5dadbbedcc2c`.
Its local functional qualification is retained; the 15 October emission review
opens explicit code-quality acceptance and reopens source-metadata fidelity.
See [the review and repair boundaries](doc/EMISSION-QUALITY-REVIEW.md).
The approved essential metadata/condition-provenance correction is implemented
and locally qualified; [receipt](evidence/provenance-20261015/README.md).
Adrian authorised its develop publication and CI-monitor handover on 15 October
2026. The known STEP04 package dependency failure remains open separately.
Wider quality, clause/TRACE and redesign gates remain open.
STEP04 qualification baseline:
`bcfb068b7abe2f3a2b9a23232fdb4244cc8ffa67`. Original planning source baseline:
`47ee4e40ccffcb4c3942f477c0820f8e3c02cbd4`.

Start with the [STEP01 review](doc/STEP01-REVIEW.md), then the
[architecture](doc/architecture.md), [C/G scope](doc/poc-scope.md) and
[Level L IR design](../doc/ir-design.md). The acceptance criteria below remain
open until implementation evidence satisfies their stated scope.

## Vision and intended outcome

Build new Level C and Level G compilers in cREXX using Level L. Each frontend
recognises its own language and preserves its semantics, then produces a common
intermediate representation (IR). Shared analyses and optimisations transform
that IR before a shared backend emits RXAS. The existing assembler, linker and
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
G and C. The proposed staging below implements that direction. C's compiler
host should depend on B and Level L, independently of G.

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
The [Level L programme plan](../../../research/level-l/PLAN.md) delegates this
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

### Proposed directory and product layout

```text
lib/rxfnsl/
  rexx/                     Level L IR/schema/storage/views and generic services
  compilers/
    PLAN.md                 this plan and acceptance record
    common/                 B/C/G operation/type extensions, ABI and specific passes
    levelb/                 proper B frontend/profile and self-hosting checks
    levelc/                 Classic grammar, checks and semantic lowering
    levelg/                 G grammar, checks and semantic lowering
    rxas/                   cREXX instruction selection, registers and emission
    driver/                 reusable compile service and distinct tool entries
    examples/               paired C/G programmes and readable stage dumps
    tests/                  independent controls and permanent regressions
    doc/                    compiler usage, IR reference and migration matrix
```

The subtree contains the plan, review documents, source controls, C/G
semantic families, frontends, backend and regression consumers. The source
compilers use distinct build targets and installed commands `llrxc-c` and
`llrxc-g`. `llrxc-b` remains proposed for the later proper B compiler, with a
private library namespace. Ordinary `rxc`, `rxc-c` and wrapper routing continue
to select existing products until the replacement gate. No prototype silently
falls back to an old compiler for unsupported syntax. Product names and exact
API spellings are reviewed in STEP01.

Generated artefacts stay in build directories. Grammar files retain the normal
Level L source conventions. Do not place compiler-specific declarations in the
basic grammar surface unless their general usefulness has been demonstrated.

## Architecture and semantic contracts

```text
Level C source -> C grammar -> Classic AST -> Classic semantic lowering --+
                                                                       |
Level G source -> G grammar -> typed G AST -> G semantic lowering --------+-> common IR
                                                                           |
                                                       validate -> analyse -> optimise
                                                                           |
                                          cREXX RXAS selection/registers/emission
                                                                           |
                                                       existing rxas -> rxlink -> rxvm
```

RXAS remains the low-level IR and interchange boundary. The new common IR
retains values, types, control structure, effects and source identity before
register choices and instruction sequences obscure them. Initial optimisation
work happens here; existing assembler optimisation is a separate stage and is
disabled in controls used to attribute a transformation to the new compiler.

### Ownership: Level L IR, compiler-defined language semantics

Level L owns the extensible IR framework and its format. The compiler supplies
the complete meaning of its language operations through that framework. The
new C/G compilers are consumers and extension providers; they do not own a
private IR container that other languages must adopt cREXX semantics to use.

| Owner | Responsibilities |
|---|---|
| Level L | Modules, functions, blocks, values, edges, schema registration, numeric storage, handles, sparse properties, source origins, validation, traversal, rewriting, facts and text/binary views. |
| Level L common operation contracts | Precisely specified generic control/value operations and interfaces for effects, type checking, constant evaluation and lowering. Generic analyses and transformations use these contracts. |
| C/G compiler libraries | Source-language rules, type and operation extensions, numeric/conversion behaviour, dynamic pools, alias/ownership semantics, errors, tracing, calling conventions and language-specific lowerings. Shared cREXX definitions live in `compilers/common`; language-specific definitions live with their frontend. |
| RXAS backend | Supported target operations, instruction selection, calling/register rules, literals and RXAS metadata/output. Other consumers may provide different targets. |

Level L is more than a byte container: a branch must identify its successors,
values must have defined uses, and a generic transformation must have explicit
semantic premises. A standard operation has one documented meaning. A compiler
selects it only when that meaning fits; otherwise it defines an extension or
lowers its operation to a suitable combination. An extension must not silently
redefine the meaning of a standard opcode.

For example, C and G additions can share the same numeric record structure
while using different registered operation families. Their descriptors and
cREXX semantic services specify exact operand/result types, conversions,
numeric-context dependencies, overflow/failure behaviour and constant
evaluation. Level L's common folding pass requests evaluation from that
contract. The result is a checked constant, an explicit refusal to fold, or a
diagnostic; the pass does not guess Rexx arithmetic. Similarly, converting a
source-language truth value to the Boolean input of a common branch belongs
to the language mapper.

Register namespaced, versioned operation/type families through the Level L
schema API. Derive numeric codes, readable names and structural checks from
that registration. Additional semantic checks/evaluators/lowerings are cREXX
services registered explicitly by the host compiler, not executable code
loaded from an IR file. An installed schema supplies the interpretation of a
saved module's codes.

An unfamiliar operation may be preserved in an explicitly opaque tooling view,
but successful compilation requires its registered verifier and an admitted
lowering/backend route. Missing effect information grants no permission to
remove, reorder, duplicate or fold an operation. A control-affecting extension
must provide the required edges and continuation contract before a flow
analysis can claim complete coverage. Report an unsupported analysis explicitly
when that contract is missing.

State proof boundaries in the architecture: Level L checks graph/schema and
transformation obligations; correctness of a language-specific operation or
summary is established by its compiler contract, tests and any retained proof.
Generic proofs are conditional on those contracts, not proofs inferred from
arbitrary extension names or annotations.

Verify this ownership with an independent small operation-family fixture that
uses the IR, codecs and a generic pass without importing C/G compiler modules.
Keep the numeric storage, format and generic API reference in Level L's own
documentation. Compiler architecture pages link to it and document only their
operation families, language mappings and backend contracts. Maintain both
layers alongside implementation under AC15.

### Shared representation, explicit meanings

The Level L core supplies owned modules/functions, typed values, constants,
operations, blocks, branch edges, calls and returns. Value uses and branch
targets are checked references: an IR is not forced into an AST's single-parent
tree shape. Loops are graph cycles; an operation may use a value also used by
other operations. Preserve the source tree and its origin links independently.

STEP02 implements explicit typed slots and block-local immutable temporary
values. Values crossing a join use stores and loads; the verifier rejects
cross-block temporary references. Full SSA, with its additional join and
dominance checks, remains a later profile. See the
[IR reference](../doc/ir-design.md). Do not implement sharing by attaching the
same owned AST child twice.

Operations have defined operand/result types, evaluation order, state effects,
failure behaviour and control outcomes. Ownership, reference escape and cleanup
also become explicit as references and classes enter the admitted language.
Unknown calls/effects block transformations that require stronger knowledge.
An effects marker alone cannot supply the edges needed to analyse a nonlocal
transfer or resumable handler.

| Source distinction | Required representation or lowering |
|---|---|
| G typed arithmetic; Classic contextual arithmetic | Distinct operation semantics or runtime calls with explicit numeric context; never one unqualified add with assumed shared rules. |
| G named storage; Classic pools, stems and exposure | Typed bindings versus explicit dynamic pool operations and alias effects. |
| Classic eager logical operands; G short-circuit forms | Preserve both operand evaluations for Classic and conditional evaluation for the admitted G forms. |
| Literal values and source spelling | Preserve spelling and conversion rules where observable; typed constants retain exact representations. |
| Typed calls; Classic argument presence, RESULT and activation | Explicit signatures and calling conventions, including required runtime state. |
| Ordinary branches; Classic labels and condition handlers | Ordinary graph edges where sufficient; explicit exceptional/nonlocal/resumable contracts before admitting the latter. |
| Source tracing and diagnostics | Retained authored-operation anchors and ordered trace events through optimisation and emission. |

The common IR can carry named operation families such as `crexx.g.iadd` and
`crexx.c.add`. These registered operation names appear in readable dumps.
They do not introduce source-language syntax. Classic and G mappers share infrastructure without erasing semantics.
Lower specialised operations into supported core operations/runtime calls
through checked passes. Verify every stage's admitted operation set.

### Numeric storage and readable views

Adrian proposed compact numeric IR storage with human, LLM and programmatic
views. The recommendation is a checked, indexed binary representation behind
named operations and fields. STEP02 implements this foundation. Define
the IR's meaning independently of its physical layout so storage improvements
do not change compiler semantics or the authoring interface.

Use three layers:

1. **Semantic API:** checked builders, handles, readers and rewrite services.
   Callers use operation names, typed operands and named fields; they do not
   calculate byte offsets or write opcode numbers themselves. Resolve names to
   codes once when registering/loading a schema; transformation loops use the
   resolved descriptors and handles.
2. **Numeric working storage:** integer operation/type/block/value IDs, compact
   operation records and separate operand/result/edge ranges. A storage owner
   holds these buffers, together with constant, symbol and source-origin tables.
   IDs refer to records; they are not memory addresses or RXAS registers.
3. **Views and interchange:** deterministic named text dumps for reading and
   review; structured records for tools and LLM consumers; a checked binary
   encoding for persistence. In-process algorithms use the semantic API directly
   over numeric storage, without text conversion or constructing a second tree.

For example, the worked SAY example includes:

```text
%3 = crexx.g.iadd %1 %2 @origin:1
```

With that example's registration order, the third operation has opcode 17,
block 1, operand start/count 1/2, result start/count 3/1 and origin 1. The values
table identifies result 3 as defined by operation 3 with type 3 (`crexx.g.int64`).
The two operand records contain value IDs 1 and 2, whose defining constants
contain the integer values 2 and 3. The full fixed-width layout is in the
[IR reference](../doc/ir-design.md#saved-forms-and-numeric-storage).

The verifier checks operand/result types and counts, definition/use rules,
block membership and source references. A compact record cannot replace those
semantic checks. Variable result ranges allow operations with more than one
result where the selected IR model needs them.

Keep strings, source bytes and exact constants in typed pools, referenced by
IDs. Intern repeated immutable names/types where equality permits it; preserve
source occurrence and spelling separately. Do not force decimal, binary,
Unicode or Classic literal representations through a machine integer or float.
Use explicit lengths for arbitrary bytes and text containing zero. Store common
fields in predictable records; keep optional analysis facts and uncommon
properties in side tables. Defaults remain schema rules, and presence remains
observable where an absent value differs from an explicit default. In
particular, absent analysis knowledge is not a false condition or a proved fact.

The existing Level B [binary-memory operations](../../../docs/books/crexx_language_reference/binary_memory.md)
provide canonical little-endian fixed-width fields as well as separate
host-native packed access. The IR uses the portable fixed-width fields in
`.binary` buffers. Ordinary `.int[]` values are not automatically packed; a
numeric-looking array alone does not establish compact storage. The new arena
uses these existing Level B facilities and borrowed references, with no new C
helper or Level G dependency. Retain existing AST storage and APIs
unless a separate, measured migration is agreed.

STEP02 compared fixed-width operation records with parallel packed columns
and retained row records for construction. Keep variable operand/edge lists in separate
appendable storage and use indexed lookup. Measure scan-heavy and rewrite-heavy
passes before selecting a layout. Avoid variable-length integer compression or
bit packing in the first working arena: its decoding and mutation costs need
evidence before trading away direct indexing. Compaction may rebuild a completed
arena with an explicit ID map; ordinary append/growth must preserve live IDs.
Deleted records and stale/foreign handles must fail checked access. Bind facts
and analysis results to an arena revision and invalidate them after relevant
rewrites. Storage chunks and numeric widths are implementation choices, not
arbitrary user-facing grammar limits; reject unrepresentable extents explicitly.

A portable saved image has an explicit version, schema/opcode identity, widths,
byte order, counts, ranges and exact constant encodings. Never persist raw
pointers or treat a host-native packed buffer as a portable format. Validate
all extents, overflow, IDs and admitted codes under resource limits before
exposing a loaded image. Resolve file-local IDs to the loaded arena; do not
persist live borrowed handles. Publish a decoded or transformed arena only
after validation, preserving the prior input on failure.

Provide a deterministic lossless textual interchange form alongside convenient
filtered views (one function, block, source span or analysis result). The
lossless text and binary forms must round-trip the semantic module, including
observable constant spelling and origins; a filtered display is explicitly a
view, not a reloadable module. Tests cover both round trips, invalid/truncated
images, wrong schemas, stale references, sparse defaults and failure atomicity.
Do not execute operations merely because they appear in a decoded image.

Measure total arena storage, auxiliary indexes/facts, copying, allocations,
construction, traversal and rewrite costs on the same C/G inputs. Document the
storage formula and measured overhead separately. A smaller byte count does
not by itself prove a faster compiler. Keep the selected encoding, ID lifetime
rules, codecs and comparison evidence in `trees-and-ir.md` and `algorithms.md`.

### First shared optimisations

Start with constant evaluation for explicitly supported operations, propagation
of compiler temporary values, branch simplification and elimination of unused
operations proved safe to discard. At least one shared pass must make a useful
change to each language; two independent optimisers behind one driver do not
satisfy the PoC.

Preserve evaluation that can change state, raise an observable error, diverge,
trace or transfer control. A known condition outcome permits edge pruning; it
does not by itself permit deleting the condition's evaluation. Classic pool
access is not an ordinary immutable local, and an unassigned Classic name is
not automatically a source error. Numeric context must be established before
context-dependent folding. Dynamic INTERPRET invalidates dependent state facts
unless a justified narrower summary is available.

For each pass record its preconditions, postconditions, termination argument
or work bound, and an equivalence argument for its admitted operation profile.
Schema checks establish shape, not arbitrary semantic equivalence. Check each
candidate result before publishing it; failed passes preserve the input and
return a located diagnostic rather than a partial successful artefact.

### RXAS backend and emitter

Implement both in cREXX. The general text emitter supplies chunk/rope-backed
construction, indentation, appending completed sections, streaming/file sinks,
source-origin mappings and explicit resource/error results. The RXAS adapter
owns labels, exact literal encoding, instruction operands and existing metadata.

Begin with a simple correct register assignment for the admitted scalar/call
profile. Register reuse follows demonstrated liveness, call-window and cleanup
contracts. No C allocator or emitter is used as a temporary product backend.
Validate numeric round trips, Unicode/binary quoting, ABI signatures, source
metadata and linked execution against the existing assembler/VM contracts.
Emitting RXBIN directly is outside this plan; RXAS is the deliberate boundary.

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

Also build the proper B frontend/profile and qualify its self-hosting closure
under STEP07B. B and G may share typed syntax and semantic code where their
contracts agree; the B build must not require generated G services or the G
compiler. The C/G-first development sequence exercises the common IR early.
The final product build order begins with B and L, as specified below.

For G this includes classes/interfaces, references and ownership, arrays,
numeric modes, imports/providers, initialisers, task/parallel and extension
contracts where supported. For C it includes the whole-instruction/BIF matrix,
PARSE, stems/exposure, numeric state, calls/arguments, labels/SIGNAL/conditions,
TRACE, ADDRESS, Unicode, streams and host contracts. Reuse the shared runtime
algorithms through explicit ABI calls rather than implementing a second set of
Classic BIF semantics in the compiler. Existing compiler bugs are recorded and
resolved against the language contract, not copied merely to match output.

### Phase C: reusable compilation and INTERPRET

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
| **LLCOMP-STEP06** | Qualify and document the two-frontend PoC: bootstrap, installed tools, both VMs/modes, failure cases, resource measurements and semantic comparisons. | Phase A portions of AC01–09/12–14 | STEP05. **PoC review gate:** assess usefulness and complexity; approve the next coverage increments. This does not authorise production replacement. |
| **LLCOMP-STEP07** | Expand C/G coverage together through named increments: imports, classes/references and ownership, dynamic Classic constructs, runtime/host contracts, tooling and extensions. | AC03–10/12–14 | STEP06 review; keep the complete coverage matrix current and generic services independent of either language. |
| **LLCOMP-STEP07B** | Implement the proper B frontend/profile over the shared IR/backend; inventory full B coverage and the self-hosting source closure. Build B1 from the native seed, rebuild B/L/B to B2 and B3, and qualify convergence, semantic behaviour and independence from G/native compiler services. | AC01/02/04–10/12–15 | Shared backend and required typed/metadata coverage from STEP04/07; review the exact B coverage and stage manifest before implementation. Can advance alongside STEP07 once those inputs exist; required before STEP09. |
| **LLCOMP-STEP08** | Prove and implement the shared compile-service/Classic interpreted-group activation and continuation route, with bounded generated-code ownership. | AC05/09/11–13 | Service foundations from STEP04; required semantic coverage from STEP07. **INTERPRET mechanism gate** before runtime integration; existing LC criteria and no-C constraint remain binding. |
| **LLCOMP-STEP09** | Close full B/C/G replacement coverage, installed/platform qualification, constrained-host/bootstrap requirements and representative performance budgets. | AC01–16 | STEP07/07B/08; reuse unchanged evidence and keep release/sanitizer gates distinct from ordinary develop publication. |
| **LLCOMP-STEP10** | Switch default B/C/G routes, retain the explicit native B seed, and retire superseded production compiler paths/documentation. | AC01/02/10/12/14 | STEP09 plus **Adrian's explicit replacement approval** and a verified rollback procedure. |

**AC15 and AC16 apply throughout the programme.** STEP01 presents the architecture and purposeful
tree/graph designs for review. STEP02 documents storage, ownership, schemas and
verifiers; STEP03 documents each frontend and language mapping; STEP04 documents
emission, registers and ABI; STEP05 documents analyses and transformation
arguments. STEP06 verifies the complete walkthrough. STEP07–10 update these
accounts in the same increment as every new capability or replacement change.

Adrian approved the [STEP01 review](doc/STEP01-REVIEW.md) and authorised STEP02
on 15 October 2026: Level B-only IR foundations and semantic extension API,
regressions, architecture documentation and user guide. STEP03–06 retain their
subsequent implementation and named pass/PoC gates.
STEP07–10 preserve the full destination and later decisions; bounded approval
does not remove those outcomes.

## STEP02 implementation record

Authorised 15 October 2026; baseline `d13a91c1b`. The outcome is a reusable
Level B-only IR library with checked semantic extension services, usable by a
small independent language consumer before the C/G frontends exist. Existing
AST consumers keep their APIs. STEP03 frontends, STEP04 target backend and
STEP05 production optimisation passes remain separate deliveries.

Acceptance for this increment (IR2-AC01–07 verified by the
[STEP02 receipt](evidence/step02-20261015/README.md)):

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
The [qualification receipt](evidence/step02-20261015/README.md) records the product
build, 16 new and 113 existing CTests, both seed-built B-only bootstrap modes,
eight whole-no-opt consumer executions, installed-wrapper example and storage
comparison. Source/tool hashes identify the qualified working-tree inputs.

SAY was explicitly added at Adrian's request: G and C registrations declare
observable output, the calculator-like example prints `5`, and folding preserves
SAY. The compiler families live in `compilers/common/llcrexxir.crexx`; generic
services remain independent in `rexx/llir.crexx` and `rexx/llirservices.crexx`.

The initial lowering coordinator admits pure straight-line fragments. The
stateful/control mappings and complete C/G semantics required by the eventual
backend remain open under STEP03–05. This foundation does not close the overall
compiler acceptance criteria, imply proper-B self-hosting or replace an existing
compiler. Adrian authorised committing and publishing this increment to
`develop` on 15 October 2026, together with the existing baseline. The required
local gates passed and their input hashes were rechecked before publication.
Normal Build CREXX and CodeQL workflows are expected to pass; their observed
results belong to GitHub and the lightweight monitor, not a status-only commit.
No hosted CI pass is claimed by this local qualification record.

## STEP03 implementation

**Approved 15 October 2026; F3-STEP01–04 complete and F3-AC01–05 verified locally.** The outcome is two real
source-to-IR frontends under `compilers/levelc` and `compilers/levelg`, written
in Level B with Level L grammars. Each keeps its own source semantics and
produces a checked module in the shared IR. This implements the frontend part
of the agreed [POC-01–10 scope](doc/poc-scope.md), without narrowing its scalar,
control, routine, numeric-context or library-call requirements. Executable
RXAS delivery remains STEP04 and production optimisation passes remain STEP05.

The basic source surface is existing C/G syntax. For a small illustration:

```rexx
/* Classic */
options levelc
total = 2 + 3
if total > 4 then say total
else say 0
```

```rexx
/* Typed G */
options levelg
total = .int
total = 2 + 3
if total > 4 then say total
else say 0
```

Both have expected output `5` when the executable pipeline is complete. STEP03
produces their source trees, semantic tables and verified IR. G binds `total`
to an explicitly initialised typed slot and selects typed integer operations.
C retains the dynamic pool name `TOTAL`, exact literal spelling and explicit
numeric context. Both use core branch blocks and their language's SAY contract.
The [paired walkthrough](doc/pipeline-walkthrough.md) shows the larger example
with a routine, loop and actual library call. Its source controls run
with existing tools; its source trees and frontend IR are now generated outputs.

Acceptance for this increment (**verified at the source-to-IR boundary**,
with the [STEP03 receipt](evidence/step03-20261015/README.md)):

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

The [STEP03 receipt](evidence/step03-20261015/README.md) records the warning-free
product build, 24 new and 128 existing distinct CTests, both seed-built B-only
bootstrap modes, twelve complete no-opt consumer executions and generated
C/G tree/IR/error examples. The syntax group checks categories, messages and
exact source coordinates; failed requests expose no successful IR. A focused
NUMERIC FORM comment regression and readable named IR attributes are included.
The architecture, schema/algorithm reference, frontend API and user guide now
describe the implementation. Runtime execution equivalence remains STEP04–06.

No new source syntax, native C compiler logic, default-tool replacement or
INTERPRET execution is part of this approval. Any newly required public contract
or architectural change returns for review before implementation. Completing a
small arithmetic example alone does not close F3-AC01–05 or STEP03.

## STEP03 publication and CI preparation repair

Approved 15 October 2026. Publish the locally qualified STEP03 frontends with a
narrow repair to the STEP02 test preparation dependency graph. A clean hosted
optimizer-parity build must produce the linked test images before CTest runs;
normal product builds must remain valid. Compiler semantics and source syntax
are unchanged by this repair. Preserve unrelated checkout files.

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

Current status: QA3-STEP01/02 and QA3-AC01/02 verified by the
[repair receipt](evidence/ci-preparation-20261015/README.md): all six absent test
images rebuilt through CI preparation, 32/32 focused tests and the product
build passed. Retained STEP03 sources/artifacts/examples match except the
qualified CMake preparation change. QA3-STEP03 / QA3-AC03 completed with repair
`980f0e2ebce9302840bace90dccac2e0a5aedc5d` and frontend publication
`bcfb068b7abe2f3a2b9a23232fdb4244cc8ffa67`; local/remote identity was rechecked
during STEP04. The existing monitor tracks that publication. Hosted results
remain distinct from local qualification; no status-only follow-up is planned.

## STEP04 executable backend

**Approved and published 15 October 2026; original F4-STEP01–04 implementation
and local execution evidence retained.** The subsequent emission-quality review
reopens source-metadata portions of F4-AC03/05 and adds open F4-AC07 / AC16.
The [STEP04 review](doc/STEP04-REVIEW.md) records the approved compile service,
RXAS backend structures, simple register assignment, Level L emitter use and
worked C/G example. Its F4 acceptance criteria and implementation steps refine
LLCOMP-STEP04 without narrowing POC-01–10 or the broader replacement criteria.
STEP04 completes the unoptimised executable path; shared optimisation remains
STEP05, complete PoC qualification STEP06, and INTERPRET STEP08.

The [emission review](doc/EMISSION-QUALITY-REVIEW.md) separates direct defects
from later optimisation and necessary language/ABI work. Its EQ-01–07 findings
and the acceptance/step record below are the current disposition. This is an
approved review and acceptance update, not authorisation to change the backend
or its public source-identity/activation contracts. All repairs remain Level B
and continue to use the Level L emitter libraries.

The B-only `llcompiler.request` service and separate installed `llrxc-c` /
`llrxc-g` commands now compile the admitted sources through shared checked IR
to RXAS using the existing Level L writer. The installed `llcompilers.rxbin`
package supports independent service consumers. Stable registers and private
call windows preserve typed values, Classic pools/arguments/numeric snapshots,
recursion, source locations and exact literals. The B-only target adapter uses
the existing Classic runtime, whose G prerequisites remain outside the host.

The [STEP04 receipt](evidence/step04-20261015/README.md) maps the six criteria to
product, 36 focused CTests, installed service/command checks, independent native
controls and the required isolated sanitizer scheduling measurement. Four new
aggregates each execute 31 programs with both assembler modes, 35 service
assertions and failure cases. The [actual examples](examples/step04/README.md),
[backend architecture](doc/backend.md) and user guide describe the result.

An independent native-control comparison exposed an existing optimized-assembly
discrepancy; the receipt retains its failing evidence separately. New-backend
output passes the corresponding case in both assembler modes and VMs. No native
compiler/assembler repair is included. This STEP04 publication carries the
implementation, documentation and qualification evidence together. Normal
automatic hosted workflows are expected to pass; their observed results are
reported separately by the lightweight monitor. Full programme acceptance stays
open, including shared optimisation, proper B, INTERPRET and replacement.

### Emission-quality follow-through

**Vision:** preserve the working C/G path while making its source maps truthful
and its baseline output proportionate to the source. Establish structural
regressions now and use both frontends to drive later shared IR improvements.
The original functional receipt is evidence for execution, not a code-quality
pass. The review's small controls are not representative timing measurements.

1. **EQ-AC01 — Source identity and fidelity (open).** Recompiling the same
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
3. **EQ-AC03 — Proportionate baseline construction (open).** `SAY expression`
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
5. **EQ-AC05 — Durable qualification (open).** Add permanent structural and
   semantic regressions for repaired findings, both assembler modes and concrete
   VMs where affected, with source metadata retained. Keep examples, architecture
   and user guide current. Reuse unchanged qualification; do not claim runtime
   speed, full TRACE conformance or hosted success from these bounded checks.

| Step | Criteria / dependencies | State and owner |
|---|---|---|
| EQ-STEP01: inspect STEP04 RXAS, compare native B/G/C, retain evidence and classify findings | AC16; identifies EQ-AC01–05 closure evidence | Complete as a review; [report](doc/EMISSION-QUALITY-REVIEW.md). |
| EQ-STEP02: repair source identity/provenance/grouping, redundant source-state emission, unused default construction and needless frame/entry scaffolding | EQ-AC01–03/05; review source-identity and clause-mapping surface before implementation | CP essentials complete locally. Real clause grouping, unused defaults and frame/entry scaffolding remain open for later approval. |
| EQ-STEP03: review Classic activation/numeric/helper lowering with paired typed controls | EQ-AC04; inspect actual helper work, error/lifetime obligations and INTERPRET compatibility | Bounded condition-provenance correction completed below; broader numeric/activation work remains open. No automatic native-C/runtime rewrite. |
| EQ-STEP04: execute approved shared passes and subsequently review liveness/register reuse | EQ-AC04/05; LLCOMP-STEP05 pass gates and EQ-STEP02 baseline | Open; shared IR/backend. Simple stable allocation was approved, not claimed optimal. |
| EQ-STEP05: retain focused structural/semantic proof and regenerate the walkthrough with each repair | EQ-AC01–05; affected implementation and existing publication rules | Open; compiler regressions/docs. No broad QA merely for this documentation review. |

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

The [review design](doc/CONDITION-PROVENANCE-REVIEW.md) provides the exact surface,
mechanism and paired C/G error example. On 15 October 2026 Adrian authorised the
design/plan update, then explicitly narrowed its priority to essential
corrections rather than a wholesale redesign. Accordingly the proposal keeps
function-level Classic handlers and their necessary location state; new adapter
handler placement and a source-map companion are not part of this increment.
Adrian approved implementation of this bounded correction on 15 October 2026.
CP-STEP01–03 and CP-AC01–06 are verified for this bounded profile. The
[receipt](evidence/provenance-20261015/README.md) retains actual product,
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
adapter-boundary alternative are named further work in the review. EQ-AC03's
unused defaults/frame layout, EQ-AC04's numeric/helper costs, STEP05 shared
optimisation and STEP08/AC11 INTERPRET requirements retain their existing scope.

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
  measurements and any production optimisation verdict. Use the existing
  benchmark corpus where applicable; new performance tooling is Level B.
- Keep the Level L user guide an enduring use manual. Put plans, unresolved
  decisions and qualification receipts here or in linked planning/evidence
  records. New features enter the manual when implemented and verified.
- Update this plan after each approved increment: criterion/step status,
  evidence, remaining coverage, exact scope decisions and next gates. Read it
  together with AGENTS.md and the parent Level L plan after session takeover.

## Current acceptance record

| Item | State |
|---|---|
| Architecture direction: cREXX compilers, Level L-owned extensible numeric IR and parallel documentation | Agreed; detailed contracts presented in STEP01. |
| STEP01 | Reviewed and approved as the STEP02 foundation: scope, architecture, trees, IR/API, algorithms, dependency inventory and six source controls. [Review packet](doc/STEP01-REVIEW.md). |
| STEP02 / foundation implementation | IR2-AC01–07 verified locally; [receipt](evidence/step02-20261015/README.md). Approved develop publication includes baseline `d13a91c1b`; hosted outcomes reported separately. |
| STEP03 / frontend increment | F3-AC01–05 verified locally; [receipt](evidence/step03-20261015/README.md). Published as `bcfb068b7abe2f3a2b9a23232fdb4244cc8ffa67` with the CI preparation repair. Hosted results are separate. |
| STEP04 / executable backend | Published `bf74c72111fbd466d1f93c421e6d5dadbbedcc2c`; original [functional receipt](evidence/step04-20261015/README.md) retained. Emission review reopens source-metadata portions of F4-AC03/05; new F4-AC07 and EQ-AC01–05 remain open. Hosted results are separate. |
| Essential metadata/condition correction | CP-AC01–06 and CP-STEP01–03 verified locally; [receipt](evidence/provenance-20261015/README.md). Approved for develop publication with the lightweight monitor following this commit. Existing diagnostic structure retained; original faults now survive the relay. Wider redesign and hosted outcomes remain separate; the known package dependency failure is still open. |
| STEP05–06 / remaining Phase A | Shared optimisation and complete PoC qualification remain open with their later gates. |
| STEP07–10, including STEP07B / full replacement and INTERPRET | Planned destination; proper B self-hosting is now explicit under AC02. Later gates remain open. |
| LLCOMP-AC01–16 | Open for their full compiler scope. STEP02 establishes the bounded IR foundation portions; AC16 now explicitly requires emitted-code quality. |
| Existing Level L AST services | Published separately; see the parent plan's AST7 evidence. |
| Existing production compilers and bootstrap | Existing routing retained. Seed-built Level L now includes the IR modules; proper B remains STEP07B. |

## References

- [Level L AST contracts](../../../research/level-l/AST-CONTRACT.md)
- [Level L user guide](../doc/user-guide.md)
- [cREXX architecture](../../../docs/ai-context/CREXX_ARCHITECTURE.md)
- [Level B implementation guide](../../../docs/ai-context/CREXX_LEVELB_AUTHORING.md)
- [Classic architecture](../../../compiler/docs/levelc_working_architecture.md)
- [Metadata conformance](../../../compiler/docs/metadata_conformance.md)
- [RXAS](../../../docs/ai-context/RXAS_ASSEMBLER.md),
  [RXLINK](../../../docs/ai-context/RXLINK_LINKER.md) and
  [RXVM](../../../docs/ai-context/RXVM_INTERPRETER.md)
