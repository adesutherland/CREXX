# Level L target-toolchain research

**Status: standalone inliner architecture under review; implementation pending.**
Adrian has requested a standalone, effective RXAS inliner proof of concept and,
first, an architecture and mapping proposal. The [inliner contract](doc/inlining.md)
and INL criteria/steps below own that selected outcome within this single plan.
The wider assembler/reassembler/linker proposal remains proposed; all TGT
acceptance criteria remain open. This design increment includes no product
implementation, native replacement or publication.

## Vision and boundaries

The wider proposal is four cooperating tools in Level B with Level L
infrastructure:
a nonoptimizing RXAS assembler, an RXBIN reader/writer (reassembler) ready for
future middle passes, a module linker, and a common RXAS inliner. A naive,
semantics-preserving instruction expander is its correctness baseline; the
selected standalone outcome also includes proved transport simplification and
bounded post-expansion cleanup.
One owned target-module/image model carries instructions, records, references,
module boundaries and provenance through every service. Expose callable services
and thin CLI adapters; clients never capture another command's stdout.

This is a demanding final validation/debugging opportunity for Level L at this
stage: grammar/source/diagnostics, owned structures, transformation, validation
and atomic publication must work for real target tools and independent clients.
It does not establish universal Level L completion or close the separate
[parent research](../PLAN.md) or [C compiler](../compilers/PLAN.md) criteria.
Current develop baseline QA/publication remains independent of this proposal.

User requests execute B/L algorithms, without native assembler/linker encoding
backends, native source-compiler services, subprocess fallbacks or provider
bridges to those algorithms. Native B bootstrap, prepared canonical-schema
extraction, native VM execution and independent native test oracles are allowed.
Retain current product tools and defaults. No new ISA, runtime, source-language
semantics, platform, search/autoload policy or production replacement is implied.
The requested design also considers small linking, metadata, ABI/ISA or
language-contract additions where they materially simplify correct inlining;
the architecture compares them with the current-contract baseline. Their design
is in scope; adopting/implementing an extension requires its explicit decision.
Research has no formal performance-completion gate; correctness, bounded work,
code quality and actual sanitizer obligations retain their existing owners.

## Existing services and ownership

`rxfnsl-rxas` supplies `llrxascatalog` and the single-function checked records,
register assignment and renderer in `llrxas.program`; it is not an assembler,
module model or RXBIN writer. Its canonical view is extracted from existing
opcode/effect/signal owners. Extend that checked view for missing wire operand
domains and feature rules rather than hand-copying native numeric identities.
`llbinmeta` supplies bounded 007 envelope/LZSS and selected metadata reading;
its callable projection omits executable/graph data and is not a lossless codec.

Use actual L source, lexer/parser, owned AST/IR, transform, validation, diagnostic
and writer services/representations through their APIs, not copied frameworks.
Use existing `llir` ownership/freeze/revision/atomic-result and `llirgraph` services
where applicable. `llflow` models source-AST flow, not a machine CFG. Module/body,
typed metadata, relocation and target CFG/storage/base-binding/exception evidence
are deliberate integration deliverables, not assumed existing capabilities.
Missing shared capabilities and generic defects are repaired/extended at their
L owners with independent consumers; target defects stay with the target owner.
No private arena/transaction/graph framework or product-to-research edge is allowed.

Reusable target-model, codec, relocation and expansion facilities belong to the
optional L target owner. Research `common/` holds adapters/profile/orchestration;
`rxbin/`, `assembler/`, `linker/`, `inlining/`, `cli/`, `tests/` and `doc/` hold
research tooling/integration. Factor binary primitives with their current owners.
Proposed services: `llbinformat`, `llassembler`, `lllinker`, `llinline`; commands:
`llrxbin`, `llrxas`, `llrxlink`. STEP01 decides exact migration/package/API layout.
The wider toolchain delivery follows assembler, reassembler, linker, integrated
inliner; internal codec construction precedes the first assembler output and is
not a second framework. The standalone INL increment does not wait for complete
assembler, codec or linker delivery: its RXAS reader/model/writer are shared
target facilities, with existing native tools used only as independent checks.
The broader [inlining design](../../../compiler/docs/inlining_design.md#future-inlining-boundary-review)
provides background. The selected proposal separates expansion from a bounded
machine-cleanup service; it does not commission a general optimizer framework.
The nonoptimizing assembler and linker retain their contracts. A later driver
may explicitly invoke the same inliner/cleanup services around them.

## Selected standalone inliner outcome

The proposed [architecture and mapping rules](doc/inlining.md) implement one
producer-independent B/L target pass, callable separately from compiler,
assembler and linker orchestration. The first standalone adapter reads ordinary
UTF-8 RXAS and writes current RXAS for the unchanged-contract profile, preserving
separate module artifacts. An extension-enabled profile requires matching
approved directives/records and assembler/linker/VM support; it cannot claim
compatibility with unchanged consumers. Already optimized bodies are valid
inputs. A checked constructed-model client is the first development slice, not
the finished standalone proof of concept.
RXBIN input/output reuses the future complete codec and is not a prerequisite.
No runtime request shells out to native tools to parse, expand or optimize.

The complete selected outcome covers every case representable under the approved
ABI and preservation contract, including any selected extensions, rather than
stopping at scalar leaf calls. Stage growth through aliases/rebinding,
omissions/varargs, nested calls, multiple returns, objects/binaries/references,
state and imported helpers.
Specific unproved cases remain calls while implementation is incomplete;
they are not automatically reclassified as inherently infeasible or accepted
exclusions. Native/dynamic targets without one bound bytecode body, foreign private
globals without a binding mechanism, and physical-frame observations without an
equivalent are explicit current-boundary cases, not permanent impossibility
claims. In particular, review linker-private global aliases and exact-site
initialization ingress alongside virtual inline provenance and useful language
effect/ownership facts. Recursion/growth policy is distinct from semantic
feasibility. API/ISA, language/platform changes and production default replacement
require separate decisions.

| Stable criterion | Observable completion and verification |
|---|---|
| **INL-AC01 — shared service and real adapter** | One B/L checked target-model inliner serves an ordinary RXAS CLI, a callable B client and an independent handwritten/DSL-style producer. No AST inline payload, copied parser/framework or native backend is required. Input/output, unknown/unsupported forms and atomic publication have explicit contracts. |
| **INL-AC02 — exact call/storage mapping** | All admitted direct, counted, fixed and fused forms preserve actual evaluation, counts/presence/defaults, active and entry argument bindings, slot identity/permutations, status components, explicit reference/value behavior, nested calls and return copy/move ownership. Independent executable and structural controls include repeated actuals, mutable aliases and abrupt exits. |
| **INL-AC03 — full representable profile** | The architecture's coverage catalogue is implemented and checked, including rebinding, varargs, multiple returns, aggregate/reference lifetimes, context/cleanup and exact-bound imported helpers. Every retained case has a specific semantic, missing-proof, body-availability or policy reason. Unimplemented representable categories keep this criterion open. |
| **INL-AC04 — module and observation integrity** | Globals keep their module/storage owner; initialization, poison/cycle ingress, dependencies, numeric/signal state, reference expiry, source/TRACE/diagnostic and declared physical-frame observations remain equivalent. Evaluate linked private aliases and narrowly scoped runtime/metadata or language additions; record the selected compatibility/observation contract before implementation. Cases lacking an approved equivalent stay calls. |
| **INL-AC05 — effective bounded cleanup** | Expansion is correct with cleanup and assembler optimization disabled. Separately prove status/transport/copy elision, constants/presence, branch/reachability cleanup and alias-aware storage reuse on changed callers. A trivial leaf loses call machinery; an alias/snapshot case retains required copies. Previously optimized modules are covered without assuming their old analysis facts remain valid. |
| **INL-AC06 — deterministic policy and resources** | Separate legality, availability and profitability decisions. Explicit recursion, depth, code/register/metadata growth and work limits terminate predictably, support diagnostics and forced-legal/disabled controls, and preserve ordinary calls on policy refusal. Malformed input or failed transformation publishes no partial output. |
| **INL-AC07 — metadata and lifecycle** | Relocate executable and observation references together; keep runtime graph/provider/task/source contracts, source/module identity and initializer dependencies. Prune only under explicit reachability/consumer rules. Invalid/repeated/interleaved/resource/cancellation/I/O cases retain owned results and last-good publication. |
| **INL-AC08 — reviewed research delivery** | Early coherent implementation and representative RXAS review, focused development checks and a final compact integration panel qualify both expanded and retained cases against independent expectations. Guides, coverage/limits and this plan are current. No production-speed, full toolchain, wider platform, SAN-clean or publication claim follows automatically. |

| Stable step | Status, dependency and decision gate |
|---|---|
| **INL-STEP01 — architecture and contract** | Proposal under review; AC01–08. Freeze mapping, observation/coverage boundaries, service/CLI, default policy and bounded cleanup before product edits. This task prepares the concrete proposal; architecture approval remains the implementation gate. |
| **INL-STEP02 — shared model and conservative expansion** | todo; after 01; AC01/02/04/06/07. Extend the optional target owner and canonical schema, then qualify constructed-model local calls with fresh identities and no copy elimination. Review first emitted RXAS early. |
| **INL-STEP03 — standalone RXAS client** | todo; after 02; AC01/02/04/06/07. Add the L-owned RXAS reader/writer and thin CLI, preserving module artifacts, with a real generated caller and exact-bound imported global-free helper. This is an intermediate slice; full assembler/codec/linker delivery is unnecessary. |
| **INL-STEP04 — complete representable expansion** | todo; after 03; AC02–04/06/07. Batch the reviewed alias/rebinding, optional/variadic, nested/multi-return, aggregate/reference and state/cleanup categories. Classify each hard boundary; do not leave feasible categories silently outside the finish line. |
| **INL-STEP05 — proof and cleanup** | todo; after each coherent conservative category, with final qualification after 04; AC02–07. Implement bounded machine proofs and minimum cleanup as a separate callable pass. Retain independent no-cleanup results; inspect simple and alias-heavy output, not only execution. |
| **INL-STEP06 — standalone research acceptance** | todo; after 03–05; AC01–08. Freeze and run the compact agreed integration panel through the matching assembler/linker/VM profile (current tools for the unchanged contract; explicitly approved owner extensions when selected), plus CLI/service/DSL, negative/lifecycle and structural cases. Use one normal VM; broaden only for specific unresolved risks. Commit reviewed authorized paths locally; no push/default switch. Wider TGT criteria remain open. |

No build/test campaign is selected by this documentation increment. Per-feature
checks and execution allocation follow the repository instructions;
INL-STEP06 is the named standalone integration milestone.

## Complete initial target profile

RXAS means the full current UTF-8 source-visible language: declarations,
imports/exports, register banks, labels/fixups, all current instructions and
operand widths, constants/aliases, jump tables, metadata and source/TRACE
directives. Runtime-only opcodes are not admitted as source instructions.
Nonoptimizing assembly resolves required symbols and constructs valid graph
data, but performs no peepholes, instruction fusion, register reuse or inlining.
External legacy text encodings and future syntax are separate scope decisions.

RXBIN means current 007, not a new format: the fixed-width little-endian
container and all six sections (modules, instructions, constants, metadata,
graph facts and graph indexes), standalone/multimodule images and concatenated
container libraries. Read raw and current LZSS-compressed sections. The first
writer may emit valid raw sections; identical compressed bytes are unnecessary.
Decode to the owned model, validate, then encode; copying the input file cannot
satisfy reassembly. Header/feature/schema/alignment and reserved-field rules,
packed instruction integers, signed extrema and unsigned sentinels stay exact.

All 23 current record kinds are included: STRING, BINARY, DECIMAL, FLOAT, PROC,
EXPOSE_REG, EXPOSE_PROC; META_FUNC, REG, CONST, CLEAR, CLASS, ATTR, INTERFACE,
IMPLEMENTS, MEMBER, INLINE, SOURCE_STEP, TRACE_EVENT, TASK_TARGET, PROVIDER,
INITIALIZER and AUTOLOAD. Preserve unused records, ordered chains, raw literal
bytes and IEEE float bits, including signed zero/NaN payloads. Keep known inline
payloads opaque when not interpreting them. Preserve origins, procedure and
metadata addresses, module globals, initializer order, required/optional
providers, autoload hints, descriptions/RexxDoc and sealed task bindings with
their use-site records.

Semantic roundtrip preserves every supported payload and reference relationship,
not incidental pool IDs, packing, compression or serialized index ordering.
One relocation owner maps logical instructions, pool/module/graph IDs and
expanded code-cell addresses; it regenerates affected indexes and task seals.
Unknown structural record/section kinds, versions, schemas, unsupported feature
bits, invalid references and malformed or over-budget inputs fail explicitly.
An opaque known payload is distinct from an unknown layout that cannot be
safely relocated; never discard the latter or claim future-format roundtrip.

The linker implements current selection/control semantics: OMIT, INCLUDE, ROOT,
main/first-input defaults, import closure, conservative dynamic-member and
interface-factory retention, selector forms, duplicate/signature diagnostics,
maps, provider requirements and documented source/inline stripping controls.
Unresolved external/provider imports follow current link/runtime policy; required
closure failures remain errors rather than a blanket ban on external imports.
Preserve module identity/global counts, exposed alias links, stable module order
and module-instance initializer dependency/cycle/poison semantics; initializers
are neither exports nor selection roots. Merge compatible leaf
constants, relocate structured records and graph-bearing operands, refine only
unresolved import signature fields, and retain strict known type/flag contracts.
Rebuild the image graph, declaration origins, interfaces, dispatch/factory/
provider indexes and task-binding digests/placeholders. Autoload hints do not
discover link inputs. Reuse the committed mandatory-core provider policy and
generated canonical declarations, with checking implemented in B/L: retain
all provider/map metadata and omit only validated core bindings from external
archive requirements. Linking changes references/encoding where necessary but
does not optimize, expand or otherwise rewrite instruction sequences.

Naive expansion is one language-neutral target service, independent of source
compiler AST inline transport. Begin with proven direct local scalar calls;
then extend the reviewed profile to defaults/presence, nested calls, multiple
returns, imported helpers and a handwritten DSL using the same service.
Conservatively retain ordinary calls where equivalence is unproved, while
repairing representable supported shapes rather than broadly disabling them.
Each supported expansion proves argument evaluation/order/presence, aN entry
baselocals and UNLINK behavior, return moves and storage ownership, local/label/
metadata relocation, per-module gN identity, module-initialization ingress and
physical-frame, exception, numeric/context and cleanup observations. Mapping
aN to fresh rN alone is unsound: active and initial bindings must both survive
UNLINK/LINK/SWAP and nested call windows. RET may move owned locals but copy
arguments/globals/linked storage; arbitrary copies can change reference identity.
Preserve EXPOSE/borrow/reference expiry and signal/CANCEL cleanup. Inlining at
initializer ingress must retain callee initialization and cycle guards. Composite
origins alone do not reproduce an inline call stack. Missing proof keeps the
call with a specific reason; later profile growth needs review. Prove expansion
with subsequent RXAS optimization disabled, fresh IDs and no coalescing first.

## Stable acceptance criteria

Every criterion is **open**; passing an early subset cannot close the profile.

1. **TGT-AC01 — owned model and schema.** One checked, bounded module/image
   model represents the entire profile, typed reference domains and provenance.
   Canonical catalog/schema alignment and independent constructed-model tests
   prove operand identities, graph contracts and rejection of stale references.
2. **TGT-AC02 — L grammar and diagnostics.** Full current UTF-8 RXAS parses
   through L with correct byte/source spans, literal bytes, precedence and EOF.
   Independent positive/negative fixtures cover directives and grammar errors;
   diagnostics distinguish syntax, binding, format and resource failures.
3. **TGT-AC03 — lossless semantic codec.** All six sections/23 record kinds,
   raw/LZSS reads, libraries and linked images survive decode/encode/redecode.
   Independent fixed-width/record/graph oracles inspect complete meaning,
   extrema, float bits and opaque payloads; native interoperability adds proof.
4. **TGT-AC04 — complete nonoptimizing assembler.** Every profile construct
   assembles with exact symbols, literals, register banks, fixups, tables and
   metadata. Independently expected VM results and structural instruction/
   graph assertions pass; native differences are diagnosed, not copied blindly.
5. **TGT-AC05 — complete module linker.** Selection, signatures, globals,
   initializers, providers, interfaces, stripping, graph/task resealing and all
   relocation domains pass independent multi-input and negative fixtures.
   Instruction sequences remain unchanged apart from required reference remaps.
6. **TGT-AC06 — shared naive expansion.** The staged profile above uses one
   service for generated and handwritten callers. Structural expansion/call-
   retention assertions plus once-only, alias/state, omission, nested and
   multiple-return controls prove behavior and each admitted ABI obligation.
7. **TGT-AC07 — observable execution and provenance.** Output/control/state,
   TRACE/diagnostic source, condition/exception/context, module initialization
   and ownership/lifetime observations survive assembly, reassembly, link and
   admitted expansion. Physical-frame-sensitive cases retain calls unless proved.
8. **TGT-AC08 — resources and atomic lifecycle.** Checked input/output/work,
   decompression, graph/record and expansion limits fail without partial results.
   Malformed/truncated/unknown data, repeated small-after-large and interleaved
   owned requests, I/O failure and cancellation preserve last-good publication
   and clean temporary state; rejected input cannot leak partial model mutation.
9. **TGT-AC09 — independent consumers.** A non-C/G grammar client, constructed
   record client and real record-transform client exercise generic L services;
   a handwritten DSL emits, links and expands through the same target services.
   A representative compiler/library client supplements these independent tests.
10. **TGT-AC10 — bootstrap and delivery.** Explicit seed/producer dependencies,
    optional build/install closure and relocated CLI plus B service consumers
    work on the agreed platform scope. Runtime requests contain no forbidden
    native backend. Product defaults and core L packages gain no research edge.
11. **TGT-AC11 — final-stage L validation/debugging.** Exercise grammar/source/
    diagnostics, owned records/modules, transformation, validation and atomic
    output together, including failed transformations and independent consumers.
    Repair exposed defects at the generic L or target owner and retain focused
    regressions; document remaining profile limits without universal-L claims.
12. **TGT-AC12 — reviewed and qualified delivery.** Detailed scope/API/schema
    approval precedes implementation; independent expected behavior and early
    coherent source/RXAS/model review precede freeze. Selected affected checks,
    owning guides, limits and open criteria are current; scratch is deleted.
    Local increments, final initiative QA and any publication remain distinct.

## Stable implementation steps and decision gates

All steps are **todo**. Preserving the proposal does not complete STEP01 or
authorize implementation; detailed profile and contract approval remain open.

1. **TGT-STEP01 — profile and contract approval (AC01–12).** Freeze current
   grammar/wire/ABI crosswalk, ownership, service APIs, CLI names, bounded limits,
   platform/package scope and independent oracles. Resolve expansion/optimizer
   boundary without implementing an optimizer; obtain detailed user approval.
2. **TGT-STEP02 — owned model and primitives (AC01/08/09/11/12; after 01).**
   Reuse L-owned structures/validation, factor checked binary/schema primitives
   with existing owners and expose constructed-model/transform clients first.
3. **TGT-STEP03 — complete codec (AC01/03/07–12; after 02).** Implement raw/
   LZSS decoding, full validation and raw writing; qualify semantic no-op and
   a real record transform, malformed inputs, graph/task integrity and atomicity.
4. **TGT-STEP04 — complete L RXAS assembler (AC01–04/07–12; after 03).**
   Generate the grammar, bind every profile construct and emit through the same
   model/codec. Inspect first representative instructions early; complete the
   source/binding/error panel before claiming nonoptimizing-assembler coverage.
5. **TGT-STEP05 — module linker (AC01/03/05/07–12; after 04).** Implement
   selection, shared pools and relocation/graph/task closure without instruction
   optimization. Qualify independent linked-module lifecycle and failure cases.
6. **TGT-STEP06 — integrated scalar expansion (AC01/06–09/11/12; after 05).**
   Integrate the one shared expander developed through INL-STEP01–05; do not
   implement another algorithm. Prove local direct scalar cases and unchanged
   unsafe calls through the wider toolchain. Standalone development need not
   wait for TGT-STEP05.
7. **TGT-STEP07 — reviewed expansion growth (AC06–12; after 06).** Add defaults/
   presence, nested/multiple-return and imported-helper cases plus the handwritten
   DSL only with each ownership/frame/context obligation proved. Preserve open
   unsupported shapes explicitly; do not label the broader profile complete.
8. **TGT-STEP08 — final agreed integration (AC01–12; after 03–07).** Review the
   complete services and representative emitted code, qualify independent and
   compiler/library consumers plus agreed installed/relocated/platform checks,
   resolve final-stage L findings at their owners and update durable guides.
   Select broader QA for actual integration risks; keep replacement/performance
   decisions and unrelated develop publication outside this milestone.

GPT-6 Sol Extra High coordinates work and owns centrally scheduled build/test
execution and diagnostics. GPT-6.1 Sol Extra High workers own design,
implementation and source review with disjoint service/test owners, handing
execution to that coordinator. Development normally uses one normal configuration
and one VM, independent expectations and early coherent record/RXAS review; batch
material repairs and reuse unchanged prepared inputs. At STEP08 qualify both
concrete VMs and the declared delivery scope. Source-only B/L work does not
trigger per-increment sanitizer matrices; actual native changes/findings retain
repository rules. Do not retain run reports, logs or timing archives in Git.
