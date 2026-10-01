# Optimization architecture and tool-boundary audit

Status: audit complete; imported-inline and status-bit boundaries accepted by
Adrian on 2026-09-18. Entry-alias repair is implemented, locally qualified and
published to develop in `f7a8b08c1`; combined-head hosted checks are tracked in
the [selected defect batch](../../qa/beta3-defect-batch-2026-09-18/README.md).
Requested by Adrian on 2026-09-18. This records
existing behavior and proposals for review; it does not approve new language,
assembly, ABI, optimization or architecture contracts beyond the explicit
acceptance below.

## Accepted follow-up decisions — 18 September 2026

- **OPT-BOUNDARY-02 accepted and disposition closed:** retain imported inlining
  as an RXC-owned compiler-IR concern. Its template/executable coherence
  obligation remains documented; it is not an RXAS proof input. No removal,
  disabling, or new integrity mechanism is requested. Do not reopen this
  architectural choice merely because the retained inconsistency probe exists.
- **OPT-BOUNDARY-03 accepted:** retain the documented RXAS status-bit assertion
  contract as part of the public instruction semantics. No new restrictions or
  certificate design are selected.
- Preserve RXC's structured AST-rewrite support, including its importance for
  Level C; favor consistent extension of that structure. No RXC-to-RXAS proof
  transport or AST framework replacement is selected.
- Address OPT-BOUNDARY-01 as a correctness defect under the existing calling
  convention. Review RXAS maintainability and VM fusion extensibility; new
  performance mechanisms remain proposals until supported and selected.

## OPT-BOUNDARY-01 repair plan

**Vision:** valid assembly must preserve results when incoming arguments share
storage, including aliases reached through explicit links and globals. Correct
the shared component-value model so all its consumers benefit; retain proved
private-local simplifications. Preserve the existing ISA, calling convention,
RXC framework and accepted metadata boundaries.

- [x] **ALIAS-AC-01:** original optimized/unoptimized runtime reproducer both
  produce `42`; retain a permanent CTest regression, not only the audit probe.
- [x] **ALIAS-AC-02:** cover argument/argument and argument/global may-alias
  writes, explicit linked aliases, joins and calls in focused analysis/runtime
  checks. Unknown aliasing must invalidate affected component facts.
- [x] **ALIAS-AC-03:** retain exact alias transfer and independent private-local
  facts; existing storage, repetition, copy/guard and flow regressions pass.
- [x] **ALIAS-AC-04:** affected tools build, the normal RXAS correctness suite
  passes, and the code/docs/evidence describe the actual scope and limits.

1. **ALIAS-STEP-01 — complete** (AC-01/02): retain the current failure and add permanent
   regressions before editing production logic.
2. **ALIAS-STEP-02 — complete** (AC-02/03): keep exact StorageIds distinct, but distinguish
   exact identity from possible aliasing when invalidating component facts.
   Apply the correction in shared SSA handling, rather than disabling just the
   repeated-conversion consumer or treating all incoming arguments as identical.
3. **ALIAS-STEP-03 — complete** (AC-03/04): run the new tests and existing focused/normal
   RXAS suite; inspect any lost valid optimization before widening the change.
4. **ALIAS-STEP-04 — complete** (AC-04): record correction evidence and update the live
   finding status. This is correctness restoration, not a selected performance
   enhancement or new architecture; no broad performance programme is opened.

Repair evidence: [local repair record](https://github.com/adesutherland/CREXX/blob/108257c3d5ecfed961471fc79fa4c955dfd4eb7c/docs/qa/optimization-boundary-audit-2026-09-18/repair/README.md).
The original probe now prints `42` in both modes. All 102 affected Debug
RXAS/flow/runtime tests pass, including eight permanent alias/control cases.
The correction adds incoming-base may-alias invalidation to shared SSA value
queries and call-window handling, while retaining exact identities and the
existing dynamic/reference model. A first over-conservative variant lost a
valid attribute-read optimization; it was narrowed before qualification.

## Vision and intended outcome

Establish a source-grounded account of optimization in RXC, RXAS and RXVM,
including RXBIN serialization, RXLINK and library imports where they carry
information across those boundaries. Determine whether correctness depends on
compiler assertions unavailable from the assembly/bytecode contract, especially
for handwritten RXAS or another compiler. Distinguish facts reconstructed by
the receiving tool, trusted declarations, optional hints, observable metadata,
and private internal analysis. Audit the documentation and identify explicit
boundary decisions for Adrian without implementing an architectural change.

Reviewed source baseline: `15c8a3ba42009ab8b5a9b447aa8c06ce86b9b392` on
`develop`. Existing local roadmap, release and KeyAccess documentation edits
are preserved and are not optimizer changes.

## Checkable acceptance criteria

- [x] **AUD-AC-01:** map the current optimization stages, their owners and
  analysis scope to source entry points and architecture documentation.
- [x] **AUD-AC-02:** inventory optimization-relevant cross-tool information,
  including representation, producer, consumer, validation, persistence,
  missing/invalid behavior, and availability to independent assembly producers.
- [x] **AUD-AC-03:** trace RXAS proof premises to their actual origin, including
  call/alias/signal/metadata handling; distinguish shared opcode semantics from
  compiler assertions and identify any implicit compiler-shape assumptions.
- [x] **AUD-AC-04:** retain focused executable checks where useful for assembly
  independence and metadata behavior, with exact commands/results and limits.
  A review or passing probe is not a general soundness proof.
- [x] **AUD-AC-05:** reconcile the documented architecture with the inspected
  implementation, report concrete gaps/risks and propose explicit ownership
  boundaries and decisions for Adrian. Preserve unresolved questions visibly.

## Audit steps

1. **AUD-STEP-01 — complete** (AC-01): read the tool architecture/performance authorities
   and identify the stage entry points. No production edit is selected.
2. **AUD-STEP-02 — complete** (AC-02/03; depends on STEP-01): follow emitters, RXAS parser
   and proof consumers, serialization/disassembly/linking, imported inline
   templates and VM preparation/dispatch. Build the information-flow inventory.
3. **AUD-STEP-03 — complete** (AC-04; depends on STEP-02): select small decisive checks
   against the available build, retain inputs/results, and investigate any
   mismatch without expanding into performance tuning or broad QA.
4. **AUD-STEP-04 — complete** (AC-01–05; depends on STEP-02/03): complete this report,
   add documentation pointers where needed, and present findings and proposed
   contracts. Architectural changes remain a separate maintainer decision.

## Verdict

The concern is justified. A tool boundary should describe every semantic
obligation, and valid assembly should not depend on its having been generated
by RXC. However, the specific suspected route is not what the inspected code
does: **RXAS does not consume RXC's formal read-only/escape/callable summaries
as premises for its procedure proofs.** Those summaries travel inside explicit
`.meta ... ".inline"` assembly records and RXBIN metadata for a later RXC
import. RXAS preserves them as opaque strings.

The audit found two concrete concerns:

1. **OPT-BOUNDARY-01 — reproduced P1 defect, repaired and published:** baseline RXAS can wrongly treat
   incoming argument registers as independent storage. A valid handwritten
   assembly example returns `41` with optimization and `42` with `rxas -n`.
   There is no compiler, inline template or metadata in that failing path.
2. **OPT-BOUNDARY-02 — accepted RXC-owned boundary:** a library's compiler inline
   template and executable procedure are separate representations. The importer
   does not establish that they compute the same thing. Deliberately editing
   only executable `ret 42` to `ret 99` yields `42` when inlined and `99` when
   called. This demonstrates a trust obligation, not evidence that an ordinary
   unmodified compiler build produces inconsistent libraries.

There is also an existing explicit trusted-value contract: RXAS instructions
may assert Unicode normalization certificates. That is documented executable
state, not hidden compiler-to-assembler proof data, but it belongs in any
approval inventory of trusted assertions.

The original audit made no production change. Its follow-up repairs
OPT-BOUNDARY-01 in the existing SSA model; no ABI or language change was made.
OPT-BOUNDARY-02 and OPT-BOUNDARY-03 are explicitly accepted as documented above.
The original audit acceptance criteria concern the review itself; the separate
ALIAS criteria record the published correction.

## 1. Actual stage ownership

| Stage | Current optimization role and scope | Boundary result |
| --- | --- | --- |
| RXC: source/typed AST | Constant folding/propagation, source-level inlining, argument copy elision, receiver/temporary ownership, typed-flow transformations, SELECT dispatch lowering and register placement. Inlining includes supported imported templates. | Ordinary RXAS instructions and explicit directives/metadata. Private source analyses are not delivered to RXAS's proof engine. |
| RXAS: local scan | A bounded 100-record peephole handles exact mechanical rewrites, compact/fused instruction selection and local patterns. | A procedure record stream containing instructions and observable metadata. |
| RXAS: procedure analysis | Builds CFG, rooted reachability, dominance, requested loop analyses, signal/effect state, symbolic storage and component SSA/use facts. Performs repeated-conversion/load/copy simplification, forwarding, branch simplification, linked-read reuse and selected loop reuse. | Rewritten ordinary instructions plus surviving metadata; proof objects are temporary and not serialized. |
| RXLINK | Resolves modules, contracts, providers and semantic graph identities; shares/deduplicates pools, remaps references and optionally strips metadata. | Canonical linked RXBIN and runtime contracts. It is not a cross-procedure code optimizer consuming RXC effect summaries. |
| RXVM | Prepares a private execution image, resolves callable pointers, chooses switch/threaded dispatch and recognizes bounded instruction fusions. Runtime type/dispatch caches, string certificates/caches and representation fast paths avoid repeated work. | Process-local state; canonical public instructions remain in the serialized image. No transported RXAS SSA proof is required. |

RXC inlining really does enlarge the caller procedure subsequently analyzed by
RXAS. This matches Adrian's proposed division. RXAS need not recover the original
source call or understand how the larger procedure was produced.

RXC still needs its own proofs for transformations it performs before assembly
exists. For example, preserving a source-level by-value argument while eliding
a copy or inlining a method is RXC's responsibility. RXAS can prove properties
of the resulting machine program; it cannot certify that this program preserves
the meaning of source it never receives.

Source anchors:

- `compiler/rxcpmain.c:918–953`: argument marking, AST optimization, dispatch
  lowering and typed-flow overlay before emission.
- `compiler/rxcp_opt.c:2462–2593`: read-only by-value copy elision and AST
  optimization/inlining. `compiler/rxcp_emit_reg.c:1042` consumes private
  argument facts by choosing emitted storage, not a read-only directive.
- `assembler/rxas_opt.c:1943–2130`, `assembler/rxasassm.c:2482–2621`:
  queueing, no-opt path and procedure flush boundaries.
- `assembler/rxas_flow.c:4373–4510` and
  `assembler/rxas_flow_pass.c:19–52`: immutable analysis epochs, consumer
  ownership, capability census and rewrite routing.
- `interpreter/rxvmintp.c:2683–2741,5376–5456`: structural fusion recognition
  and private execution-image construction.
- `docs/ai-context/RXVM_INTERPRETER.md`, dispatch, private-fusion and interface
  dispatch sections: runtime guards, representation and caching contracts.

## 2. Information crossing the boundaries

| Information | Representation and consumer | Validation/trust and persistence | Independent producer / absence |
| --- | --- | --- | --- |
| Instructions, labels, register classes, procedure/global sizes, exposures | RXAS text; RXAS parser, optimizer, emitter; canonical RXBIN and RXVM | Syntax/operand/reference checks plus ISA and calling-convention semantics. This is the primary machine contract. | Handwritten RXAS or another compiler can supply it directly. Required declarations must still be valid. |
| Literal constants and jump tables | `.const`, literal operands, `.jtable`/`.jcase`; RXAS and RXVM | Explicit machine data; table targets and packed selection are assembler-owned. A named constant is not a promise that a global or parameter stays constant. | Public RXAS syntax; no RXC-only knowledge required. |
| Function/type/class/interface/provider/initializer/task contracts | Explicit `.meta` records, RXBIN typed metadata and linked semantic graph | Structural/signature/binding checks are distributed across assembler, graph/linker, importer and loader. They describe runtime/ABI contracts, not RXAS purity or no-alias proofs. Declaring a signature does not prove body semantics. | Needed for the facilities used. Omitting required runtime contracts is not equivalent to removing optional optimizer hints. |
| Register names/types, source steps and TRACE observations | Register `.meta`, `.srcstep`, `.traceevent`; RXAS use analysis and debugging/runtime tooling | RXAS models register metadata as all-component observations and TRACE as typed reads. These can constrain deletion/motion; declared source types do not provide a general invariant that a register always has that value/component. | Source-debug records can be absent in handwritten assembly. Their absence changes available diagnostics/TRACE, and may permit more optimization. |
| Imported inline body and callable summary | `.meta "callable"=".inline" "I6;..."` or `"I7;..."`; `META_INLINE` in RXBIN | RXAS accepts a string and stores it opaquely. Later RXC checks supported schema, declaration/body eligibility and summary agreement against the annotated AST. No executable-body equivalence check. Libraries may preserve it; final RXLink output strips it by default, `-i` preserves it. | Optional compiler-specific extension. Other producers can omit it and retain ordinary calls. Missing/unsupported/mismatched eligible-template evidence declines imported inlining. This is not required for RXAS optimization or VM execution. |
| Call-ABI status flags | Ordinary status instructions and runtime values (`REGTP_VAL`, `REGTP_NOTSYM`) | Shared executable calling convention; VM/compiler-band rules are in `rxflags.h`. They are not `.readonly` or `.noalias` promises. | Any assembly producer using that ABI must follow it. |
| Normalization certificates | Language-band bits set/read by explicit status instructions and trusted runtime/library algorithms | Positive facts about current complete string bytes. Missing bits mean unknown; exact copies preserve them and content mutation invalidates them. Trusted RXAS can assert a false bit, so presence is a semantic obligation. | Public, documented machine-level assertion capability; not a secret optimizer file. Ordinary source flag views cannot write this band. |
| Opcode effect/signal tables | `rxops.h`, `rxopeffects.h`, `rxopsignals.h`, compiled into tool implementations | Shared ISA descriptions maintained against VM handlers. These are the so-called semantic "sidecars", not per-program output from RXC. | Identical for all assembly producers. Incorrect table/consumer semantics are toolchain defects, not producer obligations. |
| AST overlays, RXAS CFG/SSA/proof epochs, VM caches | Tool-private memory | Reconstructed/owned by each stage, not serialized as proofs for the next one. | An independent producer need not reproduce these internals. |
| Project-dependency snapshot | Optional `rxc --project-dependencies` file | Build/freshness input to project tooling and RXC checking; not read as RXAS optimization evidence. | Irrelevant to direct handwritten assembly. |

Inspection of RXAS's CLI/context, input path and grammar found no additional
per-program proof file or direct RXC analysis-object channel. The assembler
library can also be invoked on an input buffer; that changes the input transport,
not the facts available to the optimizer. This is a source-review conclusion,
not a system-call sandbox proof for every possible embedding.

Relevant implementation anchors include `assembler/rxaslib.c:190–262`,
`assembler/rxasgrmr.y:75–165`, `assembler/rxasassm.c:3003–3018`,
`assembler/rxas_flow_use.c:588–603`, `binutils/rxbin007.c` (`META_INLINE` cases),
`disassembler/rxdadism.c:227`, `linker/rxlinkmain.c:1837,2649`,
`binutils/include/rxflags.h:31–65`, and
`compiler/rxcpmain.c:166–167,1004–1006`.

## 3. What RXAS proofs actually rely on

RXAS reconstructs control flow and value/storage/effect facts from the queued
machine records. An opcode's read/write sets, definite kills, implicit operands,
alias/lifetime behavior, failure phase and signal continuations come from the
shared opcode contract. At this baseline the metadata consistency executable
reports **660 slots: 590 classified source, 0 conservative source, 67 reserved
and 3 internal**. Table consistency testing is not a formal proof that every
VM handler matches its entry or every proof consumer is sound.

The proof service distinguishes register names from storage mappings and
separate integer/string/float/binary/etc. components. It includes signals and
observations because matching numeric output alone is not enough. For example,
the optimizer explicitly refrains from general dead-result deletion where a
seemingly dead scalar write could release native/reference state.

Calls are not assumed pure. Argument windows name caller-owned storage; explicit
argument forms and reference/external effects constrain proofs. A known range
count is reconstructed from instruction value facts; an unknown count widens
the considered local range. Unaffected, unaliased private local values can
sometimes survive a call. Globals and arguments are not declared universally
constant. The global-mutation control in this audit correctly preserves the
second conversion across a callee write.

Unknown control flow, unavailable analyses, stale graph epochs, allocation
failure or work-budget exhaustion cannot supply a successful proof; applicable
routes decline. The local/CFG routes can remain available when semantic analysis
is too large. These are good safeguards, but they do not fix an incorrectly
modeled relation between otherwise valid entry values.

The important counterexample is below. An optimizer may call a structure
"StorageId" without having proved that two such IDs cannot denote the same
runtime value. Arguments can enter a procedure already aliased. A per-procedure
analysis cannot discover the caller's exact relationship from the callee body
alone; absent a supported explicit contract it must preserve possible aliasing.

## 4. Findings

### OPT-BOUNDARY-01: incoming argument aliasing miscompiles valid RXAS

**Status: repaired in `f7a8b08c1` and published to `develop`.** The following
description and source line references describe the original baseline defect.

The retained complete program links two caller slots to the same value, then
passes both to this procedure:

```rxas
aliased_args() .locals=0
    itos a1
    inc a2
    itos a1
    ret a1
```

The input integer is `41`. Since `a1` and `a2` alias, the increment changes the
integer seen through `a1` to `42`. Its second `itos` must refresh the string.
`rxas -n` retains that instruction and the caller prints `42`. Normal RXAS
deletes it and the caller prints the stale string `41`.

The debug log identifies the responsible path explicitly:

```text
NR27 accept procedure=aliased_args candidate=2:ITOS_REG reason=redundant-component-ssa-conversion redirects=0
PERF3 flow-proof-repetition generator=0 candidate=2 proved=1 reason=proved storage=1 source=4 candidate-source=4 result=0 candidate-result=0 effect-class=0 generator-effect=0 candidate-effect=0
```

Source trace: `flow_ssa_resolve_storage()` initially assigns register-based
storage identities (`assembler/rxas_flow_ssa.c:2317–2327`), with base nodes
created at `2990–3005`. The write through `a2` does not invalidate the integer
fact associated with `a1`. The repeated-derivation consumer then sees unchanged
source/result identities. Its additional external/reference check at
`assembler/rxas_flow_proof.c:2032–2059` runs only when reference-effect identities
differ; ordinary `inc` does not make that happen in this example.

This trace supports an entry-alias modeling defect. It does not establish that
only this consumer, only argument/argument pairs, or only this opcode pattern
is affected. Argument/global, reference-derived aliases, copy/constant/guard
consumers and hidden lifetime effects need a bounded impact review when the
repair is selected. No no-alias restriction on this valid calling sequence was
found in the documented machine contract. Adding one merely to justify the
existing proof would be a new architecture/ABI decision.

**Disposition:** the shared SSA model now distinguishes exact storage identity
from additional possible aliases of incoming argument/global bases, including
phis. Fresh and cached component queries use the same write-resolution helper.
The permanent runtime matrix covers aliased and distinct arguments, detached
locals, links, joins, repeated constants, globals and calls. Local qualification
is complete; the wider alias model is not claimed formally proved. The
performance roadmap points here as the authoritative finding record.

### OPT-BOUNDARY-02: imported templates are a trusted second implementation

**Status: accepted by Adrian, 2026-09-18; disposition closed.** Retain the
RXC-only inline transport and its documented producer-coherence obligation.
The deliberately inconsistent-artifact probe below remains explanatory
evidence, not an open request to remove or redesign the mechanism.

The inline payload contains much more than a hint: typed AST, scopes, symbols,
source provenance, dependency/signature data and formal/control/result facts.
It uses internal compiler node/type representations. I7 extends the supported
receiver-storage and exact residual-member transport beyond I6.

The importing compiler performs substantial validation. It validates the
declaration and template's supported shape, rebuilds a callable summary and
compares it with the transported summary. Version, result-shape, formal-summary
and receiver-summary negative tests all passed in this audit. Unsupported
evidence leaves an ordinary call; legacy receiver-unknown handling is explicitly
restricted in code.

Nevertheless, this is **consistency checking of annotated compiler IR**, not a
proof from the executable machine procedure. In particular:

- `compiler/rxcp_inline_payload.c:2798–2808` restores transported symbol
  `readUsage`/`writeUsage` annotations.
- `compiler/rxcp_inline_analysis.c:142–155,212–225` traverses those annotations
  to reconstruct formal read/write/read-only facts.
- `compiler/rxcp_inline_payload.c:3030–3146` checks eligibility, declaration and
  summary agreement, then attaches the imported template.
- RXAS stores the string without interpreting those facts. Neither this
  attachment nor storage establishes template/executable equivalence.

| Library input | RXC optimized caller | RXC `-n` caller |
| --- | --- | --- |
| Generated body `ret 42`, matching template | `42` | `42` |
| Only assembly body changed to `ret 99`, template retained | `42` | `99` |
| Same `ret 99` body, `.inline` record removed | `99` | `99` |

Library source and assembly were absent from import lookup; these were binary
imports. All callers were assembled, linked and executed. Final linking strips
the template but cannot undo a choice already made when compiling a caller.

A checksum created only after an inconsistent body/template pair has already
been assembled would bind the inconsistency, not detect it. Versioning and
artifact identity can help prevent accidental staleness, but do not constitute
a semantic equivalence proof. The production provenance and editing contract
must therefore be explicit if this extension is retained.

### OPT-BOUNDARY-03: trusted certificates need an explicit place in the contract

**Status: existing documented capability accepted by Adrian, 2026-09-18.**

The language-band normalization certificates are real assertions. For example,
`lib/rxfnsg/unicode/tools/unicode_d.crexx:1080–1086` reads a certificate and can
return the source without running normalization; `1144–1147` sets certificates
on a completed result. Trusted assembly may also set these bits through status
instructions. RXAS does not prove the string content satisfies the assertion.

The contract is already described in `binutils/include/rxflags.h`,
`docs/ai-context/CREXX_UNICODE.md`, and the VM's register-status section.
Absence is safe/unknown; a false positive assertion is not harmless. This is
different from a source type annotation and from an ignored profitability hint.
No new certificate or producer authority was approved in this audit.

### OPT-BOUNDARY-04: documentation understated the boundaries

**Status: factual corrections made; proposed policy remains open.**

The detailed implementation documents contain much useful material, but did not
give a reliable concise cross-tool view:

- The main architecture's assembler stage omitted procedure optimization.
  The RXAS pipeline foregrounded buffering/backpatching without the local and
  procedure optimizer stages. Both summaries now describe their ownership.
- Main architecture/RXAS metadata descriptions said I6 where code and detailed
  inlining design support I6 and I7. They now identify both.
- "Sidecar" could be mistaken for producer-supplied proof data. RXAS guidance
  now explicitly says these are shared, compiled-in opcode tables.
- RXAS guidance still said production consumers did not request loop analysis.
  H01/H02 do; the description now matches the capability census.
- The fixed opcode inventory count was stale; the guide now names the dated
  measured count and the executable that supplies it.
- Inline-summary descriptions did not clearly distinguish rechecking annotated
  AST data from proving correspondence with executable code. The architecture,
  RXAS and detailed inlining guide now state that limitation.

The alias repair and its qualification limits are linked from the main
architecture and live performance roadmap. They are not a general optimizer
soundness verdict.

## 5. Proposed boundary policy for Adrian's decision

The accepted RXC-inline/status-bit decisions above are authoritative. The
remaining recommendations do not authorize new ISA or architecture work:

1. **RXC owns source semantics and source-level rewrites.** It must emit a
   complete program using documented machine semantics. Inlining stays here;
   source copy/receiver proofs remain private unless an explicit extension is
   deliberately exported.
2. **RXAS owns every proof needed for its own machine transformations.** Its
   premises are the public assembly contract and shared ISA semantics. Incoming
   arguments and exposed/reference-accessible storage may alias unless a
   separately approved contract establishes otherwise. Optimization must not
   infer permission from an RXC-shaped name or temporary pattern.
3. **RXBIN is a documented program representation, including required runtime
   contracts.** Metadata is not one undifferentiated bucket of optional hints:
   runtime contracts, observations, trusted assertions and compiler extensions
   need separate descriptions and omission/invalid-input behavior.
4. **RXVM owns guarded execution specialization.** Structural recognition must
   work for any producer's valid bytecode; runtime guards/cache invalidation and
   observable fallback preserve the canonical instruction semantics. Promoting
   private fusions into public instructions would be a separate ISA decision.
5. **Accepted: retain cross-file compiler AST transport.** It remains an
   optional, versioned RXC extension with a documented producer-coherence
   obligation and no RXAS proof role. Preserve and consistently extend the
   compiler's AST-rewrite support, including Level C.
6. **Approve any new const/no-alias/purity declaration individually.** Specify
   scope, producer obligation, consumers, verification versus trust, behavior
   when absent/false/unknown, serialization and handwritten-producer support.
   A genuinely optional hint can change cost decisions only; if lying changes
   program output, it is a semantic contract and must be described as such.

The answer to "can RXAS do all the proofs?" is: **it should do all the proofs
for its own rewrites, but not all source-level proofs, and not infer unavailable
caller facts.** Some optimizations must decline at an unconstrained procedure
boundary. Better whole-program analysis or an explicit approved ABI declaration
could provide more information; invisible assumptions cannot.

## 6. Evidence, limitations and handoff

Permanent evidence is in
[`docs/qa/optimization-boundary-audit-2026-09-18`](../../qa/optimization-boundary-audit-2026-09-18/README.md):
source revision and tool hashes, build log, full 23-test CTest log, exact probe
commands/results, source/assembly fixtures, both disassemblies and the accepted
but incorrect proof diagnostic. The probe harness can recreate the examples in
a temporary directory without changing production files.

Observed results:

- Focused Debug tools built successfully at the recorded baseline.
- All 23 selected existing tests passed (metadata/graph, calls/signals,
  storage, observations, conversions, guards, loop reuse and malformed imported
  summary fallback).
- Existing metadata-free handwritten runtime fixture passed opt/no-opt, with
  `itos` count reduced from 14 to 9.
- Cross-call global mutation retained the necessary conversion and printed
  `42` in both modes.
- New entry-argument alias fixture **failed semantic equivalence**: optimized
  `41`, unoptimized `42`. A green existing suite did not cover this case.
- Deliberately inconsistent inline-template/body fixture diverged as recorded
  above; omitting the template restored ordinary-call agreement.
- Disassembly preserved library inline metadata, `rxlink -i` preserved it,
  and normal final linking removed it in the checked examples.

The results immediately above describe the pre-repair audit baseline. The
follow-up repair's 102-test qualification and original-probe correction are
retained separately so the failing evidence is not overwritten.

This is an architecture/trust audit with focused executable evidence, not a
formal proof of every optimization or platform. Execution checks used macOS
ARM64 Debug and the selected `rxvm` switch engine. No broad sanitizer or release
matrix was run; no benchmark claim was made. The alias repair is locally
qualified within the stated scope. Other consumers of transported compiler annotations were
not exhaustively adversarially tested.

Handoff: AUD-AC-01–05 and local ALIAS-AC-01–04 are complete. The repair is
published in `f7a8b08c1`; the selected defect-batch execution record tracks
combined-head hosted qualification. OPT-BOUNDARY-02/03 are accepted and must not be reopened
without new evidence or direction. The maintainability/fusion-ownership and
original-quickening follow-up is recorded in
[the follow-up review](optimization-maintainability-and-fusion-ownership.md);
its ISA/architecture decisions remain unapproved. It separates static R1/R2
fusion from the rejected Q7 adaptive prototype, the accepted direct MKREF
improvement and today's runtime interface caches. Runtime learning is a valid
VM responsibility; no new adaptive client or general framework is selected.
The existing fusions' removal, migration or bounded retention requires a
current benefit/maintenance decision, not an inference from the historical
absence of missed exact R1/R2 patterns.
