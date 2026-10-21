# RexxScript quote-aware string intrinsics

## Vision and intended outcome

RexxScript is intentionally a smaller, sandboxed language subset, but its
existing `WORD`, `WORDS`, and `POS` intrinsics cannot safely process quoted
argument sequences. Add the existing quote-aware `QWORD`, `QWORDS`, and `QPOS`
operations to RexxScript so macro bodies and embedded scripts can inspect
quoted text consistently with Level B cREXX, without enabling unrestricted
function dispatch or duplicating the established parsing implementation.

## Acceptance criteria

- **AC-01** — RexxScript accepts `QWORD(text, index)` and returns the existing
  quote-aware word, including embedded commas and spaces.
- **AC-02** — RexxScript accepts `QWORDS(text)` and returns the quote-aware word
  count.
- **AC-03** — RexxScript accepts `QPOS(needle, text [, start])` and searches
  outside quoted regions with the existing argument and error semantics.
- **AC-04** — Existing RexxScript intrinsic allow-list behavior remains
  sandboxed; no general namespace or arbitrary function dispatch is introduced.
- **AC-05** — The RexxScript user guide lists the new intrinsics and a focused
  regression verifies ordinary, quoted, and invalid-argument cases.

## Implementation steps

- **STEP-01** — Inspect the RexxScript intrinsic dispatcher and existing
  quote-aware Level B implementations. (AC-01, AC-02, AC-03, AC-04)
- **STEP-02** — Add safe dispatcher entries and any required adapter bindings,
  preserving the existing call-context and error behavior. (AC-01, AC-02,
  AC-03, AC-04)
- **STEP-03** — Add focused RexxScript functional coverage for quoted words,
  quoted delimiters, start positions, and invalid arguments. (AC-01, AC-02,
  AC-03, AC-05)
- **STEP-04** — Update the RexxScript user guide and record focused validation.
  (AC-04, AC-05)

## Status

STEP-01 is complete. The existing RexxScript dispatcher exposed the
non-quote-aware word and position family but omitted `QWORD`, `QWORDS`, and
`QPOS`; the Level B `rxfnsb` implementations provide the required quote-aware
behavior.

STEP-02 is complete. The legacy evaluator now explicitly allows the three
intrinsics and forwards them to the imported `rxfnsb` implementations. Their
`INVALID_ARGUMENTS` conditions are translated into evaluator diagnostics;
general namespace dispatch remains unavailable.

STEP-03 is complete in source. `testRexxScriptRuntime.crexx` covers quoted
words, quoted delimiters, a start position, and an unmatched-quote error.
STEP-04 is complete; the user guide documents the new intrinsic names and
quote-aware behavior.

## Validation

- `cmake --build cmake-build-release --target rexxscript testRexxScriptRuntime --parallel 1`
  passed, including both optimized and non-optimized test bytecode generation.
- `ctest --test-dir cmake-build-release --parallel 1 -R
  "^testRexxScriptRuntime_(noopt|opt)$" --output-on-failure` passed the
  non-optimized test. The optimized CTest wrapper could not start because the
  configured CLion-bundled `cmake.exe` was denied permission by the local
  environment.
- Running the unchanged generated optimized wrapper with the regular `cmake
  -P` command passed: `PASS: RexxScript runtime`.

All acceptance criteria are complete for the validated implementation.
