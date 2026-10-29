# Standalone RXAS inlining architecture

**Status: approved architecture; implementation in progress.** This document specifies
the target-level expansion service and its standalone proof of concept. It
distinguishes the current tools from the approved design and the optional
extensions requiring concrete owner contracts. It does not replace the native
AST inliner. The existing [toolchain plan](../PLAN.md) owns
acceptance criteria, numbered steps, decisions and approval, specifically
INL-AC01–08 and INL-STEP01–06 for the standalone outcome. This is its enduring
architecture account, not another plan. The complete outcome includes all
representable cases under the approved preservation/ABI contract and bounded
cleanup; the scalar/imported-helper examples below are intermediate slices.

The objective is one inliner written in Level B using Level L services, usable
by compilers, a standalone command and later assembler/linker drivers. It
expands executable RXAS bodies through a checked target representation. A
producer supplies ordinary call semantics and provenance, not a cREXX AST or
a second implementation of the function. Correct expansion must execute with
assembler optimization disabled. Cleanup can improve an expansion; it cannot
make an invalid expansion correct.

## Current capabilities and proposed owners

The current native compiler inlines cloned typed ASTs, including imported
`META_INLINE` templates. Those payloads are compiler transport, not executable
RXAS bodies and not machine-equivalence proofs. Native RXAS has procedure-local
CFG, component/value/storage, signal and observation analyses and transformations,
but no callee-body expansion service. `flushopt()` analyzes and emits the current
procedure; `rxasproc()` flushes it before opening the next one. RXLINK currently
selects modules and relocates references; it does not optimize or inline them.

The optional B/L `llrxas.program` owns one function's checked records, bounded
whole-cell assignment and rendering. It conservatively pins identity-sensitive
storage and contiguous call-window suffixes. It is not a module catalogue,
general machine CFG optimizer or binary instruction decoder. `llbinmeta` reads
selected 007 metadata; it is not an executable-body codec. `llirservices` supplies
atomic semantic-IR transformations, while `llirgraph` supplies owner/revision-
checked declared-CFG analysis. Neither presently supplies the full RXAS storage,
call-boundary or exceptional-flow proofs required here.

The proposed ownership is:

| Owner | Responsibility |
|---|---|
| Canonical binutils opcode/effect/signal/status owners | Existing ISA identities and operation semantics; extend extracted catalog coverage when necessary rather than copying numeric tables. |
| Optional L target owner | Checked modules, bodies, typed references, target CFG and storage observations, relocation, rendering and reusable expansion machinery. Reuse L-owned structures, revisions, diagnostics and atomic results. |
| `llinline` | B-written call normalization, legality proof, conservative cloning and glue, validation and deterministic expansion policy over that shared target model. |
| Research toolchain adapters | RXAS input/output, body acquisition, CLI orchestration and integration with existing compiler/library consumers. |
| Separate target cleanup owner | Bounded post-expansion simplification using the same canonical effects and checked observations; independent disablement proves expansion correctness. A general optimizer framework is outside the selected increment. |

Generic L defects belong to their existing owners and need independent
consumers. Target-specific machinery stays optional; core L must not depend
on this research tool. There is one expansion algorithm, regardless of which
driver requests it. A native oracle or VM may validate its output; neither is
a runtime backend for a B/L request.

## Standalone boundary and first usable delivery

Prefer an **RXAS-to-RXAS command** first. It parses a declared ordinary RXAS
profile through L source/lexer/parser services, obtains module/procedure records,
expands selected calls and writes ordinary RXAS. Keep one output module per
original module. Do not flatten all global banks into the caller or add public
exports to make an otherwise inaccessible global reachable.
That baseline uses current RXAS/current tools. An approved alias, initialization
or region extension needs versioned directives and matching assembler/linker/
VM support at the existing owners; unchanged native tools cannot consume new
semantics. This is narrower than completing the research replacements, but is
a real compatibility/delivery addition, not an invisible renderer convention.

Constructed checked-target consumers establish the model and service boundary
early. They are not by themselves the completed standalone proof of concept.
That deliverable also requires real input/output, an independent handwritten
or DSL caller, and an ordinary compiler/library caller using the same service.
A local scalar body and one exact-bound, global-free imported helper can prove
the useful boundary without finishing the new assembler, full 007 codec or
linker. Existing RXAS/VM tools can assemble/run the result as independent
oracles; the expansion request itself uses B/L algorithms only.
Already optimized installed modules are normally RXBIN artifacts, rather than
unoptimized compiler RXAS. For controlled proof-of-concept preparation, existing
`rxdas` can render their optimized instructions to RXAS before a request. The
B/L command/service does not invoke it or claim direct RXBIN input. Direct binary
consumption is the later shared-model adapter described below, with actual
executable decoding and relocation, not a metadata-only shortcut.

An initial parser may declare a bounded supported RXAS profile and reject input
outside it precisely. It must not silently drop metadata or claim full current
RXAS coverage. Subsequent profile growth serves the existing plan's broader
criteria. Instruction bodies are checked records/CFG nodes, never regular-
expression substitutions or concatenated instruction strings. The renderer is
the final syntax boundary, not the transformation representation.

A later optional RXBIN adapter decodes executable bodies into the same model
and encodes transformed output with canonical relocation. It needs its own
complete supported reference/record profile; selected metadata reading is not
enough. It does not become a prerequisite for the RXAS proof of concept. A
linker driver likewise uses the same expander after module selection, without
requiring users to run a separate pre-link command.

Proposed callable operations are conceptually `inspect`, `expand` and
`validate`; exact B signatures/package layout are a plan decision. An expansion
request contains frozen caller and body owners/revisions, a call identity,
exact target binding, admitted observation profile and explicit limits. It
returns either a validated new owner with all reference maps, an unchanged
owner plus a specific retained-call reason, or a diagnostic/limit/cancellation
result. It never partially edits its input. The first `llrxinline` client is a
build-tree bytecode CLI, not yet an installed command. The CLI and service share the same
policy and diagnostics; clients do not scrape command output.

## Target identities and proof inputs

### RXAS input/output profile

The first adapter increment reads one UTF-8 module with an explicit logical
module/artifact identity. Its generated L grammar owns tokenization and syntax;
the target draft owns declaration, forward-reference and operand validation.
It admits optional `.globals`, procedure definitions/imports/exports, global
exposure, canonical source-visible instructions with ordered typed operands,
labels, `.const`, callable/register/clear metadata, opaque known `META_INLINE`
string transport, `.srcstep` and `.traceevent`. Literal operands retain their
validated value and exact payload bytes. The adapter normalizes integer leading
plus signs and decimal suffix case to native RXAS spelling; importing a float
or decimal introduces no new host conversion or constant evaluator. Decoded
strings use the existing L UTF-8 validator, including embedded NUL; binary
literals retain arbitrary bytes. Jump tables and class/interface/task/
provider/initializer/autoload records remain explicit next-profile work until
their typed model contracts are implemented; unsupported forms fail precisely,
without dropping records or publishing a partial module.

Build preparation gives this full Unicode grammar the maintained Classic
grammar's finite `dfa.states=16384` allowance. Other construction budgets retain
their existing defaults. The canonical generation service owns its output-local
lexical cache, generator fingerprint, locking and cleanup entries. Exceeding the
declared cap remains a resource refusal with no partial grammar publication;
the adapter's runtime input policy remains separately bounded.

One CLI request selects one caller module and explicit immutable body inputs,
then publishes only the caller's complete output through the existing atomic
single-file writer. It does not concatenate modules or partially update a set
of files. Current RXAS has no serialized module-name directive: the service
retains its explicit logical identity, while native reassembly derives its
module filename/description from the input filename. Controlled fixtures use
stable logical filenames; this adapter makes no RXBIN module-name roundtrip
claim. External `rxdas` preparation is distinct from the B/L request. A native
compiler-generated no-optimization input and an independent handwritten client
must both exercise the actual reader; a handwritten approximation cannot stand
in for the generated input.

The shared target model must represent these domains explicitly:

| Domain | Required identity and mapping |
|---|---|
| Module/procedure | Stable logical module and executable procedure identity, selected provider/body binding, source identity and dependencies. |
| Instruction/control | Instruction IDs, block entries, labels, fallthroughs, direct/indirect targets, return sites and exceptional exits. Addresses are produced after transformation. |
| Storage | Owned base storage, active pointer binding, immutable argument-entry binding, slot/permutation identity and reference lifetime. These are distinct facts. |
| Registers/components | Local, argument and module-global bank references; integer, float, string, decimal, binary, attributes/object/reference and status observations according to canonical semantics. |
| Context | Numeric state, signal policy/handler stack, invocation/return contract, module initialization state and any observable physical-frame relation. |
| Records | Constants, tables, procedure references, metadata, source/TRACE register references, graph/index references and task bindings/seals where present. |

The callee's `r3`, `a3` and `g3` are not three spellings of the same virtual
register. `g3` is `(module, global slot 3)`; an argument has a borrowed entry
pointer; a local has owned base storage. Fresh integer register numbers do not
prove equivalent base storage or reference lifetime.

Track register-to-`StorageId` mappings separately from per-storage,
per-component `ValueId` and presence facts. A swap renames the storage reached
through a slot; a link aliases storage; a typed copy can preserve one component's
value identity between distinct storage cells. None of those facts alone proves
whole-value or lifetime equivalence. Integer, float, string, decimal, binary,
reference and native-payload components remain independent. Canonical
`ATTRIBUTES` denotes the status/type flags copied by `ACOPY`; attribute count
and addressed child storage have separate identities. Known absence is distinct
from unknown state or an unavailable proof.

Reused VM frames relink local pointers but ordinarily retain their previous
value contents; only the optional `SAFE_RECYCLED_STACKFRAMES` guard zeros them.
Fresh frames initialize values and `a0` is reset on every activation. Inline
storage at a caller site has a different reuse history from the callee's
per-procedure frame recycler. Prove that every observed component/status is
defined independently of incoming recycler contents, and that writes/cleanup
cannot observe hidden prior reference/native-payload state. A register-level
"written once" test is insufficient for partial component writes. History-
sensitive read-before-definition RXAS retains the call; blindly zeroing inline
locals changes its behavior. A future language guarantee can exclude such
programs from its producers, but target RXAS has no assumed zero-on-entry rule.

Input validation precedes candidate selection. Canonical operand signatures
classify explicit operands, but the proof must also enumerate implicit operands,
call windows, numeric register addressing, tables, metadata and observations.
A schema/effect change invalidates dependent summaries. A body hash can bind
the selected artifact; it cannot prove equivalence to a separately supplied
inline implementation. Use the actual selected executable body.

For every candidate, prove body availability, exact target, ABI legality,
observations and representability before applying profitability. Distinguish
`unavailable body`, `unresolved target`, `unproved frame/context`, `unrepresentable
foreign storage`, `resource limit` and `unprofitable`; these have different
remedies. An unsupported but representable shape is unfinished coverage, not
a fundamental impossibility.

## Call normalization and evaluation

Normalize a call site to ordered preparation operations, result binding,
argument count/presence, argument-entry pointers, target resolution and normal/
abrupt continuations. Preserve the original instruction's ordering, partial
effects and failure boundaries when normalizing fused forms. Source evaluation
has already happened in the producer's instruction stream: do not evaluate an
actual, receiver, index or locator a second time. Retain producer-emitted
conversions and defensive copies at their original boundaries.

| Existing form | Expansion obligation |
|---|---|
| `CALL_FUNC` | Zero-argument call with no result target; retain body effects and exits. |
| `CALL_REG_FUNC` | Zero-argument result call; preserve its explicit result clearing before target resolution/entry, including failed entry. |
| `CALL_REG_FUNC_REG` | Read count and contiguous window exactly as the VM does; preserve argument setup/status and count snapshot. Do not import zero-argument result clearing from another form. |
| `DCALL_REG_REG_REG` | Expand only when the effective target is proved stable and its resolution/failure behavior is preserved; otherwise retain dynamic dispatch. |
| `SWAPCALL`, `SETTPSWAPCALL`, `SETTPCALL` | Preserve status writes, pointer/identity swaps and call ordering. Defuse to equivalent checked operations or use a checked compound model. |
| `CALL1`–`CALL4` | Capture the named argument pointers before entry, preserve status, and use the same callee argument contract. These bytecode-only forms have no counted-window unwind permutation. |
| `SRCFPROCSEL` | Model factory selection and its possible call behavior, not just a pure pointer lookup. Retain unless the selected behavior and target are proved. |

This inventory comes from the current canonical ISA and handlers. The
implementation must mechanically classify every call-bearing opcode, including
future additions; an unclassified opcode is a validation/retention reason, not
permission to treat it as an ordinary three-operand call. Inner native/opaque
calls can remain ordinary calls inside a cloned body only when their effects,
parent-frame requirements, argument mapping and unwind behavior remain valid.
In particular, fused setup may swap the count register before the call reads
it; repeated operands can invalidate a pre-swap count fact. Canonical compound
effects and handler ordering, not mnemonic spelling, define the normalization.

Defaults and omissions use the executable prologue and status operations.
The VM borrows argument pointers; it does not automatically implement language
pass-by-value or synthesize `REGTP_VAL`/`REGTP_NOTSYM`. Status belongs to the
value, so aliases can observe it. Copying a value, copying one component,
copying attributes/status and linking storage are different operations.

## Argument, local and nested-window mapping

Start with fresh private owned local storage, fresh active argument slots and
untouched private **entry anchors**. For each incoming argument `i`, the anchor
`E_i` holds the actual entry pointer and active slot `A_i` initially links to
`E_i`. The anchor is not a value snapshot and must never be rebound, swapped or
used as a call-window scratch cell. The original by-value prologue remains
responsible for making the required private value snapshot.

Conservatively reserve one contiguous private local region corresponding to
the callee's complete original physical slot order: locals, global bindings,
`a0` and incoming arguments. Put untouched anchors outside it. This preserves
arbitrary admitted counted-window adjacency; later proofs may shrink it. Each
callee global needs a private active slot `G_i` initially linked through an
untouched anchor to its module's actual global storage, even for the same module.
Otherwise `LINK`/`SWAP`/`UNLINK g_i` would change the caller's active binding.
Direct use of caller `g_i` is valid only after proving that binding isolation
unobservable. Global storage and the frame's global pointer map are separate.

Map ordinary reads/writes of `a_i` to `A_i`. Map callee `UNLINK a_i` to
`LINK A_i,E_i`, because the real child's base argument is its borrowed entry
pointer. Plain `UNLINK A_i` would restore the fresh caller-owned local's base
storage and is wrong. Map `UNLINK g_i` to its original global anchor and map
callee local `UNLINK r_j` to its fresh owned base;
if an active local can be rebound/swapped, keep a distinct untouched base
anchor as necessary. Apply the same rule to every fused unlink operation.

For example, this is a storage-binding sketch, not a new RXAS macro syntax:

```text
entry:       E1 := entry pointer actual1; A1 := E1
callee LINK a1,other:  A1 := mapped active pointer other
callee UNLINK a1:      A1 := E1
```

Value replacement is not pointer replacement. If `actual1` aliases another
argument, preserve the alias unless the existing by-value contract requires a
snapshot. At source level `f(value x, expose x)` can require a snapshot of the
first occurrence even when that formal is read-only: a write through the second
occurrence before its read changes the result otherwise. Caller deadness and
callee read-only use alone do not prove copy removal. Include all alias paths,
nested/opaque effects, references, status and observations in that proof.

Transport simplification uses these separate rules:

- **By reference/EXPOSE:** preserve the required storage alias and its original
  lifetime. Do not add a value copy merely because an argument slot is mapped.
- **By value:** preserve the producer's snapshot until all writes to the actual
  through every alias/callback during the formal's live range are excluded,
  formal mutation cannot escape, identity/status/cleanup observers agree and
  result/argument overlap is safe. Then direct binding/propagation may replace
  that particular copy. Read-only formals alone do not establish this proof.
- **Owned move:** remove or redirect a transfer only when source ownership,
  destination aliasing and last-use/lifetime facts prove the exact move/reset
  behavior. A dead syntactic register is not proof that its storage is dead.
- **Status:** remove setup only when every observed status band already has
  the required entry value and canonical protected/private bits are preserved.
  Numeric component equality says nothing about argument presence flags.

Run these proofs over component, status, active/base binding, storage identity
and reference lifetime liveness. Whole-register value liveness alone is not
enough. Capture required by-value snapshots at the original marshalling point,
including repeated occurrences that already occupy a primary call slot.

After mapping the expanded body directly to actual registers or required
snapshots, eliminate the original caller's call-window setup swaps **and their
reverse restoration swaps** when that outer window is no longer needed. This
is storage/permutation proof across the entire preparation/body/continuation
region, not cancellation of adjacent `SWAP` pairs. Preserve once-only actual
evaluation/order, required snapshots, captured result and entry/base/slot
identities, status observations and restoration on every admitted abrupt exit.
Swaps needed by the actual body or retained nested calls remain; expansion does
not require a swap-free body or removal of every argument snapshot.

Preserve complete contiguous regions still needed by counted calls, not just
operands spelled explicitly. The count cell and argument cells must remain
ordered and disjoint from result/selector scratch when the original ABI requires it.
Argument slots can themselves be nested-call windows. Their entry/base bindings
must survive the canonical setup swaps, nested calls and normal reverse swaps.
`SWAP` permutes both active pointers and runtime slot identities; `LINK` and
`UNLINK` change bindings without permuting identity. Pointer equality cannot
substitute for slot identity when two slots alias the same value.

The real VM's cold unwind restores counted caller-window permutations when a
callee is discarded before reverse swaps run. Expanded code needs an explicit
proof of the equivalent permutation and cleanup on each such transfer. Merely
leaving the normal reverse swaps in place is insufficient. Initial expansion
uses no register coalescing; anchors, identity-sensitive cells and windows stay
pinned until independent liveness/storage proofs permit reuse.
Real frame activation also resets slot identities. A net or conditional inline
`SWAP` can otherwise leave a different identity permutation for the next loop
activation. Prove canonical permutation at every exit/reentry, emit the exact
inverse permutation, or use an approved active-region mechanism. Re-linking
pointers to entry anchors does not itself reset identities.

`a0` has distinct owned count storage, whole-cell zero/reset on each logical
activation before setting the captured count, matching real frame reuse; it
is not the caller's own `a0`. `LINKARG` indexes the physical frame's argument
bank, so copying it unchanged would read the wrong bank. A constant index is
rewritten to the corresponding mapped active argument slot. Constant/dynamic
selectors require a proved in-range domain, identical index arithmetic and
binding semantics. Current raw `LINKARG` directly indexes slots without a
bounds signal; do not invent `OUT_OF_RANGE` or array semantics for unproved
access. An unknown domain retains the call with a specific reason. No attribute-
array copy may replace storage aliases. Variable arity
can specialize a known finite call-site count, preserving omitted slots/status.
An unbounded count/index without a representable mapped-bank implementation
retains the call; it is not a reason to reject every varargs procedure.

Numeric local-address instructions and implicit `r0`/`r1`/`r2` operations need
equivalent rebasing or explicit-register forms. A computed slot index can be
rewritten only with a proved domain, bounds and pointer semantics. Metadata or
reflection that can observe arbitrary physical cells prevents ordinary renaming
unless its observations are also represented and proved.

### Liveness and allocation proof

Build normal, branch, table and admitted exceptional successors before solving
liveness. Compute backward CFG fixed points with canonical per-component
`use`/`def`/proven-kill sets; a possible write is not an unconditional kill.
Track status bands, active binding, base/entry binding, permutation identity
and reference lifetime as additional observation domains. Include TRACE/source
register observations, implicit numeric accesses, complete call windows,
cleanup/finalization and all escaping aliases. Unknown effects conservatively
observe/pin their declared domain; they do not make a private copy dead.

Use those facts and storage/alias flow to build interference constraints. Two
cells cannot share storage merely because their integer components do not
overlap: an attribute/reference/status/base identity may still be live. Pin
untouched anchors, borrowed/exposed/reference-targeted cells, fixed physical
observations and complete contiguous windows. A cell cannot be reused across
its required `ENDLIFE`, payload release or failure-visible update unless the
replacement preserves that event. Start with distinct fresh cells; enable only
proved private scalar coalescing first, with an independent checker reconstructing
interference, fixed/window and identity constraints from the transformed model.
This is the bounded reuse required by INL-AC05, not a new general allocator.

Packed RXBIN registers use `r=i`, `g=locals+i`, `a=locals+globals+i`. Decode
them to bank/index identities before changing counts. Increasing caller locals
requires remapping every existing global/argument operand and register-bearing
metadata in that procedure, not just cloned instructions. Implicit/index-coded
operands need the same canonical map. Relocation validates bounds after allocation.

## Returns, lifetime and control

Split the caller at the call, clone the body with fresh labels/IDs and route
each admitted return through its own transfer and cleanup to one continuation.
Keep call result binding stable across body writes/swaps; an aliasable result
must have the same entry-captured target pointer as the original call. Preserve
ignored-result effects and void-return behavior rather than inventing a value.

`RET_REG` is a whole-value move only when the VM's original local/base test
proves a true owned local; arguments, globals and linked/swapped nonlocal
storage are copied. The proof is at each return, not a classification of the
source register's spelling. Lower to the corresponding existing transfer;
`MOVE_REG_REG` exists, though its handler is marked deprecated, so its admitted
use must be explicit in the canonical target profile. Do not replace a move
with an arbitrary copy: identity/native-payload/ownership observations can
differ. A mixed binding requires a representable exact decision or retains
the call. Immediate integer/float returns write their components directly;
ordinary integer/float `LOAD` setters can clear other payload/reference state
and are not exact substitutes on an arbitrary result. A fresh temporary load
followed by the corresponding component copy can express that distinction.
String returns use `set_const_string` semantics. They are not unconditional
whole-value clear/copy operations. Preserve producer return conversions outside
them and the call form's own result clearing.

Fresh inline-owned storage has a logical callee lifetime despite residing in
the caller's physical frame. `ENDLIFE` can invalidate references to owned locals
and owned `a0`/nested attributes at the original logical exit, without clearing
ordinary contents. Incoming borrowed arguments and globals do not acquire that lifetime.
Returning a reference to an inline local must expire at the same boundary as
an ordinary call; returning a reference to caller storage must stay valid.
Restore active local slots to owned bases before lifetime cleanup so it cannot
invalidate borrowed storage hidden behind an active link. Preserve move/reset order and
native payload finalization obligations.

Loops, multiple blocks/returns, direct branches and tables are structurally
feasible through fresh CFG cloning. Remap every target, including table and
exception targets, then rebuild affected graph indexes. `EXIT`/terminal
operations remain terminal; they are not rewritten as local returns. Recursive
or mutually recursive bodies can be expanded a finite number of times, leaving
the remaining call; full recursion elimination is neither needed nor generally
possible. Indirect control needs an exact mapped target domain or retention.

## Context, abrupt exits and physical observations

An ordinary child inherits numeric state and signal policy; child mutations
vanish when it returns. Unknown numeric values can still be saved/restored with
the existing getters/setters if all admitted exits and setter ordering/failure
behavior are proved. An inherited unchanged context requires no redundant
setup. Do not copy `NUMSCI`/`NUMENG` into the caller and leave its state changed.
Plugin/context sharing and cleanup remain owned by the VM contract, not by a
new inliner runtime algorithm.

Signal policy requires more than adding normal-path `SIGPOP`s. `SIGPUSH`/
`SIGPOP` can save/restore known scoped policies, with all pushes before related
branch-handler installations and reverse pops on normal exits. But inherited
`SIGRET`, branch-style handlers, handler calls, CANCEL/unwind, pending delivery
and handler-stack markers can make the removed frame observable even if the
callee never installs a handler. Prove the exact possible responses and
continuations; remap logical return/branch cleanup and call-window restoration
for every admitted abrupt path. Unknown inherited return policy plus a raising
operation is not proved safe. The proposal assumes no generic VM instruction
that virtualizes arbitrary child-frame unwinding.

Ordinary source spans, clauses, source-step and TRACE operand locations must
remain truthful and use fresh remapped identities. Preserve call-site and body
origins; do not stamp the entire clone with the caller location. Composite
origins alone do not create an inline call stack. Raw signal objects and runtime
reflection may expose physical module/address/frame information beyond source
metadata. Retain calls when those exact observations cannot be reproduced by
the current representation.

`METALINKPREG` explicitly accesses the physical parent frame and
`METALOADCALLERADDR` exposes its caller address. Ordinary cloning changes those
relations, including when such an instruction is in a retained nested callee.
A known observation can sometimes be specialized to a mapped binding; arbitrary
physical-frame access, debugger stack/register traversal and opaque observers
need a real virtual-frame contract or the call. Do not invent that capability
or describe rejection of one observer as rejection of every method/reference/
condition. Legality is transitive through observed callees and callbacks.

### Approved observation policy

Exact physical diagnostics and useful frame-eliminating inlining cannot be
silently equated. The current diagnostic capture walks actual frames; an
asynchronous CANCEL/termination can expose a removed frame even in a scalar
body with no explicit observer. Under a strict preservation contract, retain
any site where that difference can be observed; this can prevent nearly all
useful expansion. The native AST inliner already removes physical frames, but
that precedent does not approve a new target contract.

**Approved proof-of-concept policy:** preserve
language-visible values/control/state/lifetime, authored source/TRACE mapping,
signal tag/payload and logical fault source, while allowing an optimized inline
call to be absent from the physical traceback. Explicit physical-frame,
raw-address/register-bank and raw physical-event observers still retain calls
transitively unless specifically specialized. This is an explicit observable-
behavior decision, not a proof that physical stacks remain equal. If strict
logical call chains are required instead, the inline-origin extension below
needs implementation before claiming that guarantee. Approval of the ordinary
inline policy does not establish virtual-frame support.

## Modules, globals and initialization

Same-module global references keep their original module/slot identity. Across
modules, preserve the callee module instance, global count, initializer records,
provider/autoload dependencies and callable contracts even if the direct call
disappears. Do not turn an initializer into an export or selection root.

Current RXAS `gN` addressing selects the executing procedure's module global
bank. There is no general import operand for another module's private global.
The model may retain `GlobalRef(callee-module,N)`, but rendering it as caller
`gN`, renumbering it into the caller bank, or synthesizing an `.expose` name is
not a faithful lowering. Already exposed name-sharing storage can be reused
only after proving the exact existing alias/owner/load-order relationship.
Private foreign-global access is a representation limit under unchanged
RXAS/ABI and retains the call until a binding mechanism is approved/implemented.
It is not permanently infeasible: the narrow extension below addresses it.
Global-free imported bodies remain useful and feasible under the other proofs.

Ordinary bytecode call ingress can initialize an unready callee module, detect
a call back into an `INITIALIZING` module, or fail on a poisoned module. Removing
that call must not bypass the state machine. Normal entry after all selected
modules are ready can provide a proof; the same body used during initializer
execution or late-load ingress may lack it. Retain the call at such ingress
unless the exact initialization action remains representable. A no-op helper
call is not a general substitute for initializing the original callee module
or detecting the same cycle. No new ensure-module instruction is assumed.
The existing helper compares the physical caller module and exempts same-module
entry. Foreign cloned code can therefore change initialization behavior of its
retained nested calls as well. An outer ensure alone does not fix that; preserve
the logical origin at each such ingress or retain initializer-reachable sites.
Absence of a target initializer is not proof that a late-loaded suffix is ready.

Module selection must therefore retain dependencies of expanded bodies,
including imported inner calls and provider metadata. Exact binding is required
before expansion: an unresolved, dynamically replaced or later autoloaded
provider cannot be treated as the body seen during an earlier build. Classic
source restrictions on B/G globals remain the producer's contract; target
expansion does not grant a new source-language access path.

## Extensions considered to simplify correct inlining

These are approved design directions, not current VM capabilities. Prefer narrow
extensions that preserve one existing runtime algorithm over frontend-specific
workarounds. Record and internally review concrete compatibility, source/wire/
schema and API/ISA contracts in INL-STEP01 before the corresponding edits;
optional mechanisms must be justified by the selected coverage requirement.

### Recommended private global binding

Add a typed, feature-gated **linker-private global binding**, separate from
`EXPOSE_REG`: an importing module's appended hidden global slot aliases an
exact selected provider module/global slot. Relocation maps both module identity
and slot; the binding retains the selected provider artifact and ultimate
storage owner. It creates no public name and never enters `exposed_reg_tree`.
An inline reference to provider `g3` can then render as the caller's hidden
alias `gH`, with ordinary instruction operands and existing frame setup.

Existing exposed globals show that the VM already supports `value*` sharing
and `globals_dont_free` for nonowners. Reuse that ownership mechanism, not its
public first-name-wins lookup policy. Allocate original owned/public globals
first across the selected modules, skip private alias slots, resolve the checked
alias graph to those owners, and only then prepare frames/initialize modules.
The current fill-all-unallocated-global loop must change deliberately; do not
allocate alias storage and silently overwrite/leak it. Importer aliases receive
`globals_dont_free=1`; the original owner remains responsible for teardown.

Validate exact provider/binding freshness, global bounds, module-instance/worker
overlay identity, conflicts with public exposure or another binding, alias
chains and missing/ownerless cycles. Canonicalize chains to one owner or reject
them deterministically. Do not resolve by basename/public spelling or transfer
the provider's initializer to the importing module. A binding retains a
dependency; it does not execute initialization or reorder conditional effects.

Appending hidden globals changes the argument-bank base for **every procedure
in the importer module**. A binary adapter must reencode all affected argument
operands and register metadata from bank/index identities; changing only the
expanded caller is unsound. The codec, linker, loader, disassembler and validation
must agree on the new typed record/feature; old readers must reject unsupported
features. No new hot-path register access opcode is needed.

A module-qualified global operand is an alternative, but it changes instruction
encoding, operand/effect catalogs, decoding and VM access paths across all users.
The private alias record is the recommended lower-cost option. Synthetic public
`.expose` names are rejected because they alter naming, storage ownership and
load-order semantics. A private alias is missing infrastructure, not a permanent
inlining exclusion.

The reviewed native increment uses the following opt-in 007 profile; these
records are not implemented yet. Append `META_GLOBAL_BINDING=23` and
`META_SOURCE_ARTIFACT=24`, guarded by feature bit `1 << 8`. After the existing
16-byte metadata wire prefix, a binding stores u32 destination-global,
provider-name StringRef, provider-global, provider-ModuleRef and flags, then
32 SHA256 bytes (68-byte payload total). A source witness stores u32 schema 1
and 32 SHA256 bytes (52-byte payload total). Both have address zero. Unresolved
bindings have zero flags and ModuleRef `RXBIN007_NONE`; linked bindings have
only `RESOLVED=1` and a checked same-image ModuleRef. Reject unknown flags,
schema, conflicting exposures/destinations, invalid slots and ownerless cycles
through one binutils-owned binding-plan helper.

The source form is `g7 .bind = "exact.logical.provider.module" 3 "64hex-sha"`.
RXLINK uniquely matches the full provider identity and derived source witness,
retains that module through its existing selection queue, then publishes direct
ModuleRefs. The loader uses same-image worker-overlay storage, never public
name/callable substitution. Allocate owned/public cells first, skip alias
destinations, then bind them with `globals_dont_free=1` before initialization.
Resolved input images remap their existing ModuleRefs on relink.

Qualification reuses the maintained exposed-global bounds, program-generation
control and module-initializer fixtures. Their observations must be checked
against the intended contracts. Existing worker-isolation checks do not prove
cross-module public-global sharing: add a focused shared-cell/mutation/worker-
isolation/one-owner-teardown control before reusing `globals_dont_free` for
private aliases. Current `rxvm_call` initializes all loaded modules before
dispatch; retaining a provider alone does not establish exact-site initializer
ingress for other entry paths.

For this RXAS-first profile, the witness is SHA256 of the exact immutable UTF-8
provider source acquired by B/L and actually assembled. An opt-in native
assembler setting derives the witness from its input; supplied declarations
are not proof. A matching explicit logical module-name option keeps serialized
name/description stable across source/output directories. It does not grant a
runtime search base. Use existing SHA256 primitives and input/relocation owners;
there is no normalized-image hash subsystem or native encoding backend in a
B/L request. The semantic-graph digest does not hash executable bodies.

Providers must be finalized before dependent acquisition/assembly. Rewriting or
re-rendering a provider invalidates dependent bindings until reacquisition/
re-inlining or an explicitly proved driver rebinding. Cyclic source witnesses
require such a driver proof, not a hash fixed point. Disassembly/reassembly
creates new source identity; binary codec roundtrip preserves the actual fields.
This is source provenance under the ordinary assembler/build contract, not
proof that arbitrary existing RXBIN bytes came from supplied RXAS.

### Initialization ingress and active regions

Where all-ready state is not proved, consider a typed operation at the former
call ingress equivalent to `ensure_module_ready(logical-caller-module,target)`.
It must delegate to the existing initializer state machine, preserve prelude/
argument evaluation before ingress, FAILED poison/no-retry and INITIALIZING
cycle behavior, and preserve failure dispatch. The same logical-caller identity
must reach initializer-reachable retained nested calls. Decide whether this is
an explicit instruction or a typed site/region preparation contract; neither
exists today. Unconditionally adding a static initializer dependency is not an
equivalent: an untaken branch must not trigger side effects or a new cycle.

For broader context/lifetime/abrupt-exit coverage, an optional explicit
`INLINE_ENTER`/`INLINE_LEAVE` plus typed region descriptor could own logical
base/argument bindings, private slot identities, changed numeric/signal state,
lifetime cleanup and initialization origin. The VM would run the same scoped
cleanup on every normal, branch, return and CANCEL/unwind departure. This could
simplify proofs that are otherwise duplicated as glue on every edge. It is
active runtime semantics, not passive metadata; regions need validated nesting,
failure/partial-entry cleanup, zero-allocation unwind, recursion and callback
rules. Reuse existing frame/context/lifetime owners rather than creating a
second initializer, reference or signal algorithm. Evaluate its cost against
plain glue before adoption; simple bodies need no active region machinery.

### Inline provenance and producer facts

A passive typed `INLINE_ORIGIN` chain can relate each emitted instruction to
its logical callee/provider origin and parent call site. An updated diagnostic
walker could reconstruct logical inline calls, including real nested frames
whose call site lies inside an inline region. Define fault versus next/resume
addresses, source/TRACE ordering, relocation, strip policy and feature/version
checks. This would improve logical diagnostics; it would not create physical
frames or make `METALINKPREG`/caller-address operations equivalent. Current
composite provenance alone does not implement it. It is distinct from both the
recommended relaxed physical-traceback policy and active cleanup regions.

Useful producer facts include definite component/status initialization,
non-escaping borrowed storage, no rebinding, no frame inspection, numeric-state
invariance, finite argument access, and an exact frozen target. They can reduce
proof cost and eliminate glue. Admit only checked, versioned contracts bound to
the executable body; a hint/annotation or native AST template is not proof.
New `noinline`/inline-request, ownership/effect or lexical-lifetime declarations
would be language decisions, with unsafe assertions/verification rules explicit.
They may enable a case; they may not redefine existing Rexx value/EXPOSE,
reference or signal semantics without approval.

Recommended decision order is the preservation/diagnostic contract, then the
private binding record, then exact-site initialization support if its coverage
is needed, and active regions only if explicit glue cannot meet the agreed
coverage simply. No extension is silently made a prerequisite for the initial
global-free, all-ready scalar/imported-helper slice. Final INL-AC03 coverage is
measured against the explicitly approved contract/extensions.

## Coverage and true retention boundaries

The intended design handles every representable call shape through the same
mapping/proof engine. Complexity alone is not an infeasibility reason.

| Case | Design disposition |
|---|---|
| Local/direct scalar, constants, read-only or writable formals, ignored results | Fresh mappings plus exact preparation/prologue/return rules; first feasibility slice. |
| Methods, receivers, computed objects/attributes, references and EXPOSE | Already lowered operand/storage identities; preserve single evaluation, aliases, ownership and lifetime. No AST receiver rediscovery. |
| Defaults, omissions, finite varargs, repeated/aliased actuals | Preserve presence/status/prologue and entry bindings; specialize known finite access without inventing values or aliases. |
| Nested/fused/fixed calls, loops, multiple returns, finite recursion expansion | Canonical call normalization, contiguous windows/identity, fresh CFG/control and bounded depth. |
| Imported/global-free body or proved existing exposed storage | Exact selected body/dependencies and admitted context; module boundaries remain intact. |
| Numeric/context changes, known signal scopes and cleanup | Feasible only with a complete lowering proof for all observable normal/abrupt exits; explicit glue may remain. |
| Unknown dynamic/native body | No executable body to clone; retain. A proved dynamic target can become an ordinary candidate. |
| Foreign private global; required initialization ingress without an expressible equivalent | Current representation cannot name/preserve the necessary behavior; retain pending a separately approved contract. |
| Arbitrary physical-frame/parent/address observation; unrepresented unwind/exception behavior | Removed frame remains observable; retain without virtual-frame support or a specific specialization proof. |
| Unbounded implicit register access or unknown relocation layout | No checked finite map/relocation proof; retain the call or reject malformed/unsupported input. |

Each retained call names the missing proof/capability. The initial profile is
smaller than the intended feasible coverage; its limitations stay open in the
existing plan. Broad fail-closed rules must not become a permanent substitute
for implementing a representable supported shape.

## Placement relative to optimization and linking

Use an expansion service independent of its driver, with a separate optimizer
service. For the requested already-optimized-module use case, recommend:
available exact-bound executable bodies, existing per-module optimization,
standalone expansion, then bounded cleanup of changed callers before final
assembly/link. The first RXAS adapter can also run before native assembler
optimization to reuse its existing cleanup as an independent comparison.
Neither ordering is a product-default switch. Both use one representation
across native/B/L/DSL producers. Body acquisition retains forward procedures/
imported modules rather than depending on native RXAS's streaming queue.

| Placement | Benefit | Cost/limit |
|---|---|---|
| Before RXAS optimization | Preserves explicit preparation/control and exposes new constants, aliases and dead paths to existing passes. | Copies a potentially larger body and needs executable-body access before streaming emission. |
| After each module is optimized, before assembly/link | Smaller inputs may avoid repeated local cleanup; works on independently produced RXAS. | Fused/implicit forms need canonical semantics; earlier choices can hide opportunities. The expanded caller still needs new analysis. |
| After link selection or inside link command | Knows actual selected module/provider bindings and can import only useful bodies. | Needs body decode/encode or retained target records, dependency/global/context proofs and shared cleanup; today's linker does not supply this service. |
| Semantic source IR earlier | Can exploit source-language facts and serve interpreted consumers. | A separate consumer need, not mandatory executable-body transport or a second general inliner. |

Keep ordinary producer and assembler optimization controls independent from
expansion. No extra public pre-link phase is required. Preserve runtime/dynamic
binding rules at every placement. The same expansion with all optional cleanup
disabled is the correctness baseline. Default migration and production
replacement remain future decisions under the plan.

## Minimal cleanup for already optimized modules

Optimized pieces are not an optimized composition. Binding an actual constant,
joining control and adding argument/result/context glue creates new facts.
Conversely, earlier optimization does not justify running every expensive pass
again or assuming all scaffolding will disappear. Prior lowering cannot always
be reversed to recover a higher-level opportunity.

**Required even with cleanup disabled:** validate new IDs/references and bank
mapping, reconstruct executable normal/exceptional CFG edges, derive conservative
storage/component/status/observation facts needed to check the expansion, assign
nonoverlapping bounded storage and call windows, and regenerate addresses,
metadata/graph indexes and task seals that depend on changed instructions.
These are correctness and publication duties, not profitability optimizations.

### Metadata and retention

Relocate executable operands and observation records together. `META_REG`
physical register numbers, TRACE's separate register-type/value-reference
domains, source-step/clause IDs and all address-bound records need typed maps.
When removing unreachable instructions, remove or deliberately reattach their
bound source/TRACE records with a proven meaning; do not leave them to drift
onto the next surviving instruction. Fresh IDs must not collide with the caller
or exceed the unsigned 32-bit metadata domain. The text reader normalizes the
native signed numeric spellings before constructing canonical checked records.
In the current sole-return inline profile, a binding opened before RET but
cleared only at the callee's detached end anchor receives a fresh matching
clear in the cloned caller. The original detached clear stays in the retained
callee. One executable `CNOP` follows any generated clears: they attach to its
address, so the debugger's backward lookup from the next instruction's
`addr-1` sees the closure. This straight-line carrier lets the current
transport and storage cleanup compact the caller; it is omitted when the
cloned bindings have already closed. The ordinary `rxas -n` path retains the
carrier. Native assembler optimization can fold adjacent CNOP forms, so an
optimized-assembly observation guarantee needs its own relocation proof.
TRACE code fields are raw byte codes, separate from ordinary escaped strings.
The adapter reads raw quoted interiors and renders admitted control/DEL bytes
literally, preserving unknown codes without an enum whitelist. Backslash token
boundaries remain a specific adapter gap. Nonnegative `K` constant-pool offsets
need a logical relocatable constant binding. These are unfinished representable
cases; a retained call or disabled expansion alone cannot make an unrelocated
offset safe.

Do not delete a procedure, global, callable contract, provider/autoload record
or initializer merely because its last direct call disappeared. Exports,
dynamic selectors/callbacks, other modules, expanded-body dependencies and
reflection can remain consumers. Prune private labels/constants only with a
complete reference proof. Retain ordinary public/library callable contracts.
`META_INLINE` remains optional compiler transport: a reusable library may need
it for later native compilation; a final image can strip it only under explicit
existing consumer/strip policy. It is not expander proof input or runtime state.
Rebuild affected graph/procedure/source indexes and task binding digests/seals
after code changes, preserving stable logical sources instead of temporary paths.

### Selected bounded simplification

These passes are consumers of reusable machine analysis, intended to merge
with later L RXAS optimization. Generic CFG/traversal/dominance and bounded
liveness stay with `llirgraph`; its checked access projection supplies explicit
reads/observations, possible writes and definite kills for target attributes.
`lltargetfacts` supplies RXAS storage/component/context transfer semantics and
complete implicit/metadata/TRACE/window observations. Canonical facts are
extracted from the existing opcode/effect/signal owner rather than duplicated
in cleanup. Facts bind to the exact frozen owner, revision, schema, catalogue
and preservation profile. Checked immutable rewrite plans precede atomic
publication; structural changes invalidate affected analyses.

A possible component write is not a definite kill. Checked arithmetic can
signal before writing its destination, and a default policy may ignore that
signal. Establish success/range or exact history/restoration before relying on
a new value. The current C optimizer's capability-demand, storage/component/use
and proof-plan structure informs this boundary; its conservative limits are
not language restrictions or the ceiling for later L reasoning.

The selected standalone deliverable includes this bounded cleanup profile;
execution can be disabled independently for correctness comparison:

1. Propagate bound constants/presence and exact copy/alias facts through the
   changed caller; simplify newly decided branches and unreachable blocks.
2. Remove proven redundant preparation/result copies, self-copies and outer
   call-window setup/reverse swaps exposed by direct actual-register mapping,
   together with unchanged context setup/restoration and certified effect-free
   private computations. Prove permutation/storage equivalence across the whole
   call region, including abrupt exits; adjacent-swap cancellation alone is
   insufficient.
   Preserve failing checks, flag/component observations, reference expiry,
   cleanup and native payload effects. A dead scalar result alone is insufficient:
   its write can release hidden reference/native payload state in the destination.
3. Thread trivial joins/branches and remove now-unused private glue/anchors only
   when their entry/base/slot-identity obligations are dead. Recalculate storage
   demand and apply the selected private-scalar reuse proof independently.

Structural acceptance includes a simple reordered-actual leaf such as `f(y,x)`
whose cleaned expansion contains neither the original call nor its outer
setup/restoration swaps; an alias case that retains its required value snapshot;
and a body with a retained nested call whose contiguous window and restoration
remain valid. Scope assertions to the removed outer call machinery: required
body/nested-call swaps and snapshots are not failures. These are proposed
acceptance checks, not claims of implemented cleanup.

Start with one bounded pass/worklist over the changed caller. Include consumers
outside the cloned region: an inlined constant return may simplify a following
caller branch. Stop at opaque/observation/alias barriers unless the canonical
effect contract proves propagation. Local fusion/packing can be left to the
ordinary assembler; loop reuse, general placement, aggressive specialization
and full optimizer iteration are optional, separate work. Native RXAS's existing
passes are useful evidence/validation, not already callable B/L optimizer APIs.

For example, separately optimized `return n+1` remains dependent on its formal;
binding actual `4` can expose `5`, eliminate a caller comparison and leave no
argument/result glue. A helper reading a volatile alias or changing numeric
state may retain both transport and restoration. Neither input optimization
nor instruction-count reduction proves that removing those operations is safe.

An owner/revision/schema/binding-scoped summary is reusable only for unchanged
inputs covered by its declared dependencies. Cloning invalidates caller CFG,
dominance, value/component/status liveness, alias/base/slot identity, signal/
cleanup, observations, register assignment and address/index facts as affected.
The first implementation may rebuild the whole changed caller's analyses.
Incremental preservation needs explicit dependency/invalidation contracts; do
not reuse a callee's old liveness or graph facts as proof in its new context.

## Bounded selection and profitability

Legality is mandatory; selection is policy. The proposed proof of concept
includes a cheap deterministic default selector as well as an all-legal
correctness mode. The default favors small direct leaf bodies and cases where
cheap constant/presence binding demonstrably removes control or transport. It
excludes recursive SCCs by default; an explicit depth/growth-bounded opt-in can
expand a finite prefix. Recursion exclusion is policy, not semantic infeasibility.

For each legal candidate, derive reachable body cells after cheap binding and
estimate the complete removed call/window/context sequence against inserted
instructions, required argument/result/context/lifetime glue, frame cells and
observation metadata. Apply approved per-body/caller/request limits, then rank
by those structural savings, specializing constants/presence first and using
stable module/procedure/call-site order to break ties. Reject candidates whose
required glue exceeds the profile's growth allowance, and retain larger
duplicated/no-proved-benefit sites unless explicitly requested. No profile may score just
the callee's instruction count while ignoring ownership and caller work. Cheap
estimates guide selection; they are not measured runtime-speed claims or a
promise that lower cell count is faster.

Use explicit per-call, per-caller and whole-request bounds for instructions,
encoded code cells (opcode plus every operand), blocks, fresh frame cells,
inline depth, total growth, literal/metadata/retained bytes and proof/cleanup
work. Charge rejected candidate attempts to the work budget too. Process
candidates in a stable order with a stable tie-break; never
depend on hash iteration or elapsed wall time for semantic selection. Track
recursive SCCs/depth and do not revisit a call indefinitely. `always`/testing
selection may bypass a profitability threshold, never legality/resource limits.
`never` preserves calls. Names and numeric defaults require approval and
representative evidence; this document invents no speed threshold or default
performance claim.

Expansion plus cheap cleanup can be assessed before atomic publication. If
the result exceeds policy limits, discard the candidate and keep the call.
Malformed input is an error; a legal unprofitable call is not. Report optimizer
cost, output size/frame demand and program execution cost separately when
evidence is eventually collected. Research feasibility does not commission a
production benchmark campaign or establish a production performance verdict.

## Validation contract

The plan owns scheduled checks and completion. The implementation must obtain
independent expected behavior before comparing to native tools and inspect
conservative and cleaned target records early. A passing simple scalar example
cannot qualify the mapping/profile above.

Maintain small behavioral and structural controls for each admitted mechanism:
single evaluation and effect order; value/EXPOSE alias snapshots; omission and
flags; entry-anchor UNLINK; LINK/SWAP identity; argument-bank/nested-window reuse;
RET move/copy and ignored results; reference expiry/native payload lifecycle;
multiple returns/control; numeric and handler restoration on admitted exits;
module/global/initialization behavior; source/TRACE and transitive observations.
Negative controls must assert both retained calls and specific reasons for real
unsupported cases, and reject stale bodies/references and malformed schemas.
Classify every refusal as malformed input, unavailable body, policy limit,
missing proof/implementation or a current semantic boundary. A negative
regression fixes the observed reason and atomic retained behavior; it does not
prove the shape is inherently infeasible. Where practical, pair it with an
independent executable counterexample for the unsafe rewrite and a positive
case once a valid proof admits the shape. For example, reusing a plain scalar
cell after `SETTP` requires a whole-cell `NULL` before the next cell's partial
integer `LOAD`: without it, `GETANDTP` can observe stale compiler flags even
though the integer output is correct. Keep representable missing-proof cases
open for a later positive regression rather than treating old optimizer refusals
or test expectations as language rules.

Run an expansion with native RXAS optimization disabled to prove its own
semantics, then separately check the selected cleanup profile and generated-code
structure. Preserve native disagreement as a classified issue rather than
copying a native defect. Include small-after-large, interleaved owners, limit,
cancellation and failed-output cases to prove atomic lifecycle. Whole-toolchain
integration, both concrete VMs and installed/platform delivery remain their
explicit later plan gates; this design document runs or closes none of them.

## Owning source contracts

### First constructed-model implementation contract

The first INL-STEP02 slice uses the optional `rxfnsl-target` package, built
explicitly, with `lltarget` and `llinline` namespaces. `lltarget.draft` owns an
existing `llir.arena`; publication freezes/verifies that arena into one
`llir.module` per target artifact. A versioned, nonexecuting catalogue function
holds checked module/procedure/register descriptors. It is excluded from target
procedure counts, rendering and machine CFG analysis and is never passed to a
generic executable optimizer. Actual target bodies are separate L functions;
their structural L terminators are distinct from recorded RXAS `RET`. The
target-owned cursor now publishes direct normal successors through the shared
`llir` control contract; wider exceptional and indirect control remains a
separate proof boundary.

`draft(logicalModule,artifact,globals,entries,bytes,work)` supplies `source`,
`synthetic`, `procedure(name,locals,origin,expose)`, module `record`, and atomic
`finish`. A body draft supplies owner-checked `reg(bank,index)`, canonical
`emit(mnemonic,operands,origin)` and typed `record`. Operands have register,
integer, byte-string, procedure, symbol and checked exact-literal forms.
Records persist canonical opcode identities and typed operand vectors as L
extension attributes, never instruction source strings. `finish` owns forward
local/label/import binding. Published module/body/record views borrow the exact
L owner and expose source-order records and procedure identities; foreign or
expired handles are rejected. Existing `llrxas` allocation APIs remain intact.

`llinline.bind(caller,callId,provider,procedureId)` captures exact frozen owners
and body binding; `inspect`/`expand` return specific keep/invalid/limit outcomes
or a new frozen target with mappings. The initial admitted body is a direct
straight-line scalar return, with no global, local-history, reference, context,
handler or nested-call effects. Entry is an explicit closed-main, default-
halting-signal integration profile with checked plain-integer actuals; unknown
inherited policy retains the call. HALT/CANCEL has no resumable continuation in
that profile. Relaxed physical traceback alone is insufficient. Conservative
expansion uses fresh argument/result anchors and retains caller marshalling;
outer setup/reverse-swap elimination remains required by INL-STEP05/AC05.
Independent constructed-model fixtures exercise the service and its output;
they do not complete the real RXAS adapter or the full approved POC profile.

### Scalar expansion and transport development contract

The next slice adds `llinline.expandmany` over exact bindings to one original
frozen module and separately callable `llcleanup.transport`. The batch exposes
each site's decision and one atomic result. Valid non-root sites retain their
calls under `closed-main-default-halt.v1`; malformed or duplicate bindings fail
the batch. Source-step/clause identities and scoped register metadata are
freshened, and copied origins retain every contributor plus the call site.
The original callable declarations and inline payload stay with their body.

`lltargetfacts` projects checked RXAS accesses into the existing `llirgraph`
liveness service. Its current transfer profile covers straight-line integer
storage, aliases/permutations, status, selected numeric setup and observations.
Checked integer arithmetic requires a range proof before its destination is
treated as available: overflow may be ignored without writing a value. The
current proof carries bounded integer intervals across acyclic joins and checks
all extremal add/subtract/product outcomes before treating the write as
successful; unbounded operands or any failing endpoint retain the call. Bounds
do not prove whole-cell equality, status, freshness or lifetime. A local
return uses MOVE/reset only after proving original owned storage. Repeated
activation must account for recycled local contents; the current proof admits
the private return cell reset by the preceding successful MOVE and retains
unproved history cases. Fresh nulls must not manufacture original freshness.

Transport cleanup remaps the whole caller's executable and metadata references
using storage identity. It can remove redundant LINK/UNLINK, outer ordering and
restoration swaps, exact-storage self-copies and unobserved plain glue. Equal
integer values alone do not justify removing a copy. Required snapshots and
retained counted-call windows remain intact. The typed assignment owner now
compacts complete live cells and reuses disjoint fresh-root plain scalar cells
only with checked entry, component/status and whole-reset premises. Sparse
registers alone do not meet the wider reuse requirement. Wider CFG, deeper
nesting, dynamic varargs, reference/aggregate,
imported/foreign-global and wider context coverage remains open in
INL-STEP04/05.

### Direct target CFG prerequisite

Target schema version 2 separates source-visible control records from ordinary
records. The draft cursor creates real L blocks for RXAS address groups, and
structural label-entry jumps resolve forward and alias declarations without
appearing in rendered RXAS. `BR`, `BRT`, `BRF`, `BRTF` and `BRTPANDT` have checked ordered
normal edges; `RET` and `EXIT` have none. Other flow forms remain transportable
with an incomplete normal CFG proof. A signal-capable normal instruction does
not by itself make its represented normal edges incomplete. The target
validator checks source opcode/edge/label agreement independently of the
builder, including decoded graphs, and enforces native RXAS's module-wide
label-definition namespace.

Source-order record views include both descriptor kinds. A record's `blockid`
names its L address group and `addressid` names the next RXAS instruction,
with zero for a body-end anchor; neither is a byte offset. Zero-width metadata
after a terminal RXAS instruction remains at that end anchor, outside normal
execution. The owner derives `normalcfgcomplete` and `endobservations` rather
than accepting a caller assertion. Straight-line scalar facts traverse a
verified sole-return normal chain through structural thunks, ignoring detached
end-address metadata tails for runtime transfer. A selected callee's tail
remains in its original declaration and is not cloned into the caller's
continuation. The original straight-line fact path retains source branches,
cycles and multiple returns; the acyclic proof below admits a selected subset.
The linear cleanup keeps changed callers with branches or typed end-anchor
observations before making source-order storage assignments. The separately
proved acyclic caller path below handles its admitted branch profile.

The constructed CFG regression checks ordered branches, forward/alias labels,
backward lexical acyclic flow, multiple RET, end-address metadata and
wrong-but-well-formed decoded edges. An independent two-arm RXAS input also
round-trips through the reader, writer, assembler and VM with both integer
zero and nonzero conditions. This proves target representation and transport,
not branch inlining. For current refusals, negative regressions assert the
specific pending reason and unchanged original owner; malformed labels or
edges are invalid-input tests. Neither kind proves a shape inherently
impossible. When a proof is implemented, replace its pending regression with
a positive expansion and execution check.

### Acyclic scalar callee expansion

The current B/L proof admits a direct-branch callee with a finite acyclic
normal CFG and multiple RETs while its root caller remains on the qualified
linear profile. The shared `llirgraph` RPO/SCC index supplies reachability and
cycle checks. Each feasible path snapshots the complete scalar component and
status state and the active/base/identity/context frame bindings. A join keeps
an integer constant only when every incoming path proves the same value;
`wholeknown` remains entry-independence and compiler ABI status has separate
known bits for `VAL` and `NOTSYM`.
An unknown but defined integer branch condition takes both zero and nonzero
arms, including values other than one. Incompatible bindings, undefined
conditions, cycles, abrupt exits and deeper or unproved nested calls retain
the ordinary call with a specific pending reason. One same-module scalar
child in a reached branch uses the path-local history proof below.

Each reached RET has its own typed copy/move/integer/bare action and inverse
permutation. The result transfers before inverse swaps, active/base restoration
and owned-local expiry. The proof meets caller-visible and recycled-local state
after every feasible normal exit; a bare RET does not invent a result write or
definite kill. The emitter clones only feasible records, freshens labels
against all module label and constant names, and rewrites only declared branch
operands. Each return branches to one post-arm closure. A common-entry scoped
binding whose original clear is detached at the callee end closes there, then
one `CNOP` carries the clear before caller continuation. A branch-local scope
requires further observation proof and stays a call in this slice.

The opt-in graph cleanup below handles an acyclic, closed-root scalar caller
after expansion. The existing straight-line cleanup still compacts callers
with a single-return `CNOP` carrier. The target draft also checks initial
catalogue allocations before using their handles, preserving a resource-limit
diagnosis for very small entry budgets and allowing the inliner to return the
unchanged original.

### Finite optional scalar prologues

An admitted caller supplies the actual physical argument cells, including any
padded omitted cells, and a proved count/window. Expansion does not create
missing cells or equate count with presence. The callee's executable prologue
runs in source order against current active bindings, so an earlier default
can change a later test when formals alias. `BRTPANDT` reads the VM's readable
status band; the proof handles masks 256 (`VAL`), 512 (`NOTSYM`) and their union
768, with per-bit knownness independent of integer value and whole-cell
identity. `SETTP` follows public-band write semantics; NULL resets the ABI
band, whole COPY/MOVE and ACOPY carry it, and ICOPY preserves it. Required
copies and status writes remain executable in the conservative expansion.

For an unknown but caller-independent one-bit presence test, the taken and
fallthrough successors receive separate facts with that bit respectively set
and clear. A false union test clears both; a true union test alone does not
identify either bit. Joins retain only facts true on all incoming paths.
An unreset recycled local whose status can depend on an earlier activation
stays an ordinary call. Unsupported masks, missing physical cells and unproved
count/window retain their exact input with specific reasons. This is finite
physical-cell specialization; arbitrary dynamic varargs remain open.

### Retained child calls in acyclic callee branches

The current scalar proof admits one same-module child call on a reached branch
of an acyclic outer callee. The child remains a real machine call in the
expanded caller. Before forking the outer paths, the facts owner allocates
stable child owned-storage IDs. The IDs do not imply execution or initialized
contents. Each path snapshots whether the child may have run and which owned
bases its last normal exit reset. At a join, a never-activated path contributes
fresh-entry behavior; an activated path with unreset observed state prevents
an unsupported recycled-state proof. Cell values, ABI bits and frame bindings
meet independently. Reached child calls use the existing normalization and
scalar transfer; their return and selector rows do not enter the outer
expansion plan.

A counted child still observes a contiguous implicit window, including slots
crossing the outer `r`/`a` boundary. The expanded caller keeps the source
setup and inverse swaps and preserves the exact window. Root graph analysis
now summarizes retained child accesses, so cleanup can reason about reached
fixed-argument calls. Graph transport continues to keep counted calls under
its implicit-storage guard: remapping the visible count register alone could
change the unspelled actual pointers. Dense placement may preserve a window
with fixed contiguous slots; removing its setup requires a separate adjacency
or constructed alias-window proof. The repeated counted example currently
keeps a wide frame and binding glue under this guard, so alias-aware reuse
remains INL-STEP05 work. Imported bodies, deeper recursion, aggregates and
abrupt exits remain separate categories.

### Ordinary string components

The closed-root normal-return profile admits ordinary UTF-8 string literals,
`SCOPY`, whole COPY/MOVE and register or immediate-string RET. The facts owner
tracks integer availability/bounds, ABI status, string bytes with VM-private
flags and normalization certificates, and absence of reference/native payload
or attributes separately. Fresh or reset cells establish independent empty
string state; an unreset recycled local does not inherit those facts. LOAD S
replaces only the string component while preserving integer and public status;
SCOPY copies only that component. ACOPY copies the status word, including string
certificates, without copying bytes. Its certificate independence therefore
comes from the source status, even if its bits differ from the destination's
bytes. An unproved recycled source prevents the copy from certifying the
destination string metadata. Whole COPY/MOVE transfer the complete proved
state, and MOVE resets the source.

An immediate-string RET lowers to LOAD S on the entry-captured result cell,
matching the VM's partial setter; an ignored result has no write. A register
RET retains the original own-base MOVE versus borrowed/rebound COPY test and
requires whole-state independence. String-only writes have no integer-write
or integer-kill fact. The current liveness owner receives conservative whole
reads and partial writes for strings; it may leave dead string stores in place.
`_plain` remains the narrower integer-only reuse/lifetime-deletion premise,
so an ordinary string ENDLIFE stays executable. Binary loads, hidden payloads,
aggregate attributes, string-bearing storage reuse and abrupt exits still need
separate proof before expansion or removal.

### Acyclic caller cleanup

The graph path analyzes the exact frozen expanded caller using `llirgraph`
RPO/SCC and projected access liveness. It admits a complete acyclic,
register-only, closed-root scalar CFG. Each reached instruction has incoming
storage identities and independent value, component and status facts; joins
require compatible active/base/identity/context bindings. Root restoration
requires both canonical identity and active pointer equality to its base at
every normal RET. Reached admitted scalar child calls have effect summaries;
counted implicit windows still keep transport under the guard above. Module
globals, exceptional exits, opaque physical-frame observers, changing scoped
metadata bindings and unsupported operations keep the unmodified owner with
a specific reason.

`llcleanup.transport` uses reached storage identities, never equal values or
lexically simulated bindings, to remove proved LINK/UNLINK/SWAP mapping glue.
It retains required COPY/MOVE snapshots, checked numeric writes and lifetime
effects. It deletes only proved plain ENDLIFE, dead scalar setup and
same-storage copies, folds proved branch choices and preserves necessary
earlier-arm exits. Scoped META_REG remains observable through its matching
clear; matching names compare as exact binary values. Point source/TRACE
records follow their executable address group, and a closure `CNOP` retains
the clear's address before caller continuation. All changes publish through a
checked new target or return the exact frozen input.

After transport, `llcleanup.storage` reanalyzes that new owner and uses the
existing assignment service in injective dense mode. Distinct reached cells
remain distinct; unreferenced cells may disappear without inventing a reset.
The linear source-order reuse/reset schedule remains limited to its proven
straight-line profile. A backward-lexical DAG can execute the later source
cell first, so cross-block sharing needs a separate interference or arrival
reset proof. An integer may be available but not an exact constant at a join;
bounded hulls now prove successful checked arithmetic when every extremal
operation succeeds. Unbounded operands, failing endpoints and value
correlations such as `x-x` retain their specific proof gap. Typed end-anchor
observers and more general CFG/alias cleanup remain open under INL-STEP05.

### Straight-line callee rebinding contract

For admitted callee LINK/UNLINK/SWAP, `lltargetfacts.analysis` supplies exact
`rebinding(callId)`, `returnaction(callId)`, `swaps(callId)` and
`swap(callId,ordinal)` facts. The action is `none`, `integer`, `copy` or `move`,
computed at the original return. Originally encoded local storage moves only
when its active pointer equals that same slot's immutable base. An owned value
returned through an argument or a different swapped local slot still copies.
An ignored result performs no transfer and establishes no MOVE reset.

A rebinding expansion reserves `S=locals+arguments+1` contiguous active slots
in original local/a0/argument order, `S` untouched entry/base anchors, and
captured-result/scratch cells: `2*S+2` cells. Entry anchors borrow actual
pointers without copying argument values. The result pointer is captured
before body mutation. Lower an original UNLINK to LINK from that original
slot's fixed anchor; a previous swap never changes its base. Bodies without
rebinding retain the smaller scalar layout.

Transfer the return value before exit restoration. Replay body swaps in
reverse to restore actual VM slot identities, then unlink active slots,
anchors and the captured result. Only then end private local/a0/scratch
lifetimes. Relinking a pointer alone cannot restore permutation identity.
Cleanup may eliminate this machinery through its complete caller proof.

Repeated-call history follows reached storage and observed components. Pure
binding changes do not observe private contents. Integer-only ICOPY can remain
independent of unobserved side-component history; whole COPY requires the
corresponding entry-state proof. For example, SETTP after a copy can leave flags
for the next call to observe before that SETTP executes again. Tracking an
original recycled cell does not itself prove equality with a fresh inline cell.
Use actual reset/overwrite proofs; do not emit zero initialization to conceal
the difference. The current nonrecursive same-module profile owns one original
cache identity per procedure. Imported aliases and deeper/recursive activation
require their own cache/binding proofs in the later categories.

One retained same-module scalar leaf may execute inside an expanded outer
body. Its fixed or counted current-frame window is normalized privately,
including physical r/a0/aN adjacency, without publishing a second root call
fact. A counted child forces the complete ordered active-slot and entry-anchor
layout even if the outer body has no LINK/SWAP. The child opcode and required
source/inverse swaps remain; the outer return still uses its original result
and base ownership. An active-procedure and depth check precede recycled-base
reuse. Count is not an argument-presence proof.

Finite straight-line `LINKARG` selection uses a per-root-site checked selector
row. It reads a register selector and adds its immediate offset before changing
the destination binding. Index `0` selects the current active `a0`; `0..N`
is bounded by captured entry arity, even if the body changes the integer in
`a0`. Lowering uses ordinary LINK to the selected **active** argument slot,
so preceding SWAP/LINK effects remain visible. A known-passing `ICHKRNG`
keeps its exact instruction; unproved or failing guards retain the call under
the current signal profile. Raw LINKARG gains no invented range signal.
Finite padded optional presence and defaults use the separate ABI-bit and
control proof above. Dynamic-count access and other component/status bands
remain open.

Private same-module scalar globals now use one persistent module StorageId per
`gN`, shared through fresh and recycled callee frames. The physical frame order
is locals, globals, `a0`, arguments; owned recycler history excludes globals.
Initial global contents are unknown until a full NULL/whole definition.
Selected calls require the caller's global active pointers and slot identities
to match their module bases, while the callee must not rebind a global slot.
Fixed actuals retain their typed bank/index, including a borrowed `gN`.
Cloned g operands, scoped metadata and TRACE keep the original module bank and
index; `RET gN` copies rather than moving or ending module storage. Both
cleanup services keep the conservative expanded caller before their r-only
mapping when globals are present. Imported/private foreign aliases, global
rebinding, counted windows across globals and initializer ingress remain open.

Current negative controls distinguish proof limits from inherent language
limits. Missing exact body, status-derived unknown count and active recursion
retain the original frozen module with precise reasons. The unknown-count
control also executes the retained call through the ordinary assembler/VM;
its zero-actual result is independently expected. A later supported proof
should turn the relevant negative regression into a positive one rather than
preserve an obsolete refusal.

The following current sources constrain this proposal; changes require the
corresponding catalog/proof/profile review:

- [Canonical opcodes](../../../../binutils/include/rxops.h),
  [effects](../../../../binutils/include/rxopeffects.h),
  [signals](../../../../binutils/include/rxopsignals.h) and
  [status bands](../../../../binutils/include/rxflags.h).
- [Call/return/link/reference handlers](../../../../interpreter/rxvmhandlers_control.inc),
  [fixed/fused handlers](../../../../interpreter/rxvmhandlers_system.inc),
  [argument indexing](../../../../interpreter/rxvmhandlers_string.inc),
  [frame activation/identity](../../../../interpreter/rxvmintp.c) and
  [VM guide](../../../../docs/ai-context/RXVM_INTERPRETER.md).
- [Global binding/loading](../../../../interpreter/rxvmload.c) and
  [module initialization](../../../../interpreter/rxvml/rxvm_run.c).
- [Assembler body lifetime and register encoding](../../../../assembler/rxasassm.c),
  [flush owner](../../../../assembler/rxas_opt.c),
  [machine pass contracts](../../../../assembler/rxas_flow_pass.c) and
  [assembler guide](../../../../docs/ai-context/RXAS_ASSEMBLER.md).
- [Existing target records](../../../../lib/rxfnsl/rxas/llrxas.crexx),
  [L IR transformation](../../../../lib/rxfnsl/rexx/llirservices.crexx),
  [L graph analysis](../../../../lib/rxfnsl/analysis/llirgraph.crexx) and
  [selected binary metadata reader](../../../../langtools/common/llbinmeta.crexx).
- [Native/future inlining boundary](../../../../compiler/docs/inlining_design.md#future-inlining-boundary-review)
  and [authoritative toolchain plan](../PLAN.md).
