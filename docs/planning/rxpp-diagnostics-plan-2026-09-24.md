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
