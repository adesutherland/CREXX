# RXAS inlining architecture

**Status: governing architecture with an implemented research checkpoint;
full representable coverage and integration qualification remain open.** The [toolchain plan](../PLAN.md) owns INL-AC01–09 and
INL-STEP01–06, including STEP03F. This document records the governing architecture before a
detailed instruction or implementation design. The earlier per-opcode admission
history and mirrored `INLINE_ENTER`/`INLINE_LEAVE` proposal are superseded;
neither is an approved ISA. Existing bounded implementation results remain
regressions, not evidence of full call or signal coverage.

## Outcome and placement

One Level B/Level L target inliner operates on a checked RXAS instruction and
module model, independently of the producing language. It has two target
driver placements and one standalone proof-of-concept adapter:

| Context | Selection and order | Available bindings |
|---|---|---|
| Assembler | Inline calls within the module before its normal RXAS optimization. | Exact local executable bodies and local globals. |
| Linker | Select the actual input modules and executable bodies, then inline cross-module calls and run the bounded cleanup below before final relocation/sealing. | Exact selected provider, its private globals and initializer/dependency graph. |
| Standalone POC | Supply checked assembler-like or linker-like context to the same service; render ordinary RXAS where the selected profile permits. | Explicit input owners and binding facts, never compiler AST inline payloads. |

The body cloned at either placement is executable code from the artifact being
assembled or linked, not a separately maintained inline template. Link-time
selection and inlining use the same frozen input revision. Cross-module
expansion also requires a closed runtime binding context, checked image-local
binding or equivalent proof that the eventual call resolves to that selected
body; the current loader's context-wide export resolution does not establish
this by itself. A provider change requires rebuilding/relinking for a new
result; an existing sealed image still executes its original selected body.
Unresolved, native and dynamically loaded bodies remain calls. A dynamic
dispatch may inline only if the exact selected body is proved and its selection
and failure behavior are preserved. No new inliner proof side channel is
added to `.meta`; normal signatures, runtime startup,
linker and source records retain their existing roles.

The inliner separates **availability**, **semantic legality** and
**profitability**. A representable case with unfinished proof remains an open
implementation case with a named reason, not an infeasible category. Depth,
recursion, size and work limits are deterministic policy refusals. Expansion
without optional cleanup must execute correctly.

The generic expander has no separate opcode whitelist. Normalization must
publish complete typed operands, references and scope ownership for an
instruction it accepts; an unrepresented form is a normalization error. The
expander's site checks cover exact selected body and owner identity, the shared
target/activation proof, executable ABI representation, runtime entry support
and explicit policy/resource limits. The former local clone-category,
physical-observation mask, textual last-instruction fallthrough and implicit
opcode gates have been removed. Optional direct register mapping and cleanup
proofs can fail while the generic scoped expansion remains available.
A requested site with no canonical call descriptor is an invalid binding;
an explicitly described selector that does not invoke a procedure remains
ordinary selection.

Level L owns any remaining proof imprecision. A possible callback is a real
observation obligation, but a callback elsewhere in a procedure does not by
itself prove that a selected call changes an observation. The shared flow
service must identify the affected source point, binding and resource phase.
The inliner must not recreate a broad callback or opcode refusal. Unknown
resource state at a raw root remains unknown until a checked entry or ownership
fact establishes it.
Matching a callback before and after expansion also needs an observation
projection: the same callback may see a changed physical frame, source/code
location or activation even at the same source point. A shared relational
proof must preserve those observations or leave that site unproved. It must
keep callback havoc for values and bindings the callback can change.
The next shared proof uses a point/edge/preparation callback fact, a checked
source-to-candidate instruction and activation correspondence in target remap,
and bounded preservation queries for activation and normalization. Matching
events must preserve order, operands, reachable storage, phase and permitted
physical observations; the correspondence is mapping data, never an asserted
proof. Identity normalization does not need an absence-of-callback premise.
Inserted clears and new inline-owned storage need their own resource proof.
The initial point-local query proves only a component delta at an identical
callback checkpoint with a complete explicit observation projection and
distinct private storage. Its result does not certify an earlier clear or a
different source/candidate owner without checked correspondence. The additive
correspondence argument relates actual checkpoints from both flow owners only
for the existing unchanged-activation profile. The query checks all six frozen
model/profile/CFG dependencies, the mapped event and full scope stack, equal
callback possibility and private storage disjointness in each flow. Its delta
is still a point-local premise: normalization must prove that earlier entry
clears compose to precisely that difference. A call-to-scope clone needs its
own checked activation mapping. Callback havoc remains in the flow solver.
`flow.entrycoverage(fn)` separately certifies extraction of owned-entry
demands. Defined reads need no clear; dirty reads retain their original owned
base/component grouped by source operation. Unknown binding, availability,
origin, child projection or exceptional observation phase makes the coverage
incomplete and publishes an unresolved aggregate row. Zero dirty rows has
meaning only with complete coverage. A source/candidate entry-clear certificate
must license each clear from a source demand, prove candidate coverage complete
with no dirty or unresolved demands, and retain the separate callback proof.
The original unconditional normalization certificate keeps its callback veto.
`lltargetnormalize.entrycandidate(owner, flow, fn, work, bytes)` uses complete
source demand coverage to prepare one procedure's clears through that same
placement, warning and remap algorithm. Its `tentative` status grants no
publication: ordinary `result()`, `operation()` and `warnings()` still return
the exact input, identity mapping and no warnings. Only `candidate()` and
`candidateoperation()` expose the private draft and map for rebuilding proof
owners under the original operational premise. The caller separately budgets
and retains both profile/CFG/flow stacks, source summaries, correspondence and
the conditional/entry certificates. Construction's `reservedwork()` and
`reservedbytes()` include the lower remapper's whole reserved envelope, not
those external proof phases.
`entrycertificate.matchesresult(owner, fn)` revalidates the live theorem,
requires a nonzero transaction result stamp, and checks the immutable candidate
identity and source-name-mapped candidate function. `matchessource(flow, fn)`
binds the original analysis and function. `commit(entrycertificate)` checks
both predicates for the single modified procedure before publishing the
candidate, map and warnings together. A failed or limited stage leaves the
exact original and exposes no usable tentative draft. Neither tentative
construction nor commit changes the old `complete(fn)` theorem or supplies
CALL-to-logical-scope activation equivalence.
The additive `entrycandidates(owner, flow, functions, work, bytes)` stages one
final candidate for distinct requested functions. `commitall(certificates)`
requires exactly one certificate per actually modified function, all bound to
that same final result. A single-function `commit` cannot publish a larger
batch. The entrymode completeness mask also discharges the bounded dependency
closure of dirty returned origins; an unrequested dirty provider cannot make
its apparently clean caller normalized. Candidate certificates check the same
dependency closure for complete, zero-dirty coverage.

`change.identityproof(sourceflow, candidateflow, correspondence, fn)` names a
different entry theorem for an unchanged procedure. Exhaustive source and
candidate coverage must have zero dirty demands and no unresolved row; the
candidate's returned-origin dependency closure must also be discharged. Exact
owner/profile/CFG/operation correspondence rejects an own-procedure insertion
or rewrite. This proof needs no callback-preservation summary because no
operation in that procedure changes. It retains and revalidates all owners,
result identity and the by-name result function. It does not change
`change.complete(fn)`, prove callback absence, or grant activation/call
equivalence. Unknown external footprints remain unresolved coverage.

The POC adapters use `llinline.normalizationreceipt`. Assign its lazy `owned`
or `borrowed` factory result into a permanent request slot before querying it,
and never copy the prepared receipt. It retains both exact proof stacks and
the final correspondence, callback and per-function entry certificates.
For complete zero-dirty unchanged procedures it retains the separately named
identity certificate without forcing source summaries. An unproved dependency
leaves that function's entry obligation open; invalid or limited proof owners
still refuse the receipt. Modified procedures require their full certificates.
`context.certifyreceipt` borrows that exact slot and revalidates it when a
conditional normalization proof is needed. Canonical fact admission and the
separate activation/ABI guards remain unchanged. The existing unconditional
convenience functions retain their original contract.
Native enrollment keeps the two phases: canonical S, then one final candidate
C. With no conditional batch, the ordinary path remains. Otherwise the adapter
builds every eligible clear directly from S, enrolls C in fixed module slots,
and finishes all receipts before exposing output, composed maps or warnings.
A required proof failure is a precise normalization error with no partial
publication. One session reserves construction and retained proof envelopes
once; source and candidate analysis owners consume their existing disjoint
allowances. Standalone receipts likewise reserve their complete owned pipeline
before expansion uses the remainder.
Reachable unknown actions and external effects without a complete owned-read
footprint also leave unresolved coverage, even if no later explicit read occurs.
Borrowed observations include typed TRACE/metadata records. Canonical scope
entry classifies a register count directly at the executor's post-preparation,
pre-capture checkpoint, before count/list validation or owned-slot checks can
return. Raw dirty integer provenance contributes the original owned backing;
a preparation swap changes that backing even when the count spelling remains
the same. A defined count contributes no dirty demand. Ambiguous exceptional
phases, unknown scope state and potentially reentrant preparation leave that
count checkpoint unresolved; simulated clear state alone is no resource proof.
The native selection session prepares every candidate owner before phase 1
enrollment. Receipt finalization waits for the phase 1 capabilities and exact
candidate facts, then publishes composed raw-to-candidate maps and warnings
without replacing enrolled owner cells. Source and candidate proof arrays stay
fixed for the request lifetime, and every receipt/proof phase draws from one
disjoint request allowance. Unproved receipts abort before publication; the
separate site activation permission still governs expansion.

## Shared target analysis

The canonical binutils opcode and signal databases own executable effects.
Their generated Level L projection must describe explicit and implicit
register selectors, ordered compound effects, reads, definite and possible
writes, clears, binding/permutation actions, call and exit effects, normal and
exceptional successors, signal names/phases/policy changes, context and frame
observations, and every relocatable reference. Missing facts are explicit
unknowns. Ordinary instructions are cloned by this description and typed
remapping; the inliner does not reimplement their value algorithms or acquire
a mnemonic-by-mnemonic admission list.

An opaque external action separately records whether its intrinsic Rexx-value
footprint is completely described by typed reads and writes. The catalogue's
`intrinsicfootprint` fact and the matching transfer-action accessor are zero for
unknown and one for complete; only external actions may carry one. The whole
SAY/SAYX output family has this complete intrinsic inventory: register forms
read the string component of their operand, literal forms read no Rexx storage,
and neither writes a Rexx value. Their opaque optimizer barriers, observable
output, possible host callback and signal uncertainty remain. Legacy SAY exits
can reenter, so the hook still havocs values and bindings and retains an unknown
observer. Complete intrinsic demand extraction neither proves callback absence
nor authorizes activation changes or publication by itself. Unknown external
footprints retain an unresolved demand even without a later explicit read.

The canonical opcode-effect row also owns a per-operand value-writer kind,
exported by `catalog.valuewriter`: `setter`, `field`, `none` or `unknown`.
Shared scalar/string/binary setters release old reference/native payloads;
their kind derives normal-path resource clears and a captured prewrite native
payload condition with a possible finalizer. This does not clear active
attributes or establish callback absence. Reviewed direct field assignments
and typed copies preserve unrelated payloads; a scalar result type alone does
not distinguish those implementations. Unknown writers keep conservative
callbacks and prove no resource clear. Hidden or indexed destinations and
alias-conditional helpers remain under their opaque/ABI contracts, rather than
being assigned an invented explicit operand. Ordered integer recipes use the
actual operand's writer kind; raw captured integer transport does not acquire
a shared setter's release. Signal uncertainty remains independently unchanged.
Native stem GET/GET2/KEYAT/VALUEAT results are explicitly unknown writers:
their shared string helper can return successfully when the destination is
already the selected child, before any payload release. They retain possible
finalizer callbacks and prove no unconditional resource clear; their existing
own validation/failure phases remain separate.

The component catalogue separates the object-type stamp from status flags.
`ACOPY` copies status only and preserves the destination object type; whole
COPY/reset effects include the separate `OBJECT_TYPE` domain. Native type
readers and failure diagnostics consume that domain. These are internal effect
masks, with no change to AST inline enums, RXAS operands or RXBIN encoding.

A callback residual is stronger than opaque output. Conditional source-copy
and destination-finalizer resources are captured before the action, while the
unknown callback barrier is final on normal and exceptional prefixes: a native
hook can run between an implementation's scalar and child writes. Logical
attribute count does not certify unused physical capacity empty. Missing ABI
resource checkpoints remain unknown; no inliner body list substitutes for the
shared executor's proof.
The generic semantic `clear` recipe retains a pre-clear resource checkpoint:
three consecutive reference/native-payload/attribute-count captures of the same
active destination, followed by a possible callback with detail
`catalog.callback.clear.active-tree`. Its subject names that destination and
its captured source names the first condition capture. The whole clear follows
the checkpoint, so earlier swaps affect the captured backing and later pointer
capture cannot run past an unproved hook. This active-tree extent does not
require unused attribute capacity empty. Discharge still needs the exact live
generation, absent reference/native payload and empty active tree. The callback
keeps unknown physical observations and exceptional phase; any separate
unrepresented call/result/host obligation retains its residual barrier.

Level L owns a reusable target IR/CFG and flow service for assembler,
inliner, linker cleanup and later RXAS tools. It builds normal, table, signal,
unwind and terminal edges. Signal target analysis may yield one target, a set
or unknown; the fact is available to clients rather than hidden by a
signal-shape recognizer. Policy changes and joins are edge-sensitive.
Inaccuracies in the shared graph/effect layer are repaired there with
independent controls. Existing Level L normal CFG and bounded signal proofs
are a foundation, not yet this complete service.

The canonical policy union covers possible handler destinations; its separate
policy-correlation query records when installation and data paths are not
resolved. Bounded G-style blocks and a compact C bound relay are implemented
under the checked execution profile. Full generated C condition/helper closure
and more general handwritten nested policies remain open. Further path
precision must retain product points for source operation and policy
environment, with data facts on those separate paths. Resource or policy-stack
overflow stays unknown; the current bounded controls do not close signal
coverage.

The same Level L target owner supplies checked register/reference remapping
across instructions, implicit windows, labels, procedures, constants and
metadata. Packed `r/g/a` indices are decoded to typed bank/slot identities
before changing local/global counts. A swap changes the slot-to-storage map and
slot identity; a link changes an active binding. Equal values, equal pointers
and equal register numbers are different facts.

Flow tracks storage identity, value identity and definite availability for
each *published* component of a register, including integer, float, string,
binary, decimal, reference/object and attribute state, string cursor or
position where RXAS can observe it, and individually queryable status flags.
It also tracks active, base and entry bindings, ownership and reference
lifetime. Derived facts carry dependencies: for example, a string converted
from an integer is current only while that integer identity and the conversion
conditions remain unchanged. The same principle covers float conversions.
An individual flag can be proved, invalidated and queried without treating the
whole status word as one opaque value; whole-word operations still observe or
replace all relevant bits. VM-private caches do not become inliner proof inputs
unless an opcode exposes them. Unknown or possible writes never become proven
kills. Effect and flow descriptions must be accurate for the entire supported
RXAS profile and later consumers; optional value proofs can be demand-driven,
with the POC exercising the cleanup queries it uses.

Component-aware liveness marks a private cell's first availability and last
use on each path; return and signal/unwind responses that leave the logical
activation end its lifetime. Resumed signals retain the activation and its
partial-write state. Interference includes every live component, flag, alias,
address, reference, TRACE/metadata observation and complete call window. This supports safe
register reuse, including sharing a cell between nonoverlapping integer and
float values when string conversion, argument exposure and other observations
do not require the old component. Allocation begins with distinct cells and
coalesces only after the shared proof.

For example, one physical cell may carry unrelated integer and float
components when neither conversion depends on the other, its string component
is never observed, and no call or whole-value operation exposes the cell.

## Procedure facts and proofs

Level L also needs a reusable **procedure-summary service** over the same
checked target IR, CFG, point facts and canonical opcode effects. The optional
RXAS target owner supplies RXAS-specific fact domains; they share one canonical
effect interpretation, not a second interpretation of instructions inside the
inliner. Summaries are inferred from executable bodies, including handwritten
RXAS, not asserted by compiler `.meta` or imported inline templates.
Generic worklists, graph traversal and proof lifecycle remain with the
existing Level L owners; RXAS transfers and summary queries belong to the
optional target owner. The canonical numeric-context inventory must separate
reads from writes, including getters and decimal/plugin operations; the
current conversion-derivation read query and combined activation mask are
insufficient for a complete procedure proof.

Each summary distinguishes direct body effects from transitive effects of
exactly bound child calls, and internal activation effects from caller-visible
effects after departure. It records *may*, *must* and *unknown* separately for
each fact family; an unknown signal set does not erase an independently proved
numeric fact.
Its useful fact families are:

| Family | Questions the inliner and later RXAS tools can ask |
|---|---|
| Numeric and other context | Which inherited numeric settings, plugin/locale state or external context are read? Which settings have known values at a point, can change, or need restoration at an exit? A read dependency alone does not require a save; any caller-visible difference must be restored or proved absent or unobservable on that exit. |
| Signal and control | Which named/implicit signals may arise, at what phase and with which partial writes? Which are handled, ignored or resumed internally, and which can leave the activation? How can policy/handler stacks change? Can the body return, fail, terminate or unwind? Inherited and asynchronous delivery remain separate obligations even without authored SIGNAL. |
| Values and registers | Which components and individual flags of arguments, globals and result may be read/written, read before definition or must be overwritten? Which argument-count or dynamic-selector facts hold? Are a return component, type, range or relation to an input known on a particular exit under explicit entry assumptions? |
| Storage and lifetime | Which symbolic argument or exact module/global slot is read, borrowed, rebound, escaped or mutated through aliases? Which references and owned payloads can outlive a path, and which physical frame/TRACE/source observations occur? |
| Calls and external effects | Which exact bodies are called, which targets are unresolved/native/dynamic, and which initializer, allocation, external, plugin, locale or diagnostic effects may occur? What call-window and bounded code/register/metadata growth does expansion require? |

Known values are conditional proof results, not unconditional labels on a
procedure. A summary may say that a defined return component equals an
argument or constant **if** required actual, numeric context, flag, alias,
global-state, signal-policy and path conditions hold. The call-site query
instantiates those premises with the caller's current facts. Normal returns,
resumed signal paths and each abrupt departure retain distinct exit facts;
incompatible joins become unknown. A must-guarantee intersects applicable
normal-return paths and records whether any such path exists, so a no-return
body cannot prove a result by vacuity. Return facts preserve component setters
versus whole-value COPY/MOVE ownership.

Retained real child frames isolate their local context changes from the
parent, while their outward effects remain visible. In particular, a summary
that says caller numeric or handler state is unchanged after a real call
cannot justify omitting restoration after deleting that frame: the callee may
have changed it internally. Caller-owned argument/global mutations are not
restored merely because the frame departs. Module readiness is a call-site
runtime fact, not a permanent property of the callee procedure.

Compute transitive summaries through the exact bound call graph with a
bounded monotone fixed point for recursive components. An unresolved,
dynamically rebound or native child contributes conservative effects unless
an independently checked contract narrows them. A work/precision limit yields
`unknown`, never a false absence or constant. Immutable summary views are
checked against frozen body revision, catalog/flow contract, exact link binding
and recorded entry premises. A body rewrite, provider selection, changed
dependency or premise invalidates affected summaries and callers. Queries
distinguish proved, conditional, unknown, budget limit and invalid inputs.
The service exposes both procedure queries and point/edge proofs so other
Level L RXAS tools can use the same evidence. A summary does not replace the
point proof required to remove a particular swap, flag write, copy or lifetime
operation.

The inliner uses these facts to avoid unnecessary numeric/signal context
operations, choose exact call/return mapping, preserve copies and lifetime
events, specialize known values and flags, simplify branches, and estimate
profitability. Missing optional value facts reduce cleanup, not generic
structural cloning. An unknown activation or unwind obligation still requires
the conservative runtime mechanism or a retained call with a specific reason.

## One call, return and signal-exit model

Normalize every direct, counted, fixed, fused and eligible dynamic call to one
ordered **call recipe**. It records the exact body/module, result pointer,
argument count and ordered actual pointers, immutable entry/base anchors,
mutable active bindings, slot identities, initializer ingress, normal
continuation and abrupt exits. The recipe preserves each existing form's
capture phase, result clear, status/swap preparation, possible failure and
interrupt boundary. Source actuals are not reevaluated. Existing producer
copies remain until alias/ownership proofs remove them; read-only formal use
alone is insufficient when another alias can mutate the actual.

Conservative expansion maps the callee's owned locals to fresh caller cells,
borrowed arguments to captured actuals, and globals to their original module
storage. It remaps the ordinary body generically. Dynamic argument indexing
requires an exact mapped argument-bank operation or a proved finite lowering;
guessing a physical `a` slot is invalid. Retained nested calls must receive
their original window, logical caller and ingress behavior. A return transfers
to the captured result and common continuation, moving only an owned,
unlinked local when the ordinary RET would move it; borrowed/rebound values
copy. Immediate return component setters keep their exact effects.

Real calls and inline bodies share **one semantic activation/departure owner**.
It identifies the logical procedure/module separately from the physical
instruction/constant-pool owner, and owns handler identity and saved stack
marker, inherited `SIGRET`, numeric/plugin context, borrowed versus owned
storage, reference expiry and normal/abrupt window restoration. G's signal
blocks, C's protection/relay layer and handwritten RXAS use that same VM
behavior. An unknown static signal destination is handled by the runtime
context where supported; it need not automatically bar structural cloning.
Physical frame/address observations without an equivalent may still retain
the call with a specific reason. Slight differences in infrastructure-only
resource-pressure failures, such as a removed call-frame allocation, are
acceptable under the agreed preservation policy.

The runtime interface must be **scoped and cheap**. Compile-time remapping and
ordinary instructions perform call preparation and value transfer. A site may
omit runtime context operations only when inherited signal policy, `SIGRET`,
asynchronous unwind, reference lifetime, initializer ingress and mutable
context obligations are all proved absent. When needed, small scoped
operations save only changed state, such as numeric/plugin state or
signal/activation identity; a combined operation is justified only if it is
cheaper and clearer than separate operations.
One VM departure owner covers normal RET, `SIGRET`, signal responses that
leave the activation, branch unwind, cancellation and exit. Ignored, resumed
and CALL-handler signals retain the activation; a returning real handler
closes only itself. The VM's existing copy-on-write signal policy should remain
available. The subsequent concrete ISA review below fixes the scoped
operation and RXBIN 007 gate. Simple proved local leaves still clone with no
entry or departure opcode.

## Modules, globals and lifetime normalization

At link time the selected callee's `gN` refers to that module's private or
public global storage, regardless of the caller's `gN`. The linker can
identify the selected module and global index without new symbol or
inliner-proof metadata. The transformation maps each borrowed global slot to
the provider's worker-local overlay and preserves one owner, initialization,
poison/cycle behavior and teardown. Current RXAS/VM `gN` is relative to the
executing module, so this needs an explicit checked target/runtime binding
representation; merely spelling the callee's `gN` in the caller, or exposing a
private global by public name, is wrong. Untaken calls must not trigger an
initializer early, and retained nested calls need the right logical origin.
The shared Level L `modulemap` now records the exact selected provider owner,
image-local module ID and global slot beside remapped fields. It reports a
foreign private global as unrenderable in ordinary RXAS. The approved
executable binding is scoped `QGLOBBIND`: it binds only a used proxy to an
exact selected image module and provider global slot. `self` is relative to
the executing module; `@N` names a linker-sealed zero-based image ordinal.
The scope restores active and base bindings on every departure. No compiler
proof or private-global exposure is serialized as `.meta`.
A retained nested foreign CALL/SIGCALL uses its ordinary opcode with a typed
qualified procedure operand `@M::P()` naming exact image and provider
procedure ordinals. The loader validates and resolves it before execution.
It is not a public export, and a standalone unsealed body cannot create the
binding.
Code relocation uses a checked operational profile that explicitly accounts
for frame, module, code and debug observations at each moved instruction.
The opcode catalogue supplies this profile to Level L; an incomplete field
remains unknown and cannot prove an effect absent. Constructed profiles test
the same Level L algorithms independently of native catalogue coverage.
No such proof is serialized as RXAS metadata.

Every owned `rN` local starts **dirty** on entry to a normal call or inline
activation. Dirty means that this activation has not defined the component;
recycled physical bytes may exist, but reading them is a source error. It is
distinct from a defined value whose contents are unknown to the analysis.
An `aN` binding borrows the caller's storage and carries that storage's actual
availability; binding does not initialize a dirty actual. `gN` follows its
module-owned storage lifetime rather than a procedure-local entry rule.
Private inline locals have an analysis-only availability marker at activation
and an end marker on every departure; these are not emitted instructions. Code
must not depend on values left by a recycled callee frame. A **standard early
assembler-stage RXAS pass**, whose checked normalization contract is also
required of linker and standalone inputs, diagnoses an entry-dependent read
of a published component
and inserts a clear (an ordinary `NULL` when a whole-cell reset is safe) that
defines the required register state before that observation, including
path-sensitive cases. The warning names the register,
component and source location. If a whole-cell clear would erase a component
already defined on a valid path, the pass must place it earlier on the affected
path or use a component-specific clear; it must never clear borrowed arguments
or globals indiscriminately. The inliner runs on normalized input and reports
any remaining such read as a shared target-analysis defect. The Level L
normalization service implements this warning/clear transaction on an exact
dirty-read proof; uncertainty about a defined value never licenses a clear. Its
assembler-stage integration remains part of this inliner increment. A mere
unknown analysis fact or whole-value copy is not itself proof of an
entry-dependent observation.

Dirty returns can create normalization dependencies between procedures.
The entry-read coverage proof is separate from callback observation
preservation. It must classify every reachable owned-component observation,
including exceptional phases and projected child reads, or publish an explicit
unknown. A candidate clear is licensed by a proved source dirty demand for
that owned base; the candidate flow must then prove complete coverage with no
remaining dirty or unresolved owned demand. A preserved callback alone never
establishes this postcondition, and an empty witness list without complete
coverage is not evidence that no clear is needed.
Privately normalize complete procedures first, then rebuild the shared flow
after a partial canonical stage for callers on that candidate. A complete
per-procedure proof and validated clears establish the normalized postcondition
directly; independent constructed premises are never replaced by canonical
semantics. Incomplete constructed premises retain explicitly. Each changed
iteration must clear a new owned
local; one cumulative budget bounds analysis, remapping and retained state.
Publish only a complete fixed point without remaining dirty observations,
composing original-to-final instruction IDs and warning locations. A failed or
unproved stage leaves the original module and publishes no staged warnings.

## Post-inline cleanup and qualification

In-module expansion runs before assembler optimization. Link-time expansion
must reanalyze only changed callers/edges and run a bounded panel because the
input modules were already optimized:

1. Rebuild and validate normal/signal CFG, storage maps, references and
   module/source metadata; these are correctness duties even with cleanup off.
2. Propagate newly known actuals, presence and individual flags; remove proven
   branches and then dead flag writes/status setup.
3. Remove redundant argument/result copies and caller call-window setup **and
   its reverse swaps** using a whole-region permutation and alias proof, while
   retaining body/nested-call swaps that matter.
4. Recompute component/flag liveness; coalesce private cells and compact
   unreachable code and dead metadata under explicit observation rules.
   Re-encode addresses, globals and task/source seals transactionally.

These are ordinary reusable RXAS optimization rules, not opcode-specific
inliner exceptions. A linker profile may schedule a cheap subset or a special
fast implementation of an equivalent rule if evidence warrants it. The
validated expansion is the fallback if optional cleanup keeps or fails;
cleanup never publishes an unvalidated result. Input/expansion failure leaves
the previous output intact and publication remains atomic.

Focused executable and structural controls should cover one simple
reordered-actual leaf with no outer swaps; alias/snapshot and return MOVE/COPY;
counted, fixed, fused, vararg and nested-call shapes; same-module and
link-selected private globals; multiple returns; G blocks and C
protection/relay including inherited `SIGRET`, cancellation and unwind; early
clear plus warning; flag and dead-branch cleanup; retained unavailable/dynamic
calls with exact reasons; handwritten RXAS; and malformed/failed-output
lifecycle. Independent expected behavior precedes native comparison. Assess
how frequently difficult signal shapes occur, but preserve G/C correctness
where used. The existing plan's INL-STEP06 remains the integration milestone;
this summary does not close it or claim product performance or publication.

## Approved implementation decisions and remaining engineering contracts

Adrian approved the five implementation decisions on 30 October 2026. The
inliner will use one logical call/return/signal activation model, with scoped
runtime operations emitted only for context that the site needs. Ordered actual
register pointers form a checked argument-bank view. Exact linker-selected
bodies may use a checked module-and-slot reference to private worker-local
globals, without exposing them or transporting inliner proofs through metadata.
The canonical opcode database drives ordinary instruction flow and cloning;
database or opcode defects found by integrated testing are repaired at their
owner. Availability, semantic legality and profitability remain separate, with
disabled and force-legal controls. Assembler-local expansion precedes normal
optimization; link-time expansion uses a bounded changed-caller cleanup panel.
Expansion must execute correctly with optional cleanup disabled.

Before changing wire or VM semantics, the implementation review records the
smallest executable operation/operand encoding, RXBIN 007 feature and validation
rule, precise departure/exception behavior, and one ordinary RXAS rendering.
This is an implementation contract review under the approved decisions, not a
new decision about whether these capabilities are required. A material change
to the approved semantics remains a user decision.

### Reviewed RXBIN 007 inline-scope encoding

`RXBIN007_FEATURE_INLINE_SCOPES` (bit 8) gates three semantic operations in
four opcode forms. The assembler parses readable operands into typed `INL1`
and `QGR1` binary constants; the VM decodes bounded fields, never source text
or an opaque inliner proof. The selected linker image seals provider module
and procedure ordinals. These ordinals belong to that immutable image, not
the context's dynamic load order.

```rxas
inlenter r0,helper(),"count=1;args=list(r10);a0=r20;proxies=(r21:1);owned=(r20,r21,r22,r23);prep=none",after_helper
linkargv r21,1
linkargv r21,r7,0
qglobbind r22,"self",0
qglobbind r23,"@2",1
call @2::3()
after_helper:
```

`INLENTER` takes a result cell, exact provider procedure, recipe and normal
continuation label. `count=` is a decimal literal or `rN`; `args=list(rN,...)`
captures ordered actual pointers, while `args=window(rN)` begins at the first
actual slot after a counted call's count/base slot. `a0=rN` is the fresh count
cell. `proxies=(rCell:ordinal,...)` maps original callee argument cells, with
ordinal zero naming `a0`. An ordinal above the captured actual count names an
absent optional formal: its proxy retains a fresh empty owned backing cell;
it never borrows unrelated caller storage. `owned=(rN,...)` lists every fresh
cell whose active/base/identity binding the clone can change, including private
locals, argument/global proxies and LINK/SWAP/UNLINK destinations. Only
comma-separated single register names are accepted; ranges and duplicate
destinations are rejected. The VM snapshots and restores only this sparse set.

`prep=none`, `clear(rN)`, `swap(rN,rM)`, `settp(rN,decimalFlags)` and
`settp(rN,decimalFlags)+swap(rA,rB)` encode only preparation that the original
call performs inside its atomic instruction. `clear` belongs to zero-argument
`CALL R,P`; a prior standalone `NULL` is never folded into it. Optional
`;result=ignored` preserves void `CALL P` without copying a return payload.
Preparation precedes result/count/window capture, target/initializer checks
and scope commit. Any failed checked entry leaves no partial scope/proxy state.
`LINKARGV` requires an active matching scope, accepts an immediate selector or
a register selector plus signed adjustment, and binds only after bounds checks.

`QGLOBBIND` binds a used fresh proxy to the selected provider's worker-local
global. `"self"` is self-relative even if the caller is not image module zero;
`"@N"` is an exact zero-based image-local ordinal sealed against the frozen
selection. Human-readable module names are accepted for linker resolution.
When linking an ordinary assembled module, `rxlink` rebases only its sealed
local `INL1` and self-relative `QGR1` references to the final image ordinal.
Foreign `QGR1` and `QPR1` operands require a validated selected-module
candidate from the linker service; unvalidated foreign seals are rejected.
The existing `P` operand accepts typed `QPR1` source
`@moduleOrdinal::procedureOrdinal()` for retained private CALL/SIGCALL; a
standalone unsealed qualified target is rejected at VM preparation. All
`INL1`, `QGR1` and `QPR1` lengths, versions, flags, counts, ordinals and
duplicate register roles are validated before execution. Scope departure
owns RET, inherited `SIGRET`, handler branch unwind, cancellation, numeric
context and reference cleanup. Caller-owned pointee mutations remain visible.

The explicit assembler driver invokes this same B/L service on a private UTF8
snapshot before ordinary RXAS optimization:

```text
rxas --inline-service llrxinline.rxbin --inline-module logical-name \
     --inline-vm rxvm --inline-runtime library.rxbin \
     --inline-runtime rxfnsl.rxbin [other required runtime images] \
     [--inline-policy default|all-legal|never] [-n] source.rxas
```

The driver passes `--no-cleanup` to the service; normal RXAS optimization owns
post-expansion cleanup, and `-n` disables only that native optimization. It
keeps the original source name as RXBIN module identity and default output
stem, reads service output through a separate private path, and removes
private files on normal success/failure. Cross-module expansion belongs to
the linker's frozen selected-image driver; arbitrary standalone `--body`
paths do not certify its provider identity.

- Specify the smallest runtime activation/context operations and their expected
  per-site cost, including inherited `SIGRET` and all departure paths; check
  their actual cost with focused evidence after an executable design exists.
- Choose the checked representation for dynamic argument banks and
  cross-module borrowed private globals, plus a sealed exact runtime provider
  binding, with handwritten RXAS and RXBIN 007 feature/version rules.
- Complete and independently validate ordered compound effects, signal CFG
  and published component/flag/cursor definitions in the canonical database.
- Specify procedure-summary domains, conservative recursive seeds, widening
  and dependency invalidation before implementing the SCC fixed point.
- Set the exact early clear/warning contract, including partial components and
  source locations; keep physical frame observations as named retention cases.
- Define the assembler/linker driver interfaces, frozen selected-body identity,
  cleanup budget and focused acceptance panel before modifying VM or wire
  semantics.

### Pending decision: guarded provider-equivalent entry (INL2)

**Unapproved proposal; no opcode, feature bit or wire change is authorized.**
The existing [INL-STEP04/05 contract](../PLAN.md#inl-step05-scoped-cleanup-implementation-contract)
owns the implementation and acceptance work. Correct logical diagnostic
walking does not virtualize physical-parent/code observations: an inherited
CALL handler can inspect the parent through METALINKPREG or
METALOADCALLERADDR. Canonical unknown inherited responses therefore remain
unproved. Default HALT alone does not remove execution-observer hooks.

One bounded alternative is a new opt-in entry/recipe version, provisionally
called **INL2**, with a required compatibility feature. INL1 cannot silently
gain a provider-call fallback: handwritten clone bodies can intentionally
differ from that provider. Old runtimes must reject new output explicitly.
The proposed entry keeps the exact sealed provider and one continuation past
the clone. After original preparation and initializer reentry, an O(1) native
effective-policy/observer guard selects the clone or the real provider call.
A miss runs no synthetic prologue and returns past the clone;
preparation, initialization, capture and failure handling execute once.
The first semantic proof variant should instead reserve the real provider
frame/backing once before the guard, then borrow it for the clone on a hit or
activate it for the ordinary provider call on a miss. Retirement stays with
the ordinary call owner. This preserves recycled payload and allocation
checkpoints but retains frame cost; it is a correctness POC, not a speed claim.
A later synthetic fast scope requires exact entry and retirement resource
certificates. Eager frame cleanup changes observable finalizer timing and is
a separate VM lifecycle decision. This whole INL2 variant remains unapproved.

The shared native call owner must preserve each original checkpoint. Fixed
CALL1–4 capture result/actual pointers before initialization; counted calls
capture count/result/window after it. Zero-actual CALL additionally clears
its result before ingress and resolves its result binding afterward. Fused
SWAP/status/clear preparation stays in its original order. Argument list versus
window representation alone cannot encode these distinctions. Allocation and
validation failures retain their original ordering and caller recovery
continuation. There is exactly one original CALL semantic mailbox claim,
after the chosen activation commits; the guard adds no early pre-preparation
poll or duplicate asynchronous delivery.

The runtime guard must exclude returning physical-frame observers, not merely
SIGCALL: reentrant host output/SAY/ADDRESS/native hooks, interrupt reentry and
debug callbacks also need a checked harmless premise or a guard miss. The
initial policy proposal admits only effective IGNORE/HALT/SILENT_HALT with
breakpoint IGNORE and no interrupt invocation. A shared static facet must also
prove clone layout/source/relocation and direct/transitive policy invariance
through canonical actions and exact provider bundles. Physical/code observers,
unknown callbacks or policy-changing descendants refuse the fast path;
ordinary scalar observations are not a body admission veto. Unknown CFG paths
remain represented, including the ordinary slow call; the guard is not CFG
pruning or a new opcode evaluator.

The first correctness POC pays one bounded entry guard/branch, maintained
native policy/observer classification and a real provider frame/backing on
both paths; the guard hit borrows that backing, and the miss activates it
without duplicate setup. A later certified synthetic scope may remove some
frame work. Neither variant has a performance result. A minimal decision
panel compares fast, guarded
fallback and original fixed/count/zero-result calls, initializer failure and
ignored recovery, preparation/result aliases, sparse mailbox delivery,
SIGCALL physical-parent inspection, reentrant host hooks, private globals,
normal/abrupt cleanup and old-runtime refusal. Keep one representative simple
output assertion and targeted native lifetime/fault checks.

The alternatives remain explicit: a checked caller-declared closed-root
execution contract can prove a smaller observer-free invocation without
changing INL1; it cannot be inferred from a manifest or module text. Full
logical-frame virtualization could support physical-parent/code observers
more generally, but must define borrowed register/global/argument identity,
source addresses and lifetime throughout nested handlers. That is a larger
VM owner contract. Adrian must select the public ISA/compatibility direction
before any INL2 product edit.

The canonical callback inventory is independent of intrinsic signal precision.
The generated `callbackobligations` mask combines reviewed inventory completeness
with input, output, retained-capacity, host, selection, invocation and ABI routes.
Zero is a complete no-hook fact; unknown inventory remains an explicit residual.
A callback can request an active-context interrupt, so canonical CFGs retain an
operation-caused unknown callback path even when the intrinsic instruction has
no signals. Native analysis keeps the raw intrinsic contract separate from that
effective execution control and does not infer clean backing from local ENTRY.
The callback boundary has a distinct identity from an intrinsic unknown-signal
boundary. A complete resource-only callback (input, output or retained capacity)
cannot rewrite the suspended frame's handler table before interrupt dispatch;
the graph retains that structural policy on normal callback return while still
representing the queued interrupt and any delivered handler path. Unknown,
host, selection, invocation, ABI and departure routes retain unknown policy
and incomplete local coverage. A later signal producer reached after such a
nonresource callback still needs path-reachable policy propagation; a
whole-function unknown flag is not the intended proof. The canonical graph
now computes that reachability over staged successors to a bounded fixed
point, including loop backedges and new handler/resumption edges. Unreachable
nonresource callback code does not by itself invalidate callback-policy
completeness. Other older aggregate completeness cases, including disconnected
scope-entry `SIGRET` and authored policy rows, still require a separate
reachable-row projection and remain explicit limits.
Native incoming-resource facts may establish no hook only from exact incoming
absent payload/zero-active-count leaves, and ordered normal postwrites target
the physical receiver captured before a callback. Those postwrites do not
retroactively remove the earlier callback or prove a resumed path.
The native exact-entry policy view is a separate, caller-declared proof over an
ordinary activation. It keeps the raw graph unchanged, separates intrinsic
failure from callback-after-body control, and admits HALT/IGNORE edges only
when the sealed entry policy and later structural writes justify them. A
handled response still havocs state; an unknown or handler-entry activation
retains the raw conservative graph. `BPOFF` changes a pending BREAKPOINT bit,
not the installed handler table: catalog signal-property bit 32 describes that
pending effect, while bit 8 marks structural policy writes.
No intrinsic UNKNOWN signal row is made precise by this foundation increment.
