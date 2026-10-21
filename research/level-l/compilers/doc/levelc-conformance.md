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
and a private observer over existing B services. The current C21 family has
86 operations, five C types and 36 target exports. These instructions include
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
  integration remain deferred; native integration scope is not expanded.
- PARSE EXTERNAL and PARSE NUMERIC remain outside the current agreed source
  scope; their final Release 1 disposition remains explicit in LC-GAP-05.
- The approved SIGNAL profile canonicalizes direct, quoted and VALUE names
  through shared simple Unicode uppercase, then compares exact canonical text.
  This intentionally differs from reference literal/evaluated-name spelling;
  `1.20`, `1.2` and names with trailing blanks remain distinct. See the
  [language guide](../../../../docs/books/crexx_language_reference/statements.md#level-c-classic-signal)
  and [whole SIGNAL contract](architecture.md#signal-complete-contract).

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
Incorrect displayed values, changed ordinary behavior, wrong targets/results,
or broken state, ownership, error and resource semantics remain material defects.

Focused development qualification uses the Debug optimized compiler with IR
transformation and assembler optimization on rxbvm. It covers instruction/BIF
state, internal/provider restoration, owned EXIT and fatal cleanup, actual
Unicode/NUL values, command statuses/inhibition, source/errors and raw C21
contracts. It does not establish full compiler, platform or delivery acceptance;
formal QA follows feature completion.

Begin-only activation clause clocks are locally qualified under
[CLAUSE1](architecture.md#clause-clock-contract); complete observations remain open.

## Material work remaining

Full expression coverage, including binary strict inequality `\==`, remains
with CLC-STEP04. Negated strict equality `\(a == b)` is currently admitted.

The [plan](../PLAN.md#remaining-actions) and
[ledger](levelc-requirements.md#shared-requirements-before-instruction-closure)
retain complete clause/clock observations, delayed CALL4 handlers, remaining
expression/BIF/host/lifecycle contracts, delivery and declared platforms.
INTERPRET remains required and last. Accepted profile choices do not close these
obligations or establish full conformance.
