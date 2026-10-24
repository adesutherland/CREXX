# RXPP Diagnostic Flags Plan

## Vision and intended outcomes

RXPP already exposes buffer, variable, include-list, macro-list, and conditional
link diagnostics, but debugging a generated source problem still requires
inferring what happened between those snapshots. Add four opt-in diagnostic
flags—`macrotrace`, `iftrace`, `includetrace`, and `timing`—so developers can
observe macro calls, conditional decisions, include resolution, and phase
durations directly from RXPP.

These flags are diagnostics only. They must write only to RXPP's diagnostic
output and must not modify generated CREXX, alter macro expansion, change
conditional selection, change include resolution or deduplication, or add
runtime statements. Existing RXPP behavior remains unchanged when the flags
are absent.

## Acceptance criteria

- **AC-01**: [x] `macrotrace` reports each expanded script or source macro with its
  name and source location when enabled, while generated output remains byte
  for byte equivalent apart from existing nondeterministic source-map data.
- **AC-02**: [x] `iftrace` reports each evaluated `##IF`/`##IFN` decision and the
  selected/skipped result without changing conditional output.
- **AC-03**: [x] `includetrace` reports include resolution and duplicate-suppression events while
  preserving existing include contents, ordering, and duplicate handling.
- **AC-04**: [x] `timing` reports pass and total durations without changing the
  generated source or normal diagnostics when disabled.
- **AC-05**: [x] With none of the new flags present, existing RXPP focused tests
  pass and generated output is unchanged.
- **AC-06**: [x] Focused regression coverage proves the four flags emit diagnostics
  and that diagnostic output does not enter the generated CREXX file.
- **AC-07**: [x] The RXPP language reference documents the flags as compile-time,
  diagnostic-only controls and records that exact-word matching is used.

## Implementation steps

- **STEP-01**: [x] Inspect existing RXPP pass, macro, and include boundaries and
  choose diagnostic insertion points without changing data transformations.
  Serves AC-01 through AC-05.
- **STEP-02**: [x] Add exact-word flag checks and diagnostic output for macro,
  conditional, include, and timing events. Serves AC-01 through AC-05.
- **STEP-03**: [x] Add standalone `testx*.rxpp` fixtures for each flag and
  verify diagnostic output separately from generated output. Serves AC-05 and
  AC-06.
- **STEP-04**: [x] Update the RXPP reference documentation and verify whitespace,
  build, and focused tests. Serves AC-06 and AC-07.

## Evidence

- `cmake --build cmake-build-release --target rxpp --parallel 1` passed.
- Standalone `testx_macrotrace.rxpp`, `testx_iftrace.rxpp`,
  `testx_includetrace.rxpp`, and `testx_timing.rxpp` each ran successfully
  against the rebuilt `rxpp` executable. Their diagnostics remained outside
  generated CREXX output.
- The existing CMake-based `rxpp_smoke` test script was not changed.
- `git diff --check` passed.

## Follow-up: `flow` and `stats` diagnostics (2026-10-23)

### Vision and intended outcomes

Make RXPP's own preprocessing decisions understandable without requiring a
developer to compare large intermediate buffers manually. `##CFLAG stats`
will report a compact end-of-run summary of the work performed. `##CFLAG
flow` will report structured, compile-time-only events for the principal
directive, conditional, include, macro, suppression, and pass-boundary
decisions. Both controls are source-local and require no new RXPP command-line
parameters. Neither may alter generated CREXX, runtime behavior, or existing
diagnostics when absent.

### Acceptance criteria

- **AC-08**: [x] `stats` emits a stable `RXPP-STATS` summary only when selected,
  covering pass completion and useful counts for directives, includes, macros,
  suppressed lines, and generated output.
- **AC-09**: [x] `flow` emits stable `RXPP-FLOW` records for pass boundaries and
  the implemented directive/conditional/include/macro/suppression decisions,
  with source location where available.
- **AC-10**: [x] `flow` and `stats` never enter generated CREXX and do not alter
  preprocessing results when absent or present.
- **AC-11**: [x] Focused tests verify both flags, their exact-word behavior, and
  the separation of diagnostics from generated output.
- **AC-12**: [x] The RXPP user guide and language reference document the flags and
  their diagnostic-only semantics.

### Implementation steps

- **STEP-05**: [x] Add shared counters and a small flow-emission helper gated by
  exact-word `##CFLAG` checks. Serves AC-08 through AC-10.
- **STEP-06**: [x] Instrument pass boundaries and existing directive, conditional,
  include, macro, suppression, and output paths without changing decisions.
  Serves AC-08 through AC-10.
- **STEP-07**: [x] Add focused source/test-script coverage and run the RXPP build
  plus focused tests. Serves AC-11.
- **STEP-08**: [x] Update both RXPP references and retain the observed validation
  command/results here. Serves AC-12.

### Follow-up evidence

- `cmake --build cmake-build-release --target rxpp --parallel 1` passed.
- Focused `ctest` passed: `rxpp_smoke`, `rxpp_diagnostics`, and
  `rxpp_flow_stats` (the required `sanitizer_fixture_integrity` prerequisite
  also passed).
- `rxpp_flow_stats` verified that `RXPP-FLOW` and `RXPP-STATS` remain outside
  generated CREXX and that macro/conditional output is preserved.

## Follow-up: compile-time `##ASSERT` (2026-10-24)

### Vision and intended outcomes

Allow an RXPP source file to validate its compile-time configuration before
macro expansion and generated-source compilation proceed. `##ASSERT` will
support the existing presence semantics for bare variables and exact string
comparisons using `==` and `!=`. Failed assertions will produce a stable RXPP
diagnostic and stop preprocessing; successful assertions and their messages
will never enter generated CREXX.

### Acceptance criteria

- **AC-13**: [x] A bare `##ASSERT name, "message"` passes only when the RXPP
  variable exists, including variables established with `##SET` and built-in
  RXPP variables.
- **AC-14**: [x] `##ASSERT name == "value", "message"` and `!=` compare the
  current RXPP variable value exactly.
- **AC-15**: [x] Failed assertions emit `RXPP_ASSERT_FAILED` and terminate
  preprocessing; successful assertions are suppressed from generated CREXX.
- **AC-16**: [x] Focused tests cover success, failure, `##UNSET`, built-in
  `platform`, and generated-source separation without rebuilding the full
  product in this phase.
- **AC-17**: [x] RXPP references and diagnostic catalogs document the new
  directive and its supported expression forms.
- **AC-18**: [x] `##IF` and `##IFN` accept the same bare-name, `==`, and `!=`
  condition forms as `##ASSERT`, while preserving bare-name existence tests.

### Implementation steps

- **STEP-09**: [x] Add `CMD_assert` and dispatch it during pass three so
  preceding `##SET`/`##UNSET` directives are honored. Serves AC-13 through AC-15.
- **STEP-10**: [x] Add focused CMake-script coverage and catalog entries.
  Serves AC-13 through AC-17.
- **STEP-11**: [x] Update RXPP references and record deferred build/test
  commands for the user's ongoing full build. Serves AC-16 and AC-17.
- **STEP-12**: [x] Share the condition evaluator between `##ASSERT`, `##IF`,
  and `##IFN`, preserving compound conditions during `##ELSE` normalization.
  Add focused coverage and documentation. Serves AC-18.

### Current evidence

- Source, tests, documentation, and diagnostic catalog edits are complete.
- `git diff --check` passed for the touched paths.
- RXPP rebuilt successfully after the final implementation changes.
- Focused command: `ctest --test-dir cmake-build-release --parallel 1 -R
  "^(rxpp_unset|rxpp_assert|rxpp_flow_stats)$" --output-on-failure`.
- Result: 100% passed (4/4, including `sanitizer_fixture_integrity`) on
  2026-10-24; this verifies ASSERT, IF/IFN value comparisons, UNSET behavior,
  and flow/stats integration.

## Follow-up: compile-time diagnostic directives (2026-10-24)

### Vision and intended outcomes

Give RXPP source authors explicit informational, warning, and fatal diagnostic
directives for documenting or rejecting selected compile-time configurations.
`##INFO` and `##WARN` continue preprocessing, while `##ERROR` terminates it.
All three use the existing catalog-based diagnostic path, preserve source
locations, and are omitted from generated CREXX.

### Acceptance criteria

- **AC-19**: [x] `##INFO "message"` emits an informational RXPP diagnostic and
  preprocessing continues.
- **AC-20**: [x] `##WARN "message"` emits a warning RXPP diagnostic and
  preprocessing continues.
- **AC-21**: [x] `##ERROR "message"` emits an error RXPP diagnostic and
  preprocessing terminates with the established failure status.
- **AC-22**: [x] Diagnostic directives are suppressed from generated CREXX and
  have focused success/failure regression coverage.
- **AC-23**: [x] RXPP references and all supported diagnostic catalogs document
  `##INFO`, `##WARN`, and `##ERROR`.
- **AC-24**: [ ] Explicit RXPP diagnostics use recognizable severity-specific
  prefixes, with failed ASSERT diagnostics using `++RXPP-DIAG` and retaining
  their stable diagnostic code.

### Implementation steps

- **STEP-13**: [x] Add a shared diagnostic-directive handler and pass-3
  dispatch, including reserved-word registration. Serves AC-19 through AC-22.
- **STEP-14**: [x] Add catalog entries, focused test coverage, and update RXPP
  documentation. Serves AC-19 through AC-23.
- **STEP-15**: [ ] Apply the agreed diagnostic output prefixes and verify them
  in focused tests. Serves AC-24.

### Current evidence

- Scope and semantics approved by the user; implementation and validation open.
- RXPP build completed successfully before validation.
- Focused command: `ctest --test-dir cmake-build-release --parallel 1 -R
  "^(rxpp_diagnostic_directives|rxpp_unset|rxpp_assert|rxpp_diagnostics|rxpp_flow_stats)$"
  --output-on-failure`.
- Result: 100% passed (6/6, including `sanitizer_fixture_integrity`) on
  2026-10-24. This verifies INFO/WARN continuation, ERROR termination,
  suppression from generated CREXX, localized diagnostic integration, and the
  previously implemented ASSERT/UNSET/flow/stats behavior.
- The diagnostic-prefix renderer/catalog cleanup was applied after that test
  run; AC-24 remains open until the user reruns the focused validation.

## Follow-up: scripted-macro `.mnote` (2026-10-24)

### Vision and intended outcomes

Allow RexxScript-backed RXPP macros to report compile-time notes as readable
comments in generated CREXX. `.mnote` will evaluate one RexxScript expression
and emit a valid generated comment `/* MNOTE ##<current-macro>: <message> */`. It will work alongside
`.mcall`, preserve nested macro context by using the currently executing macro
name, and leave macro return status and generated source unchanged.

### Acceptance criteria

- **AC-25**: [ ] RexxScript accepts `.mnote expression` at the same statement
  level as `.mcall` and evaluates the expression using the current macro state.
- **AC-26**: [ ] RXPP emits `/* MNOTE ##<macro>: <message> */` for each executed note,
  using the currently executing macro name, including nested scripted macros.
- **AC-27**: [ ] `.mnote` produces only its generated comment, no macro call or
  status change, and missing expressions fail consistently with other output forms.
- **AC-28**: [ ] Focused regression coverage and RexxScript/RXPP documentation
  describe the new statement and its output behavior.

### Implementation steps

- **STEP-16**: [ ] Add a dedicated RexxScript note record and evaluator
  statement beside `.mcall`, without treating it as generated output.
- **STEP-17**: [ ] Consume note records in RXPP's scripted-macro expansion and
  add focused tests/documentation. Serves AC-25 through AC-28.

### Current evidence

- Design approved by the user; implementation and validation open.
