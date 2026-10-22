# Experimental Level C conformance profile

This compiler implements Classic Level C in B using Level L parsing, IR and
emission. It is an experimental implementation of the agreed cREXX profile;
full reference conformance, delivery qualification and production replacement
remain open.
The [requirements ledger](levelc-requirements.md) owns detailed obligations,
[PLAN](../PLAN.md) owns delivery status and acceptance, and
[architecture](architecture.md) owns exact contracts. Existing native C is a
comparison implementation; its defects are not acceptance targets.

## Current baseline

Locally delivered profiles cover admitted static source, expression/numeric and
control forms, shared local calls and PROCEDURE/EXPOSE, RETURN/EXIT, ARG/PARSE,
selected queues, explicit and implicit ADDRESS, NUMERIC and the static
value/context/error integration of all 95 shared BIF names. Full BIF-CHECK,
host, platform and complete-C acceptance remain separate obligations.

The [whole SIGNAL contract](architecture.md#signal-complete-contract) combines
retained direct/quoted/VALUE transfers, seven-condition ON/OFF, producers,
cleanup and typed payload/origin transport as one complete instruction.
The [whole practical TRACE contract](architecture.md#trace-complete-contract)
adds source forms, shared activation options/query state, command inhibition
and a private observer over existing B services. Group 3 extends the C23
baseline to C24 with 97 operations, five C types and 38 target exports;
its static profile is locally qualified. These instructions include
their required helpers and dependencies; complete C remains open.

The approved [semantic IR route](architecture.md#dynamic-execution-strategy) is
implemented and locally qualified under STEP11. It adds same-frontend INTERPRET groups, a checked
compiled-owner bridge, persistent conditions, invocation-owned PROCEDURE pools,
ordinary checked bytecode/RXPA calls and deterministic Classic source lookup.
The seven optional dynamic operations preserve C24 identities and static-only
closure. The selected component and end-to-end checks qualify this local
increment; complete LC-87/CFC and installed/platform proof remain open under PLAN.

The current dynamic observer uses shared source-map/index and TRACE writers for
authored clauses and actual assignment/load/operator/BIF values. Literal-only
`core.constant` intermediate events are omitted as an accepted minor presentation
deviation; full expression-event coverage is not claimed. ASM observations name
IR operations and LLM records carry logical source locations, with no coordinator
VM addresses. `scope.configureSource(enabled,roots)` is the runtime policy
boundary: standalone dynamic scopes enable the approved source policy by default,
while embedding policy integration and capability proof remain open. A cwd
snapshot alone does not establish an embedded filesystem permission policy.

The Group 2 real HALT ingress uses the existing execution-owner pending
POSIX_INT word and the same Classic delivery/buffering owner. Foreign/inactive
embedding requests fail without touching another execution. Without a live host
completion boundary, default unhandled C events are nonresumable HALT with their
original tag, payload and origin. An explicit host boundary retains the first
escaping event as an owned terminal completion and unwinds internal/external
Classic calls before result use. General outward raw-provider resumption remains
open. Internal
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
  is intentional. Ordinary Classic C character and name casing keeps an input
  codepoint in 0–255 within that range, retaining the original character when a
  simple mapping would leave it. Higher codepoints use simple Unicode mappings.
  Explicit Unicode casing retains its full contract; generic B/G Unicode
  semantics are unchanged.
- Modern SAY emits Unicode text through the output route, rather than promising
  historical raw-byte output. Wider host/callback/platform proof remains in
  the [SAY ledger row](levelc-requirements.md#whole-instruction-work-units).
- [Static statement providers](architecture.md#call3-static-provider-contract)
  are explicit, signed and prepared by their owning B/G/C pipeline. External
  function expressions, ordinary scalar typed/native procedures and configured
  callbacks use the approved Group 2 extension below. Automatic late provider
  loading and Classic source execution use the optional INTERPRET profile,
  locally qualified under STEP11; full delivery/platform qualification remains open.
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

## Calls and host integration

[Group 2](architecture.md#group2-calls-host-proposal) is approved and implemented;
focused component qualification is complete for the selected local Debug/rxbvm
profile. It supplies metadata-guided ordinary scalar
B/G/RXPA calls and external expressions, callee-owned defaults/ABI, runtime
RexxValue conversion and exposed scalar temporaries whose updates are discarded.
Adrian confirmed that B/G exposed globals are invisible/inaccessible to Level C
callers and true reference values require explicit adapters. Arrays, arbitrary
objects, methods/factories and task capabilities have no implicit scalar mapping.

The increment supplies host invocation modes, exact argument
presence and completion, live callback pool windows, configuration hooks,
queue/stream/ADDRESS transport, real clock/HALT producers and context lifecycle.
Adrian requires executable CALL to share exactly ADDRESS's low-level
process/byte-channel capabilities. He approved exit status as the routine result,
with stdout/stderr inheriting host streams, including host/OS redirection, and
no new CALL redirection syntax. Explicit process bindings preserve static presence checks and shared provider
failures; ordinary non-REXX typed/RXPA routines use their declared interfaces. Automatic
Classic source invocation is supplied by the separate optional INTERPRET profile;
locally qualified under STEP11. Full delivery/platform qualification remains open. Current UTC-based
TIME(E/R) is preserved. Any required new public API/ISA/platform disposition
retains its exact approval gate. Formal compiler/platform/delivery QA remains
open under [PLAN](../PLAN.md#group2-calls-host-plan).

## Static language functionality

The [Group 3 contract](architecture.md#group3-static-language-contract) is
locally qualified for its complete admitted static profile. It adds left-associative
power, integer division/remainder and the complete admitted strict/negated
comparison family, preserving tighter prefix precedence and eager logical
operands. `llctarget.binaryop` maps arithmetic to existing shared contextual
operations and strict comparisons to existing RexxValue methods. Shared B owns
numeric normalization, rounding, LOSTDIGITS and Classic error identities.
No BIF body, numeric representation or pool algorithm is replaced.

Static admission covers the agreed header, contextual-symbol, constant, quoted
Unicode/NUL, whitespace/comment/continuation and diagnostic forms, including dead
source. The five reserved period symbols keep their live pool route; other
nonnumeric period constants use 50.1. Unquoted ending-period function names use
51.1; quoted routine names retain their existing bypass.
`7d`/`7D` are the constant `7D`. Source uses the pinned Unicode/default policy;
runtime character configuration does not re-lex an already compiled unit.
Broader configurable-source callbacks and platform negator policy retain their
existing decision and acceptance gates.

Root/provider entries install their Config-owned source inventory once when a
SOURCELINE call or exported host boundary makes it observable, using compile-owned
physical lines with exact Unicode/NUL and LF/CR/CRLF handling. Unobservable simple
programs omit that setup. Internal labels share the inventory. An SRCMAP
generated buffer is
not an original inventory: SOURCELINE reports zero available lines, while mapped
diagnostics/TRACE use original text only when the RXPP wire supplies it.
`llcsrcmap` owns bounded decoding and provenance; generic L retains physical
byte origins. The C24 `source.map` declaration is compile-time metadata and
introduces no emitted decoder, runtime source reread or target export.

All 95 BIF names keep direct existing entries, ordered operands, raw omissions
and borrowed current Config/pool/activation. The
[BF01–09 crosswalk](levelc-requirements.md#group3-bif-crosswalk) reuses maintained
native C sources with checked expected values, alongside new all-name error,
configured-context, ordinal/casing and source-map boundaries in `tests/group3`.
These are qualification inputs, not inherited native passes. One prepared
normal Debug C-opt/IR-on/rxbvm lane qualifies this local increment; full BIF-CHECK,
CFC, platform/delivery and formal compiler acceptance remain open.
STEP11 locally qualifies INTERPRET and automatic Rexx-program invocation after
that static profile; full LC-87 and formal compiler qualification remain open.

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
Unicode/NUL values, command statuses/inhibition, source/errors and raw C23
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
parsing. C23 admission and new PARSE
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
runtime. The C23 adapter owns context identity, pool/handler/callback premises
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

The separate neutral
[`rxfnsl-execute` service](../../../../langtools/doc/execute-design.md) supports
checked manifests, owned frames/local calls, argument presence and conditional
absence, protection/declared ports, session leases and lifecycle cleanup.
Classic values, pools, conditions and BIF behavior remain in their adapters and
shared B owners. STEP11 supplies the live compiled-caller and full source-program
boundaries; standalone generic fixtures do not establish those language contracts.
The C compiler continues to emit RXAS, while runtime Classic source executes IR
through the [selected bridge](architecture.md#dynamic-execution-strategy).
Modern RexxScript retains its current executor pending its separate G-derived
language and consumer migration; it does not define Classic semantics.

STEP08 integrates the four-image SDK delivery stage and explicit product
inventories. Actual package/import controls keep core/bootstrap, optional
analysis/RXAS/execution, C/G hosts and static C runtime separated. Independent
public clients select exact image roots; the RXAS guide's producer emits a
checked four-cell program that runs with the VM alone. C integration retains
the indexed graph/effects/target pipeline and stable C body ABI; no source-language
feature or native frontend changes follow from package closure. The owning
[contract](architecture.md#group4-package-closure-contract) distinguishes this
focused development closure from installed/platform and formal acceptance.

## Material work remaining

Group 3 static source/expression, retained-source/SRCMAP and all-name BIF/context
integration is locally qualified under its selected component contract.
The full per-name BIF-CHECK and complete-C acceptance criteria retain their wider
verification; local qualification does not close them.

The [plan](../PLAN.md#remaining-actions) and
[ledger](levelc-requirements.md#shared-requirements-before-instruction-closure)
retain full-ledger, BIF-CHECK, host/lifecycle, formal compiler, delivery and
declared-platform acceptance. Group 2 supplies execution-local
real HALT ingress; declared platform proof and general outward raw-provider event
resumption remain open.
STEP11 supplies the final language increment, including INTERPRET and Rexx
source-program calls. Its local qualification does not close full LC-87 or these
obligations, and accepted profile choices do not establish full conformance.
