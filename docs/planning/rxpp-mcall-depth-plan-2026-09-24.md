# RXPP `.mcall` Nesting Limit Plan

## Vision and intended outcomes

Protect RXPP from runaway recursion through `.mcall` while preserving the
existing macro model and all non-`.mcall` expansion behavior. A script macro
that emits another `.mcall` should be able to nest normally up to a depth of
16. An attempted 17th level should stop with a clear RXPP diagnostic instead
of consuming the general expansion budget or leaving an unexplained failure.

This change intentionally does not detect or limit ordinary `##define`
expansion or `.gen ##macro` workarounds. The existing total script-macro
expansion safeguard remains in place as a broader fallback.

## Acceptance criteria

- **AC-01:** Nested `.mcall` expansion at depth 16 succeeds and preserves the
  existing generated-source behavior. Evidence: focused RXPP regression.
- **AC-02:** A generated `.mcall` at depth 17 is rejected with a dedicated,
  source-associated diagnostic. Evidence: focused recursion fixture and
  diagnostic assertion.
- **AC-03:** The depth travels with `.mcall`-generated source even after the
  parent script macro has returned. Evidence: nested `.mcall` regression.
- **AC-04:** Ordinary script macros, `##define`, and `.gen ##macro` behavior is
  unchanged by this guard. Evidence: existing RXPP smoke test and focused
  non-`.mcall` check.
- **AC-05:** User documentation describes the 16-level `.mcall` limit and its
  scope. Evidence: updated macro User's Guide and reference documentation.

## Implementation steps

- **STEP-01:** Inspect the script-macro output marker and source insertion path
  to identify a non-user-visible way to carry `.mcall` depth.
- **STEP-02:** Add depth propagation and reject depth greater than 16 with a
  dedicated RXPP diagnostic, preserving existing output for allowed calls.
- **STEP-03:** Add a standalone recursion fixture and focused test assertion
  without changing CMake integration.
- **STEP-04:** Update the macro User's Guide and detailed macro reference.
- **STEP-05:** Build RXPP and run the focused smoke/regression checks; record
  any unrelated pre-existing failures separately.

## Status and evidence

- **AC-01:** Verified by `preprocessor/tests/testx_mcall_depth_ok.rxpp`;
  exactly 16 nested `.mcall` expansions complete successfully.
- **AC-02:** Verified by `preprocessor/tests/testx_mcall_depth.rxpp`;
  depth 17 emits `RXPP_MCALL_DEPTH`.
- **AC-03:** Verified by both standalone fixtures; depth is carried through
  source insertion after the parent macro returns.
- **AC-04:** `rxpp_smoke` passes.
- **AC-05:** Updated `preprocessor/rxpp-macro-users-guide.md` and
  `preprocessor/macro-facility.md`.
- RXPP Release build target passes with the existing two unrelated Level B
  scope warnings in `rxpp.crexx`.
