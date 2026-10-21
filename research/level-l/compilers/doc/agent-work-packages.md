# Level C implementation work packages

**Approved execution instructions, 19 October 2026.** The
[plan](../PLAN.md#c-first-delivery-milestone) is authoritative; this document
specifies assignments, ownership and review. It does not start implementation,
automations or new branches. The [architecture](architecture.md) and
[requirements](levelc-requirements.md) are mandatory reading with AGENTS.md.

## Coordination model

Adrian requested an Astra Extra High coordinator for review and coordination,
with GPT-6.1 Sol Extra High subagents. Configure future worker
assignments with `model=gpt-6.1-sol`, `reasoning_effort=xhigh`; a requested model
must not be silently substituted. The coordinator owns reviews, independent
validation of code and evidence, scheduling, interface and acceptance decisions,
and coordination. The coordinator also authorizes and oversees commits and
publication within then-current authority. Assigned Sol workers perform
implementation, shared-file integration, documentation, builds, tests and
cleanup. These are roles requested
by Adrian, not a claim about the model identity of a previous or current session.

Use the existing synchronized `develop` checkout. Do not create branches or
worktrees. All agents share files: the coordinator grants exclusive ownership
of named paths before writes. Record baseline HEAD and unrelated dirty paths;
keep their contents unchanged. Assign a single worker integrator for shared CMake,
public schemas and canonical plans. Workers report diffs; they do not commit,
push, stash, reset, clean, switch branches, change automation or start unassigned
broad builds.

With four available slots, use coordinator plus at most three workers. Useful
parallel work is a generic L implementation, a runtime/Unicode implementation
on disjoint files and an independent test/review task. The C frontend/selector
instruction stream has one writer at a time. Never use concurrent full builds
or build/test activity in the same output tree. Share prepared prerequisites
and schedule focused checks centrally; isolation is by build/test location,
not by extra source checkouts.

For each instruction, use a bounded implementation assignment followed by a
separate review assignment with independent expectations. The reviewer can
prepare test cases first without editing the implementation's paths. Read-only
review may overlap implementation only against a named stable checkpoint;
review the final diff again when changed. Agents do not approve their own
semantic exceptions or silently relax tests.

## Work package catalogue

Each package is split into reviewable slices. A package name is not authority
to implement every item at once. Dependencies below identify the earliest safe
start; CLC stage closure also requires the complete plan criteria.

| Package / owner | Allowed implementation area | Inputs and deliverables | Decisive acceptance / dependencies |
|---|---|---|---|
| **WP-01 Contract audit** | Read-only sources; assigned worker writes requirements/plan after coordinator review | Reconcile LC-I, BIF, LC-REF and approved exceptions; inventory current test paths, APIs/platforms and C-only host/target closures. Propose minimal independent fixtures and exact changed interfaces. | CFC-AC01/07/12; CLC-STEP01. No native status copied into new completion. |
| **WP-02 L analyses and generation** | Assigned `lib/rxfnsl/rexx/`, `langtools/rexx`, stamp source-closure manifest, independent `langtools/tests` and generic docs/examples | Existing generic analyses retain their contracts. The [approved LRUSE plan](../../PLAN.md#current-action-lexer-reuse) assigns pure generation-service/private main+shield capsule and automated stamp identity/closure here, with independent non-C/G admission/equivalence proof. Coordinate build injection with WP-10. | CFC-AC07/11; STEP02 and LRUSE-STEP01/02. Approval covers the recorded reuse contract; no SDK/runtime API or algorithm change, C semantic policy or new provider is commissioned. |
| **WP-03 Classic runtime** | Assigned `lib/rxfnsc` classes/tests | Reconcile activation/pool/config/value/condition ownership, direct operations and BIF ABI; live pool binding, shared body/entry contracts, cleanup, presence and context inheritance. Keep old consumers compatible. | CFC-AC02/03/09/11; STEP03. Direct B tests plus affected native C/RexxScript/typed Classic regressions; public ABI approved. |
| **WP-04 Shared B Unicode** | Assigned `lib/rxfnsb/unicode`, `tools/unicode` and affected data/consumer paths | Extraction/public B API, preserved G facade, one pinned data owner and acyclic bootstrap are completed foundations. Remaining assignments integrate new C names/BIF consumers; add property/simple-map/traversal/budget interfaces only for an identified need and exact API approval. | CFC-AC07/09/12; STEP03/04. Reuse unchanged B/G/core qualification; prove changed C recognition/bindings, errors and provider/freshness closure. Coordinate CMake edits; no same-file overlap with WP-03. |
| **WP-05 C source/expressions** | Assigned `levelc` grammar/parser/frontend/IR modules and compiler tests | Complete C-SRC/C-EXPR, source/AST schemas, static validation, Unicode names, operators, numeric contracts and BF01–04 binding. Use L and runtime APIs, no duplicate algorithms. | CFC-AC01–03/06/07; STEP04 after needed WP-02/03/04 contracts. Native comparisons are controls. |
| **WP-06 One C instruction** | Named grammar/frontend/IR/selector paths and a named instruction fixture directory | One LC-I row, all forms/errors/edges, worked AST/IR/RXAS, runtime interface use and independent expectations. Required shared improvements are explicit linked dependencies. | CFC-AC01–04/06; STEP05–07. Serial instruction writer; separate review packet. Full closure waits for all listed shared proofs. |
| **WP-07 BIF/host family** | Assigned runtime family/adapter and tests; frontend binding edits coordinated | One BF integration group or C-HOST contract at a time. Reuse existing BIF bodies, fixtures and valid qualification; bind names/presence/context/errors through the new compiler. Complete only named missing host contracts. Library edits require a reproduced defect, identified open contract or approved Unicode extraction. | CFC-AC01/04/09/11; STEP06–08. New public host APIs require review; no alternate ADDRESS protocol or dynamic external-call policy. |
| **WP-08 C target quality** | Assigned C selector and `compilers/common` target utility paths | Target records, liveness-driven register reuse, call windows, ABI cleanup and C-OPT01–05. Explain legality and compare actual caller plus helper RXAS. | CFC-AC06/07/11; STEP09 incrementally. WP-02 generic facts; no target policy in SDK. No broad performance work for research. |
| **WP-09 RXAS/VM primitive** | Precisely assigned assembler/binutils/interpreter/disassembler/linker paths and direct tests | One reproduced cost or missing execution capability; compare existing sequence, library/selector fix and primitive. Propose exact operands/effects/alias/errors/source/encoding/platform contract before code. | CFC-AC10/11; STEP09 or final STEP11b. Explicit native/ABI gate; mandatory first Release verdict for production optimisation. No language parser/BIF dispatch in VM. |
| **WP-10 Delivery and independent QA** | Assigned CLI/build/test/packaging files; assigned worker owns shared root integration | Retain delivery/closure duties. Approved LRUSE-STEP03/04 owns CLI transport, output-derived REUSE_LEXER persistence/lock/checksum/publication, stamp build injection/dependencies/cleanup and initial research-C-only opt-in. Independently qualify non-C/G first, then focused C cold/warm and unchanged G controls; coordinate stamp manifest with WP-02. | CFC-AC08/11/12; STEP10/11d and [LRUSE](../../PLAN.md#current-action-lexer-reuse). Reuse valid proof, measure necessary aggregate additions under AGENTS; no automatic broad/sanitizer campaign or free shared cache path. |
| **WP-11 INTERPRET compiler/runtime** | Assigned C mode/entry/continuation and B runtime dispatch files | Final-stage mechanism proof, then same-pipeline group compilation, live caller binding, cross-owner outcomes, handler resumption and source observations. Interfaces agreed with WP-09 image ownership. | CFC-AC05/11; STEP11 after static acceptance. No source compiler/evaluator fallback and no premature dynamic implementation. |
| **WP-12 Independent acceptance review** | Read-only implementation and temporary diagnostics; own test edits only if granted | Check contract-to-code-to-test traceability, inspect RXAS/helpers, challenge effect/lifetime assumptions, compare final diff to approved scope and verify preservation. Findings name mechanism, affected criterion and reproducer. | Every increment; full review at STEP10/11d. Coordinator independently validates; worker assertions alone never close a criterion. |

<a id="call-assignment-packet"></a>

## CALL assignment packet

The [CALL contract](architecture.md#call-contract) and
[CALL1–4 plan](../PLAN.md#current-action-call) own scope and CCALL-AC01–07.
Standing user authority permits stepwise reviewed local commits before INTERPRET.

- **CALL-WP01:** serial Sol 6.1 Extra High C integrator owns grammar/parser,
  frontend/IR/admission/catalogue/selector/target, all docs/AGENTS/CMake and shared
  build/test execution. CALL2/3 and local RETURN/EXIT profiles are qualified under
  the [CEXT contract](architecture.md#call3-static-provider-contract) and
  independent generic classifier; CALL4 follows STEP07. Preserve
  prepared-language reuse and existing B runtime algorithms.
- **CALL-WP02:** independently assigned runtime worker owns only `tests/call/**`;
  audit B signatures/condition/result ownership read-only, write focused source,
  metadata/admission/error and shape oracles. No concurrent shared builds.
- **CALL-WP03:** independent reviewer reads native sources/contracts and final
  C/raw-IR/caller/helper RXAS. Coordinator validates scoped qualification and
  commit paths; continue the next named feature/instruction after each commit.

### RETURN/EXIT local profiles and next dependency

The [RETURN contract](architecture.md#return-contract) owns RRET-AC01–05;
RETURN1–3 are complete for the local source/completion profile. The serial
C integrator owns `llcparse`, docs/CMake and builds. The independent fixture
writer owns only new `tests/return/{source-c.crexx,source-c.expected,contracts.crexx,
host.crexx,check-return.py}`; the reviewer remains read-only. Reuse current packets,
cleanup/status and CALL2/PROCEDURE proof; no grammar/schema/B/L/VM API changes.
The [EXIT contract](architecture.md#exit-contract) owns the locally qualified
CEXIT-AC01–05 guard/terminal and forced-cleanup profile; AC06 stays open. The generic selected-callable classification and CALL3 local provider/wrapper
profiles are delivered. Independent fixtures
and read-only review retain their separately assigned owners.

The [PULL contract](architecture.md#pull-contract) delivers the local AC01–04
source/lowering and independent fixture profile through the existing PARSE
source/executor. Bare acquisition and shared host/observation/platform owners
remain explicit. PUSH and QUEUE have separate local insertion profiles: one
serial production integrator, independent fixtures, and read-only review; each
exact contract precedes its implementation. The [PUSH contract](architecture.md#push-contract)
delivers the local AC01–04 profile: the serial integrator owns production/schema/
parser/selector/docs/CMake; the independent reviewer owns `tests/push` source/
host/refusal controls; existing B code owns queue/configuration algorithms. AC05
remains shared work. The [QUEUE contract](architecture.md#queue-contract) assigns
the locally delivered AC01–04 profile to the serial integrator for production/
schema/docs/CMake and the independent reviewer for neutral `tests/queue-insertion`
plus thin PUSH/QUEUE fixture entries. No parallel shared builds; AC05 remains
shared scope. Explicit ADDRESS and implicit commands/warnings are locally
delivered through the reviewed shared-B/C contract.

The [ADDRESS B bridge](architecture.md#address-b-binding-bridge) is a separate
prerequisite. Its worker owns only `_address.crexx`, `RexxClassicAddress.crexx`,
old/new B protocol/command tests and their owning B API docs. The integrator owns
CMake/architecture/PLAN and disjoint C design. Transfer one exclusive B build/
test lease explicitly; no C preparation while it is active. Independent review
checks actual worker/caller RXAS, compatibility, exact paths and preservation.

The [C ADDRESS packet](architecture.md#address-c-command-contract) assigns the
locally delivered C15 keyed explicit profile to the serial integrator for parser/schema/frontend/private
proof/selector, CMake and owning documentation. The independent fixture worker
owns `tests/address`; review checks emitted owner/borrow/condition cleanup and
source/clause observations. The B prerequisite is delivered separately. The
C16 implicit-command/warning profile is locally delivered. The independent
worker owns `tests/command` source/host/warning/raw controls; the same integrator
owns production and preparation. CADDR-AC08 and whole LC-I-20/21 retain their
shared condition/host/platform owners.

The [NUMERIC FORM diagnostic prerequisite](architecture.md#numeric-form-diagnostic-prerequisite)
is a separate shared-runtime correction. Its worker owns only
`RexxActivationArguments.crexx`, the existing activation test and the
target-context error expectation. The integrator owns contract/status,
preparation, tests, cleanup and commit; no concurrent builds or native compiler
changes. Independent review checks the error identity, unchanged mutation
semantics and exact local scope before C NUMERIC integration.

The [NUMERIC C packet](architecture.md#numeric-c-contract) assigns parser,
frontend/schema/selector, CMake and owning documentation to the serial
integrator. The independent fixture worker owns only `tests/numeric`
source/golden/host/tiny raw/checker inputs after exact contract review.
Independent review checks source diagnostics, current-frame install, operand
EXIT, no duplicate numeric algorithm and actual caller/helper RXAS. The
integrator retains the exclusive build/test/cleanup/commit lease; NUM-C-AC07
keeps shared observations and declared platforms open. NUM-C-STEP01–03 are
locally complete for the source-integration profile. Clause/condition
prerequisites for SIGNAL and CALL4 follow their exact contract reviews.

The [external condition B prerequisite](architecture.md#external-condition-b-prerequisite)
assigns only `lib/rxfnsc/RexxActivationArguments.crexx`,
`lib/rxfnsc/RexxClassicAddress.crexx`, the existing
`testRexxActivationArguments.crexx`, `testRexxClassicBifCondition.crexx` and
`testRexxClassicAddressCommand.crexx` fixtures, and `lib/rxfnsc/address.md` to the
runtime worker after exact record review. The integrator owns planning, serial
preparation/testing, cleanup and the separate commit; no new executable or
CMake target is needed. C wrapper integration, SIGNAL/CALL source and native
relay are excluded from that source lease.
COND-B-STEP01–03 are locally complete for the shared B prerequisite; its AC06
keeps C/provider and wider condition integration separate.

<a id="procedure-assignment-packet"></a>

## Approved PROCEDURE assignment packet

**Approved static/fatal profile locally qualified; CPROC-AC07 and whole LC-I-12 remain open.**
The [approved contract](architecture.md#proposed-procedure-contract) owns the
AST, PROCEDURE entry/IR baseline, existing B method use, cleanup and CPROC-AC01–07.
The [current C contract](architecture.md#current-c-family-contract) supersedes its
original family inventory; old C schemas are not admitted.
The [plan](../PLAN.md#current-action-procedure) owns PROCEDURE1–4; CALL follows.

- **PROCEDURE-WP01 — design complete:** one documentation integrator owns this
  packet, architecture and brief plan links. Runtime and independent reviewers
  audit APIs, source/error semantics, entry/DO invariants and lifetime read-only.
- **PROCEDURE-WP02 — local profile complete:** one serial C writer owns `levelc.rxl`,
  `llcparse`, `llcfrontend`, `llcir`, `llcrxas`, bounded private C admission and
  required `llcdoprotocol` integration. No shared B algorithm/API, native C,
  generic L or VM implementation is assigned. Coordinate metadata/view/CMake
  changes through that writer; no simultaneous frontend/selector writer.
- **PROCEDURE-WP03 — local profile qualified:** separately assign independent
  `tests/procedure` fixtures and existing frontend/backend/context/metadata
  selectors. One build owner schedules affected preparation, focused normal
  Debug, consumer regressions and product targets. WP-12 reviews the frozen
  final diff/full caller/helper RXAS against CPROC acceptance; root validates.

Preserve unrelated work, current B/G/Classic consumers, exact target exports
and truthful source. Only the assigned build owner runs shared preparation/tests;
do not stage/commit, push or change automation. Any local commit requires
coordinator authority; wider observations/platform/INTERPRET remain open.

### Approved prepared-language reuse

The [approved contract](architecture.md#proposed-prepared-language-reuse) owns
the exact L borrowed APIs, private C owner and CPREP-AC01–05. Adrian approved
CPREP-STEP01; CPREP-STEP02/03 are implemented and locally qualified.

- **CPREP-WP01:** one assigned generic L writer owns `lllanguage`, `llmunch`,
  `lllr`, `llcfg` and independent borrowed-reader tests. Preserve owning APIs;
  add no C/Unicode policy or native/VM implementation.
- **CPREP-WP02:** the serial C integrator owns the private module initializer,
  parse/header/name integration and CMake/CLI/service consumers. Keep comments,
  limits and source state request-local; no table-copy fallback.
- **CPREP-WP03:** independent reviewers check the complete ownership/copy chain;
  one build owner schedules the focused proofs and reuses unchanged qualification.
  Preserve the qualified scope and reuse unchanged checks.

<a id="arg-assignment-packet"></a>

## Approved ARG instruction assignment packet

**Exact ARG AST/IR/export contract approved by Adrian.** The
[ARG contract](architecture.md#proposed-arg-contract) owns its requirements;
the [plan](../PLAN.md#current-action-arg) tracks status and remaining work.
Original CFC/CPARSE criteria and last-stage INTERPRET remain.

| Assignment | Exclusive ownership / deliverable | Independent review |
|---|---|---|
| **ARG-WP01 — ARG-STEP01; approved/complete** | One documentation integrator prepared exact CArg(templates:ParseTemplates), retired ArgTarget/items, removed arguments.read/readarg and C v8/five types/53 operations/33 exports; Adrian approved the packet before product edits. | Verify public changes are explicit, no token/span fabrication or compatibility shim, unchanged LCP1/executor/source-adapter/ISA and numbered acceptance preserve whole-instruction boundaries. |
| **ARG-WP02 — ARG-STEP02; locally qualified** | One serial writer owns `levelc.rxl`, `llcparse`, `llcfrontend`, `llcir`, `llcrxas`, target `llctarget`, and assigned C schema/admission/target fixtures. Reuse ParseTemplates and private template lowering; classify empty CArg before dead-block preparation; retire only the obsolete scalar operation/export/body and update exact admission/version inventories. | Source/AST spans, contextual tokens/ELSE, empty ordinals, packet UPPER, raw arguments/presence, live reads/writes/compound names and authored negative/error behavior. Shared B executor/Unicode/native C remain read-only. |
| **ARG-WP03 — ARG-STEP03 locally qualified / STEP04 locally committed** | Assigned test writer owns existing frontend/backend selectors, independent ARG positive/error fixtures and case registration; the integrator updates affected direct-IR/target/argv consumers and maintained example views. One build owner prepares affected targets and schedules focused normal Debug/toolchain proof. | Paired B-mode/VM cells, both C IR modes, assembler-off with a separate assembler-on control, exact v7 refusal/current schema/export checks, bare/comma-only blank-equivalent dead-code shape and full caller/helper RXAS. Measure any expanded nested aggregate before accepting scheduling/timeout properties. |
| **ARG-WP04 — ARG-STEP04 locally committed / STEP05 open** | WP-12 read-only final implementation/RXAS/diff review; single docs integrator maintains enduring contracts, open shared work and delivery status. | Coordinator independently validates the work and preserves unrelated files; no push, hosted wait, native refresh, broad aggregate or per-slice sanitizer campaign. ARG-AC05/whole LC-I-11 and all CFC criteria stay open without required shared activation/observation/delivery proof. |

Focused Unicode cases cover a decisive sample such as é/µ/ß/ÿ, emoji and NUL,
unchanged raw source and unchanged dynamic patterns; reuse qualified shared
Unicode/executor proof. Do not add BIF surface or a new Unicode campaign merely
to make fixtures convenient. Native C is a control with a known eager-operand
limitation; independent expected pool states decide disagreement. No competing
plan, raw run bundle, duplicated matcher or ARG-specific runtime is commissioned.

<a id="parse-integration-assignment-packet"></a>

## Approved PARSE integration assignment packet

**Implementation approved and delivered in the finite fatal-error profile.**
The ARG increment introduced v8/53 operations and 33 target exports; the
[approved PROCEDURE protocol](architecture.md#proposed-procedure-contract) advances
the shared source-unit ABI; the [current C contract](architecture.md#current-c-family-contract)
owns the complete family inventory. ARG leaves PARSE's packet,
executor and acquisition signatures unchanged.
The [approved integration contract](architecture.md#proposed-c-parse-integration)
owns the v3 cursor-only descriptor, v7/54-op C schema, LCP1 packet,
two B signatures and CPARSE-AC01–08/STEP01–05. STEP05 remains open for
complete conditions/TRACE and declared delivery/platforms. Approval covers shared cursor/B reuse, focused tests,
actual RXAS review and scoped local commits with Sol 6.1 Extra High workers.
The coordinator assigns exclusive paths and schedules builds before product
work; push/publication is not commissioned by this packet. Do not
implement native Classic frontend changes, ARG/PULL instruction closure or
INTERPRET incidentally.

| Assignment / phase | Exclusive write ownership to assign and deliverable | Required independent review |
|---|---|---|
| **PARSE-WP01 — CPARSE-STEP02** | Named `interpreter/rxvmparse.c`/private header, cursor-driver admission and opcode guard, and assigned `tests/rxpa/parse_cursor*` fixtures. Implement exact v3 section transitions through the existing seven-callable built-in service and shared kernel; preserve v1/v2/direct operations. Root build/static manifests only through the single assigned integrator. | AC02/05/08: direct non-C B/assembly transition, targetless/live-input, malformed, lifetime/resource and compatibility controls. Reviewer checks actual movement/empty-pattern/anchor cases and both VM drivers; source/manifests do not prove real platforms. |
| **PARSE-WP02 — CPARSE-STEP03** | New shared `lib/rxfnsc/RexxClassicParse.crexx`, named direct-B functional tests and the one acquisition export in existing `levelc/runtime/llctarget.crexx`. Own runtime LCP1 envelope/map validation, existing CharacterScan word distribution, current raw-name reads/writes, exact finite cleanup/relays and checked source adapters. Shared `rxfnsc`/target CMake/metadata entries via assigned integrator only. | AC03–05/07: direct consumers with independent expected pool/ARG/source/UPPER/blank state; all-drop laziness, skip/error cleanup and original signal provenance. Conditions/TRACE seams remain open until actual STEP03/07 integration; no invented generic signal ABI. |
| **PARSE-WP03 — CPARSE-STEP04** | Named `levelc.rxl`, `llcparse`, `llcfrontend`, `llcir`, `llcrxas` and a single C-owned `levelc/llcparseprogram.crexx` codec/admission module, plus assigned `tests/parse` fixtures. Own named AST, exact source/comma lowering, immutable LCP1 construction, all-record/dead/loaded admission and constant-binary target calls. Codec is shared inside the compiler, with no runtime dependency on it. | AC01/03–06/08: independently authored forms/errors/order tests, malformed packet/schema/import controls, old-output preservation, compiler-host closure and complete caller/helper RXAS. Import exact metadata through explicit artifacts; target runtime is never linked into compiler host. |
| **PARSE-WP04 — CPARSE-STEP05** | Read-only final diff/source/RXAS review; own only explicitly assigned acceptance fixtures and the canonical four planning/docs paths through one documentation integrator. Review LC-I-19 against every CPARSE criterion, identifying current fatal-profile versus full observation/platform boundaries. ARG and PULL receive separate later instruction packets. | Verify worker evidence against exact inputs and independent expectations. Keep full criterion open when its conditions/TRACE/platform proof is absent; completed cursor or native status cannot substitute. No additional broad run merely to accumulate green checks. |

Implementation starts with one generic worker and an independent contract/test
reviewer; only disjoint shared-B/compiler preparation may overlap after their
exact interface checkpoint is stable. There is one writer per selector, runtime
adapter, manifest and planning file. Builds/tests in a shared output tree are
scheduled centrally. New aggregates are measured in isolated normal Debug,
serialized with generous evidence-based timeout properties; native v3 changes
retain applicable sanitizer obligations. Reuse valid unchanged-input proof and
do not dispatch overnight matrices for this routine research integration.

Each worker's handoff states all open CPARSE/CFC obligations, exact changed
paths/input identity, mode/platform and code-shape results. Read-only native C
and Regina controls are classified when they disagree with the independent
contract. Keep only lasting conclusions and maintained fixtures; delete probe
files/logs instead of creating another report, run directory or handoff plan.

## Assignment template

The coordinator supplies these fields in each worker prompt:

1. **Objective:** one concrete behavior/failure and the intended end state.
2. **Baseline:** exact HEAD, approved architecture version and relevant existing
   local edits; no resetting or syncing by the worker.
3. **Scope:** CLC step, CFC criteria, LC-I/BF/LC-REF IDs and exact owning docs.
4. **Write ownership:** explicit files/directories, exclusions and integrator for
   shared files. Read access is broader; write ownership is not.
5. **Contracts:** accepted records/APIs/ABI/effects, decisions still open and
   producer/consumer interfaces expected from other packages.
6. **Tests:** independent expected values/state/errors/source and specific normal
   commands; maintained sanitizer work only where required/assigned. Declare
   shared build-tree access and any nested aggregate scheduling requirement.
7. **Quality:** representative complete RXAS cases, helper bodies, expected
   structural improvements and preserved observations; compiler and RXAS modes.
8. **Stop conditions:** unapproved language/architecture/ABI decision, unsafe
   dependency assumption, unexpected active-file edits, regression or actual
   sanitizer finding. Report promptly; do not weaken tests or invent a fallback.
9. **Handoff:** changed paths, mechanism, criterion status, exact tests/input
   identity and limitations, unresolved findings and proposed next step. Keep
   lasting conclusions in owning docs; delete temporary diagnostics. No raw-log
   bundles, dated receipts, screenshots or per-run hashes in the source tree.

For example, WP-06/LC-I-02 covers DROP direct/indirect forms, compound lookup
order, alias lifetime and invalid-word policy. It must state expected pool
states before/after each operation, specify `C-POOL` effects, verify native
comparisons only in matching dialects and inspect actual pool-drop RXAS calls.
It must not concurrently add PROCEDURE, change the global pool ABI or implement
VALUE merely to make a test pass; those dependencies belong to their owners.

## Integration and publication gates

The coordinator first checks changed paths against ownership and active-work
hashes, then reviews the implementation and the independent review findings.
Assign workers only decisive checks whose evidence is missing or invalidated;
the coordinator validates their evidence rather than executing builds or tests.
Verify evidence for the normal product build and appropriate functional/correctness
suites before any substantive publication, following AGENTS. An author and reviewer
both reporting success does not replace coordinator inspection.

For a production RXAS/VM performance edit, freeze after minimum focused
correctness, assign a worker the governed first ordinary Release comparison,
review and report the verdict, then stop for Adrian's direction before broad
closeout. This gate does not apply to ordinary Level L research increments;
actual sanitizer findings retain SAN worklist/regression/platform closure
requirements in either case.

Commit only assigned, qualified paths when publication is authorised. Verify
remote develop and discover actual exact-SHA normal workflows; do not rerun or
resume CI monitoring without the current monitoring instruction. A new unrelated
remote commit requires reconciliation while preserving active work. A green
individual job is not a green workflow or release.

Before a context handoff, read/update the current plan rather than creating
another plan or dated note. Name open criteria and decisions explicitly. After
approval, maintain enduring contracts, current task status and known limits;
report qualification results in conversation and preserve regression fixtures.
