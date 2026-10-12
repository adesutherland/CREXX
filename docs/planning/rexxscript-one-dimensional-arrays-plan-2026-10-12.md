# RexxScript One-Dimensional String Arrays

## Vision and intended outcome

Give RexxScript scripts a small, practical one-dimensional string-array
facility without changing the host `REXXSCRIPT` contract or introducing the
more invasive bracket-expression parser. The first surface uses Rexx-style
compound names such as `measurements.1`, with `measurements.0` conventionally
holding the current element count. Numeric suffixes are stored as independent
string values, and a variable suffix such as `measurements.i` resolves `i` to
support ordinary loops.

The host boundary uses the existing `EXPOSE` clause. A host `.string[]`
variable is flattened into `name.0`, `name.1`, ... for RexxScript and is
reconstructed after execution. No additional `[]` marker is added to the
`EXPOSE` clause.

## Acceptance criteria

- **AC-01:** RexxScript accepts assignments to `array.0` and
  `array.<numeric-index>` and reads those values in expressions.
- **AC-02:** A variable suffix such as `array.i` resolves the current integer
  value of `i`, so a `DO i = 1 TO array.0` loop can read each element.
- **AC-03:** Array elements are string values and preserve arbitrary text;
  missing elements read as the existing undefined-variable error rather than
  silently becoming a scalar.
- **AC-04:** A declared host `.string[]` exposed through `EXPOSE name` is
  imported as `name.0`, `name.1`, ... and updated from the RexxScript result.
- **AC-05:** Existing scalar identifiers, intrinsic calls, OUTPUT, RESULT,
  and COLLECT behavior remain unchanged.
- **AC-06:** Focused runtime and compiler-exit tests cover literal indexes,
  dynamic indexes, string values, malformed suffixes, and host array
  round-trip behavior.
- **AC-07:** User and developer documentation describe the notation and the
  type-driven `EXPOSE` array boundary.

## Implementation steps

- **STEP-01:** Extend the RexxScript name parser and assignment validation to
  recognize one dotted suffix and normalize dynamic suffixes. Serves AC-01,
  AC-02, and AC-03.
- **STEP-02:** Extend the RexxScript compiler-exit to inspect declared host
  types, flatten `.string[]` input into dotted names, and copy returned dotted
  values back into the host array. Serves AC-04 and AC-05.
- **STEP-03:** Add focused functional coverage and documentation. Serves
  AC-06 and AC-07.
- **STEP-04:** Rebuild the RexxScript runtime and compiler-exit targets and
  run the focused no-opt/opt regressions. Serves AC-05 and AC-06.
