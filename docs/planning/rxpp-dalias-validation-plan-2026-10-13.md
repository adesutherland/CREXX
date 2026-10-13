# RXPP DALIAS validation plan

## Vision and intended outcomes

Make `##DALIAS` safe for the planned bare-block syntax by validating every
alias when it is declared. Invalid identifiers, reserved REXX/RXPP words, and
duplicate registrations must fail immediately with a useful diagnostic rather
than later being misinterpreted as ordinary source.

## Acceptance criteria

- **AC-01:** Valid aliases such as `FORTH` are registered case-insensitively
  and continue to support both `##FORTH ... ##END` and the planned bare form.
- **AC-02:** Invalid alias spelling is rejected at the `##DALIAS` declaration.
- **AC-03:** Reserved REXX keywords and RXPP directive names are rejected at
  declaration time.
- **AC-04:** Duplicate aliases are rejected at declaration time.
- **AC-05:** Existing valid DALIAS behavior passes in both optimized and
  non-optimized focused tests.

## Implementation steps

- **STEP-01:** Add declaration-time alias validation using RXPP's identifier
  character rules and reserved-name table. Serves AC-02 through AC-04.
- **STEP-02:** Add focused valid and invalid DALIAS regression cases. Serves
  AC-01 and AC-05.
- **STEP-03:** Update RXPP reference documentation with the validation rules
  and run the focused tests. Serves AC-01 through AC-05.
