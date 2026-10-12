# RXPP REXXSCRIPT Free-Form Block

## Vision and intended outcome

Allow a cREXX author to write a multiline RexxScript program directly in an
RXPP source file without manually assembling its script text. The feature is
only an RXPP source-packaging convenience: existing `REXXSCRIPT` execution,
variable exposure, output capture, and result handling remain owned by the
existing compiler exit and runtime.

## Acceptance criteria

- **AC-01:** `REXXSCRIPT RUN ... END REXXSCRIPT` collects the body in order and
  emits one ordinary `REXXSCRIPT` statement using the generated script text.
  **Satisfied:** focused generation output contains the ordered generated
  string assembly and one runtime statement.
- **AC-02:** The block preserves quotes, punctuation, blank lines, and multiple
  source statements without leaking the block markers into generated cREXX.
  **Satisfied:** the focused block regression checks body text, marker removal,
  and statement ordering.
- **AC-03:** Existing one-line `REXXSCRIPT` syntax is unchanged.
- **Status:** unchanged one-line syntax is outside the collector path.
- **AC-04:** A focused RXPP regression verifies collection and generated
  execution syntax, and the generated source is accepted by `rxc`.
  **Satisfied:** `run_rxpp_rexxscript_block.cmake` completed successfully.

## Scope revision (2026-10-12)

The RXPP-specific `C$RX EXPORT COLLECT` bridge is removed from this phase.
Native RexxScript `COLLECT`/`VALUES` semantics remain unchanged; real array
support is a separate future change and will be designed and tested there.

## Implementation steps

- **STEP-01:** Add the RXPP collector and generated script-string assembly;
  serves AC-01 and AC-02. **Complete.**
- **STEP-02:** Add a focused block sample/regression and documentation;
  serves AC-03 and AC-04. **Complete.**
- **STEP-03:** Rebuild RXPP and compile the generated sample; retain the
  generated-source inspection as evidence for AC-01 through AC-04.
  **Complete:** Release RXPP rebuild and focused RXPP→rxc regression passed.
