# RXBIN Packed Jump Tables

This document is the durable internal contract for assembler-built jump-table
constants. User-facing RXAS syntax and instruction behavior are documented in
`docs/reference/rxas/program-syntax.md` and
`docs/reference/rxas/instructions/01-program-control.md`.

## Ownership

- `rxas` collects procedure-local `.jtable`/`.jcase` declarations after
  label optimization and writes one immutable `BINARY_CONST`.
- `rxvm` and `rxbvm` read the packed payload directly. Lookup does not
  allocate or copy the source key.
- `rxdas` decodes the payload, synthesizes a stable table name and case
  labels, and emits reassemblable RXAS.
- Shared constants, little-endian helpers, hashes, and `auto` policy belong in
  `binutils/include/rxjtable.h`.

The implementation is adapted from the byte-column discrimination approach
reviewed in `adesutherland/acph` at commit `c8b99aa`. CREXX does not link
that project or serialize its C structures.

## Common Header

Every payload is host-layout-independent and begins:

```text
u8  algorithm       1=linear, 2=openhash, 3=acph
u8  flags           zero in Release 1
u16 header_size
u32 key_length      fixed byte length, or zero for variable length
u32 case_count
```

All integers are little-endian. Offsets are absolute byte offsets from the
start of the payload. Targets are instruction addresses in the containing
module binary space. The enclosing `BINARY_CONST` logical length is the
payload length.

The opcode defines key interpretation: `jumpi` uses a signed eight-byte
little-endian integer; `jumps` exact UTF-8; `jumpr` trailing-space
canonical UTF-8; `jumpn` a canonical eight-byte IEEE-754 key; and
`jumpb`/`jumpbs` exact bytes. Numeric signed zero is folded. NaN source
values use an internal alias to the first case; `rxdas` omits that alias
because `rxas` recreates it. NaN case literals are rejected.

## Linear Body

The common header is followed by `case_count` 12-byte entries and then the
key blob:

```text
u32 key_offset
u32 key_length
u32 target
```

## Open-Hash Body

The 16-byte header adds a power-of-two `u32 slot_count`. Each immutable
16-byte slot contains:

```text
u32 fnv1a_hash
u32 key_offset       UINT32_MAX means empty
u32 key_length
u32 target
```

Tables are built at no more than 50% load. Lookup uses linear probing and an
exact length/byte comparison before returning a target.

## ACPH-Derived Body

The 16-byte header adds the root-node offset. Each node contains:

```text
u32 selected_column
u16 slot_count
u8  prime
u8  reserved
```

Each eight-byte node slot contains:

```text
u16 symbol           byte value, or 256 for end of key
u8  kind             0=empty, 1=leaf, 2=child
u8  reserved
u32 value_offset     leaf-record or child-node offset
```

A 12-byte leaf record contains key offset, key length, and target. Lookup
checks the selected symbol at every node and always performs a final exact key
comparison at a leaf.

## Validation And Corruption

`rxas` rejects duplicate canonical keys, inconsistent source modes,
variable/empty `jumpbs` widths, empty tables, unresolved procedure-local
symbols, and values outside packed widths.

The VM uses cheap bounds and structural checks while traversing. Invalid
algorithms, headers, node/slot kinds, key offsets, leaf records, or matched
targets raise `RXBIN_CORRUPTION`. Entry-local corruption that is never
traversed need not be diagnosed eagerly.

Post-assembly mutation tests cover all three algorithms in both VMs. `rxdas`
round-trip tests cover all six jump forms and re-execute reconstructed modules
under both VMs.

## Algorithm Policy

Release 1 `auto` selection is:

| Shape | Algorithm |
| --- | --- |
| One case | `linear` |
| At least two cases, average key length at most two bytes | `openhash` |
| 2-255 cases, longer average keys | `acph` |
| At least 256 cases, average key length three or four bytes | `openhash` |
| At least 256 cases, average key length above four bytes | `acph` |

This chooses the packed lookup algorithm only. The compiler separately decides
whether a source ladder is profitable enough to become a table.
