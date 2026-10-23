# RXPP free-form macro expansion plan

## Vision and intended outcomes

Allow an RXPP source to opt in to macro expansion inside free-form embedded
text blocks. This supports generated source for external languages and other
data-oriented blocks without changing the established default behavior of
`##DATA`, `##DALIAS`, and the other free-form collectors. The opt-in is a
source-level `##CFLAG blockmacros`; when absent, embedded lines remain
literal apart from the existing collector behavior.

## Acceptance criteria

- **AC-01:** `##CFLAG blockmacros` enables expansion of ordinary RXPP
  command macros and facade-to-script-macro calls while free-form text is
  collected.
- **AC-02:** Without `blockmacros`, existing free-form blocks retain their
  current literal-text behavior, including the existing `##DALIAS` behavior.
- **AC-03:** The opt-in applies consistently to the supported free-form
  collectors, with matching end-marker and callback behavior preserved.
- **AC-04:** The RXPP user documentation describes the flag, its default-off
  behavior, and a concise example.
- **AC-05:** Focused regressions cover enabled expansion and default-off
  preservation in optimized and non-optimized preprocessing paths.

## Implementation steps

- **STEP-01:** **Complete.** Add the `blockmacros` CFLAG to the RXPP
  documentation and plan the shared collection helper. Serves AC-01 through
  AC-04.
- **STEP-02:** **Complete.** Refactor free-form line collection so macro
  expansion is conditional on `rxpp_cflag_present('blockmacros')`, including
  the facade-to-script-macro second stage. Serves AC-01 through AC-03.
- **STEP-03:** **Complete.** Add focused fixtures for enabled and default-off
  behavior and run the corresponding optimized/no-opt regression targets.
  Serves AC-05.
- **STEP-04:** **Complete.** Update the external-interpreters example with the
  explicit opt-in, then compile and execute it with the available interpreters.
  Serves AC-01 and AC-04.

## Status

Implemented and locally validated with the approved `blockmacros` flag name.

Validation: `cmake --build cmake-build-release --target rxpp --parallel 1`
passed; `rxpp_dalias_opt` and `rxpp_dalias_noopt` passed; and
`examples/external_interpreters/simple_rxpp.rxpp` completed successfully with
Regina, Python, Node.js, and PowerShell.
