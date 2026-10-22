# Experimental Level C conformance profile

This compiler implements Classic Level C in B using Level L parsing, IR and
emission. It is an experimental, incomplete implementation of the agreed cREXX
profile, not a claim of full reference conformance or production replacement.
The [requirements ledger](levelc-requirements.md) owns detailed obligations,
[PLAN](../PLAN.md) owns delivery status and acceptance, and
[architecture](architecture.md) owns exact contracts. Existing native C is a
comparison implementation; its defects are not acceptance targets.

## Current baseline

Committed local profiles cover bounded expressions/control, shared local calls
and PROCEDURE/EXPOSE, RETURN/EXIT, ARG/PARSE, selected queues, explicit and
implicit ADDRESS, NUMERIC and binding routes for all 95 shared BIF names.
A binding route does not establish complete BIF, host or platform conformance.

The [whole SIGNAL contract](architecture.md#signal-complete-contract) combines
retained direct/quoted/VALUE transfers, seven-condition ON/OFF, producers,
cleanup and typed payload/origin transport as one complete instruction.
The [whole practical TRACE contract](architecture.md#trace-complete-contract)
adds source forms, shared activation options/query state, command inhibition
and a private observer over existing B services. The current C22 family has
88 operations, five C types and 36 target exports. These instructions include
their required helpers and dependencies; complete C remains open.

Controlled typed HALT exercises condition delivery, not real host/asynchronous
HALT. Default unhandled C events are nonresumable HALT with their original tag,
payload and origin; broader outward host resumption remains open. Internal
helper raw-location presentation may differ while required SIGL, condition
state and error behavior remain material. A dropped DO control read on advance
is attributed to END in this compiler; the native helper uses the DO header.
These boundaries do not split SIGNAL
into separate prerequisite or partial-profile deliveries.

## Accepted profile choices

- [Unicode text](../../../../compiler/docs/levelc_unicode_and_streams.md) uses
  codepoint positions, exact text and no implicit normalization. Ordinals
  0–255 retain the fixed Latin-1 bridge; byte-valued operations reject input
  above U+00FF rather than reinterpret UTF-8 bytes. Unicode behavior above 255
  is intentional. Names use the shared simple-casing policy.
- Modern SAY emits Unicode text through the output route, rather than promising
  historical raw-byte output. Wider host/callback/platform proof remains in
  the [SAY ledger row](levelc-requirements.md#whole-instruction-work-units).
- [Static statement providers](architecture.md#call3-static-provider-contract)
  are explicit, signed and prepared by their owning B/G/C pipeline. External
  function expressions, late provider search/loading and general callback
  integration are not implemented by that profile. The proposed Group 2
  extension below requires approval before expanding it.
- PARSE EXTERNAL and PARSE NUMERIC remain outside the current agreed source
  scope; their final Release 1 disposition remains explicit in LC-GAP-05.
- ADDRESS retains authored raw resource symbols. The shared 54.1 message prints
  that resource spelling, such as `missing.0`; native C can print `MISSING.0`.
  The condition identity and causing clause are unchanged.
- The approved SIGNAL profile canonicalizes direct, quoted and VALUE names
  through shared simple Unicode uppercase, then compares exact canonical text.
  This intentionally differs from reference literal/evaluated-name spelling;
  `1.20`, `1.2` and names with trailing blanks remain distinct. See the
  [language guide](../../../../docs/books/crexx_language_reference/statements.md#level-c-classic-signal)
  and [whole SIGNAL contract](architecture.md#signal-complete-contract).

## Proposed calls and host extension

[Group 2](architecture.md#group2-calls-host-proposal) is documented for approval;
it is not implemented or qualified. It proposes metadata-guided ordinary scalar
B/G/RXPA calls and external expressions, callee-owned defaults/ABI, runtime
RexxValue conversion and exposed scalar temporaries whose updates are discarded.
Adrian confirmed that B/G exposed globals are invisible/inaccessible to Level C
callers and true reference values require explicit adapters. Arrays, arbitrary
objects, methods/factories and task capabilities have no implicit scalar mapping.

The complete proposed increment retains host invocation modes, exact argument
presence and completion, live callback pool windows, configuration hooks,
queue/stream/ADDRESS transport, real clock/HALT producers and context lifecycle.
Adrian requires executable CALL to share exactly ADDRESS's low-level
process/byte-channel capabilities. He approved exit status as the routine result,
with stdout/stderr inheriting host streams, including host/OS redirection, and
no new CALL redirection syntax. Explicit process bindings and argument/failure
rules remain proposed, while
ordinary non-REXX typed/RXPA routines use their declared interfaces. Automatic
Classic source invocation remains deferred to final INTERPRET. Current UTC-based
TIME(E/R) is preserved. Any required new public API/ISA/platform disposition
retains its exact approval gate. Formal compiler/platform/delivery QA remains
open under [PLAN](../PLAN.md#group2-calls-host-plan).

<a id="practical-trace-target"></a>

## Implemented practical TRACE

The implemented contract is the native **Level C practical scope approved 2026-10-06**, as
defined in [TRACE requirements](../../../../docs/ai-context/CREXX_TRACE_REQUIREMENTS.md)
and native [LC-I-24 / LC-AC-77](../../../../docs/planning/release-1/levelc-compatibility-worklist.md#implementation-steps).
It does not require broader TRACE functionality for this compiler:

- Bare TRACE resets to Normal. Static/VALUE options and TRACE() share
  invocation state; the BIF returns the previous setting and applies a valid
  new one. Invalid options produce a source-anchored Classic error.
- `!` inhibits commands. `?` changes the reported interactive flag without
  prompting or reading control input. Signed counts are validated: zero selects
  Off; nonzero skip/pause and suppression counts are not enacted.
- SCAN is unsupported. Labels mode emits no label-pass records. Source,
  result and intermediate coverage is partial, including compound names,
  final expressions and optimized-away values. Emitted scalars must be their
  actual Unicode text; unavailable events are omitted.
- TRACE ENV selects Classic Error via its first letter E; the B/G environment
  extension is separate. ADDRESS classification remains coarse: N/E report
  nonzero RC or a condition; F reports negative RC or FAILURE.
- As in native C, an unexported command-only unit without TRACE, a TRACE BIF or
  an external-call boundary does not activate the shared controller at startup.
  Its initial N option state alone therefore produces no command records.
  Explicit Normal or bare TRACE activates the existing command reporting.

The new compiler omits the native text `>  >` file-transition marker; LLM
records retain logical filenames. A multiline composite clause can produce
additional truthful source records. These presentation differences and the
partial event coverage above are accepted within this practical scope.
Retained records can also include exact BIF results or static TRACE operands
omitted by native transcripts; CCOND adds the authored PARSE/ARG/PULL fields.
Incorrect displayed values, changed ordinary behavior, wrong targets/results,
or broken state, ownership, error and resource semantics remain material defects.

Focused development qualification uses the Debug optimized compiler with IR
transformation and assembler optimization on rxbvm. It covers instruction/BIF
state, internal/provider restoration, owned EXIT and fatal cleanup, actual
Unicode/NUL values, command statuses/inhibition, source/errors and raw C22
contracts. It does not establish full compiler, platform or delivery acceptance;
formal QA follows feature completion.

The [conditions and observations contract](architecture.md#conditions-observations-complete-contract)
extends those foundations with CALL ON/OFF for ERROR, FAILURE, HALT and
NOTREADY, policy replacement, delayed delivery/resumption, suppression and
buffered HALT. The shared B activation remains the single policy/event owner.
CALL handlers receive zero arguments and a fresh CALL/DELAY state; normal
RETURN results are ignored. Internal EXIT propagates the interrupted program
root; signed external handlers keep their distinct external root. CALL updates
ordinary SIGL while reserved .SIGL is preserved. SIGNAL retains its existing
same-frame transfer and causing-event state.

Authored clause completion preserves captured decisions, loop headers and
RETURN/EXIT values. Clock-free clauses remain clock-free; interrupted clause
samples survive handlers. Opted-in C PARSE/ARG/PULL executions emit exact retained
field and dot text in R/I text mode after each effect, including Unicode/NUL and
partial earlier writes. Direct library consumers default to no observations;
LLM retains its existing partial event scope. A source-less BIF handler's stream
completion belongs to the interrupted source dispatcher, with shared child
DELAY suppression for its same event. No isolated BIF child SIGNAL landing or
arbitrary raw-provider resumption is implied.

Qualification starts with maintained native fixture sources and checked expected
outputs, running the sources unchanged through B/L. Only harness/compiler and
controlled queue-injection boundaries adapt. Multi-error diagnostic fixtures
isolate each unchanged checked row at its original line/column for fail-first
parsing. C22 admission and new PARSE
observations have independent supplements. One normal Debug opt/IR-on/rxbvm
component lane establishes this local increment. Full compiler, delivery,
platform and formal QA criteria remain open.
The ADDRESS fixture's one raw-stem diagnostic parameter follows the existing
contract above; its other checked output is unchanged. Its actual `cat` command
uses a runtime-only PATH containing that command, while the compiler and other
targets retain an empty PATH without native source compilers.

## Reusable Level L backend boundary

Group 4 STEP01–06 uses the public, optional Level L
[`rxfnsl-analysis`](../../../../langtools/doc/ir-design.md#optional-analysis-and-proof-services)
and [`rxfnsl-rxas`](../../../../langtools/doc/rxas-design.md) packages. They supply
generic graph/effect/proof services and canonical instruction records with
checked whole-cell assignment. They can be used without a Classic frontend or
runtime. The C22 adapter owns context identity, pool/handler/callback premises
and conditional results; shared B remains the runtime algorithm owner.

The backend coalesces proved immutable context aliases, reuses eligible plain
scalar temporaries within one target block, and indexes per-function work.
Protected, cross-label, unknown-effect, reference and owned-object storage retain
conservative identities and cleanup points. Canonical native opcode effects and
signals stay authoritative; an unknown signal contract does not become a pure
operation to obtain a smaller frame. Existing practical TRACE and clause state,
argument presence, recursion and numeric failure semantics remain binding.
This is a compiler infrastructure increment, with no new source feature, native
C frontend change or performance/replacement claim.

Group 4 STEP07 adds the separate neutral
[`rxfnsl-execute` prototype](../../../../langtools/doc/execute-design.md),
with trusted schema/version admission, owned frames and local calls,
argument presence, bounded handler results and linear suspended continuations.
Its small Classic fixture reuses the actual shared VALUE, LENGTH and numeric
bodies; it does not execute the complete C22 family. In particular, neutral
`protect`, borrowed/escaping values and conditional extension results are
rejected before effects. An owned standalone fixture pool does not establish
live compiled-caller activation, labels, condition handling or source lifetime.
The C compiler continues to emit RXAS, modern RexxScript keeps its current tree
executor, and the existing compiled-image INTERPRET design remains authoritative.
The [execution assessment](architecture.md#group4-execution-assessment) owns the
remaining route decision and required bridges; INTERPRET is still last.

STEP08 integrates the four-image SDK delivery stage and explicit product
inventories. Actual package/import controls keep core/bootstrap, optional
analysis/RXAS/execution, C/G hosts and static C runtime separated. Independent
public clients select exact image roots; the RXAS guide's producer emits a
checked four-cell program that runs with the VM alone. C integration retains
the indexed graph/effects/target pipeline and its C22 ABI; no source-language
feature or native frontend changes follow from package closure. The owning
[contract](architecture.md#group4-package-closure-contract) distinguishes this
focused development closure from installed/platform and formal acceptance.

## Material work remaining

Full expression coverage, including binary strict inequality `\==`, remains
with CLC-STEP04. Negated strict equality `\(a == b)` is currently admitted.

The [plan](../PLAN.md#remaining-actions) and
[ledger](levelc-requirements.md#shared-requirements-before-instruction-closure)
retain remaining expression/BIF/host/lifecycle contracts, exact diagnostic
completion, delivery and declared platforms. Real asynchronous HALT and outward
raw-provider event resumption remain host integration work.
INTERPRET remains required and last. Accepted profile choices do not close these
obligations or establish full conformance.
