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

Static SIGNAL C18 is locally committed with partial qualification. Its [direct/quoted/VALUE contract](architecture.md#signal-static-contract)
is separate from SIGNAL ON/OFF and condition handlers. New-C TRACE is **not
implemented**; existing source/debug records and shared runtime state do not
establish TRACE instruction or event support.

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
  and [C18 contract](architecture.md#signal-static-contract).

## Practical TRACE target

The target is the native **Level C practical scope approved 2026-10-06**, as
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

These are accepted baseline limitations, not newly discovered new-C failures.
Minor formatting or event-coverage differences within this scope are recorded
for later judgment rather than repeatedly reopening delivery qualification.
Incorrect displayed values, changed ordinary behavior, wrong targets/results,
or broken state, ownership, error and resource semantics remain material defects.

## Material work remaining

The [plan](../PLAN.md#remaining-actions) and
[ledger](levelc-requirements.md#shared-requirements-before-instruction-closure)
retain clause clocks, configured conditions and SIGNAL ON/OFF, delayed CALL
handlers, practical TRACE, remaining expression/BIF/host/lifecycle contracts,
delivery and declared platforms. INTERPRET remains required and last. Accepted
profile choices do not close these obligations or establish full conformance.
