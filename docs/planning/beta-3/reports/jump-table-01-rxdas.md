# Jump Table Report 1: Disassembler Reconstruction

Date: 2026-07-10

## Scope

Close the Release 1 tooling gap where `rxdas` rendered a packed jump-table
constant as inline binary, producing RXAS that could not be assembled again.

## Findings

- A jump table has no source-level name in RXBIN. A stable name must therefore
  be synthesized from its constant-pool offset.
- Targets are packed instruction addresses. Reconstruction must create a
  synthetic `.jcase` label at every target, including multiple cases sharing
  one address.
- Open-hash and ACPH storage order need not match source order. Distinct keys
  remain semantically equivalent because every case maps directly to a target.
- `jumpn` includes an internal canonical NaN entry targeting the first source
  case. This entry is not legal RXAS source and must be omitted; `rxas` recreates
  it when rebuilding the numeric table.
- A table's key interpretation comes from the referencing `jump*` opcode, not
  from its packed payload. Reconstruction must therefore correlate code and
  constant-pool data.

## Result

`rxdas` now validates and decodes linear, open-hash, and ACPH payloads, emits
procedure-local `.jtable` declarations using the packed algorithm, emits
synthetic `.jcase` labels, and names table operands in all six jump forms.
Malformed or inconsistent payloads are left in the existing raw-binary form
instead of being dereferenced unsafely.

## Validation

Three assemble-disassemble-reassemble tests cover:

- linear `jumps`, `jumpb`, `jumpbs`, and `jumpi`;
- explicit open-hash and ACPH tables;
- `jumpr` and `jumpn`, including suppression and recreation of the NaN alias;
- equivalent output and exit status under both `rxvm` and `rxbvm`.

Focused result: 3/3 jump-table round-trip tests passed.
