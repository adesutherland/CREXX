# RexxScript VALUE Intrinsic

## Vision and intended outcome

Allow RexxScript programs to read dynamically named scalar variables through a
sandboxed `VALUE(name)` intrinsic. This provides a bridge for RXPP-generated
scalar representations of host arrays while preserving the RexxScript
language limitation that it has no arrays, stems, or dynamic assignment.

## Acceptance criteria

- **AC-01:** `VALUE(name)` accepts one expression, resolves the resulting
  variable name case-insensitively, and returns its string value.
- **AC-02:** Missing, omitted, or invalid names produce a normal RexxScript
  diagnostic and do not access state outside the script variable pool.
- **AC-03:** Existing intrinsic behavior remains unchanged.
- **AC-04:** A focused runtime regression verifies dynamic lookup inside a loop,
  and documentation describes read-only semantics.

## Implementation steps

- **STEP-01:** Add `VALUE` to the intrinsic allow-list and implement read-only
  variable-pool lookup; serves AC-01 through AC-03.
- **STEP-02:** Add the loop regression and documentation; serves AC-04.
- **STEP-03:** Rebuild the RexxScript runtime and run its focused regression.

