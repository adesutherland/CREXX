# Jump Table Report 2: Runtime Corruption Coverage

Date: 2026-07-10

## Scope

Prove that malformed assembler-built jump-table payloads are treated as RXBIN
corruption by both VM implementations, without relying on assembler rejection
or a duplicate unit-test decoder.

## Findings

- The useful boundary is an assembled RXBIN that is mutated after assembly.
  Invalid RXAS cannot reach the VM and therefore cannot exercise this contract.
- Structural faults such as an invalid algorithm, header, key offset, or ACPH
  slot kind are detected while traversing the table.
- A corrupt target is intentionally checked only after its key matches. Tests
  must mutate the entry selected by the fixture; damaging an unrelated entry
  can legitimately leave the lookup result unchanged.
- The bytecode and threaded VMs share the same validation behavior for every
  tested corruption.

## Result

A test-only RXBIN mutator now reads a normal assembled module, locates its packed
jump-table constant, applies one controlled mutation, and writes a structurally
valid RXBIN container containing a corrupt table. Seven mutations cover:

- invalid algorithm and common header;
- linear key offset and matched target;
- open-hash matched-slot key offset;
- ACPH matched-slot kind and matched target.

Every fixture is executed by both `rxvm` and `rxbvm` and must report
`RXBIN_CORRUPTION`.

## Validation

Focused result: 14/14 VM corruption tests passed.

The first test run usefully failed four cases because the mutator damaged a
non-selected entry. The mutator was corrected to locate the exercised
`"beta"` key before changing entry-local offsets or targets, preventing a false
positive test.
