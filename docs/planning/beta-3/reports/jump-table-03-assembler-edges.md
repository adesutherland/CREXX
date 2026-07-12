# Jump Table Report 3: Assembler Edge Coverage

Date: 2026-07-10

## Scope

Audit the RXAS jump-table grammar and assembler diagnostics, add missing
negative tests, and exercise practical Release 1 value and scope boundaries.

## Findings

- A top-level `.jtable` was rejected by the generic header error before the
  semantic procedure guard ran. The parser now gives the specific diagnostic
  that jump tables can only be declared inside a procedure.
- Table names are procedure-local. A use in another procedure is an unknown
  table even when that spelling was declared elsewhere; the same spelling may
  be independently declared in multiple procedures.
- `jumpbs` requires a non-empty fixed key width. The existing test covered
  mixed widths but not the zero-width boundary.
- Padded and numeric canonicalization can create duplicates not visible in
  source spelling. Signed zero is another numeric example and is now covered.
- Invalid UTF-8 case text reaches a dedicated assembler diagnostic when UTF-8
  validation is enabled.
- The omitted `.jtable` algorithm correctly means `auto`. Signed 64-bit
  integer limits and numeric infinities assemble and execute correctly.

## Coverage Added

Negative tests now cover:

- declaration outside a procedure;
- cases without a declaration;
- cross-procedure table use;
- zero-length `jumpbs` keys;
- invalid UTF-8 case text;
- signed-zero duplicate numeric keys.

A positive boundary fixture covers omitted `auto`, independent same-name
tables in two procedures, `INT64_MIN`/`INT64_MAX` dispatch, and positive and
negative infinity canonicalization.

## Defensive Guards

The following guards remain impractical to trigger with a bounded source test:
more than `UINT32_MAX` cases, a key blob or target address above 4 GiB, host
`size_t` arithmetic overflow, and an impossible ACPH construction after
duplicate elimination. They remain necessary memory-safety checks and are
reviewed structurally rather than by allocating multi-gigabyte fixtures.

The `unknown jump table case label` guard is also defensive: the grammar
requires `.jcase` to decorate the same label it records, so ordinary RXAS
cannot create an unresolved case label.

## Validation

Focused result: 22/22 assembler-negative and boundary VM tests passed.
