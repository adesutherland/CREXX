# RXAS Jump Table Working Design

Status: Slice 1 linear implementation in progress.

This note proposes the RXAS surface for a fast static multi-branch instruction.
The intended use is compiler-emitted dispatch for `select`, parser/token
classification, and other cases where a fixed set of literal values maps to
labels in one procedure.

The initial algorithm candidate is ACPH, reviewed from
<https://github.com/adesutherland/acph> at commit `c8b99aa`. If ACPH is used,
copy the relevant source into this repository and adapt it to cREXX coding and
serialization rules rather than linking the external project.

## ACPH Review Summary

ACPH is a byte-oriented, precomputed lookup structure. It selects discriminating
byte columns per node, hashes a byte at that column, and descends through a
small tree until a leaf is reached. The lookup then does a final full-key
compare before returning the stored value.

The useful properties for RXAS jump tables are:

- The table is immutable and can be built once at assemble time.
- Lookup input is bytes plus a known byte length, which matches binary constants
  and string UTF-8 bytes.
- Miss-heavy parser-style workloads are a good fit when selected columns
  discriminate well.
- The final full-key compare should remain in the safe general form to avoid
  false positives from malformed or unexpected input.

The caution from the ACPH review is equally important: ACPH is not automatically
better than generated `switch`, trie, DFA, gperf-style minimal perfect hash, or
open addressing for every table. The assembler should be able to choose a
strategy with `auto`, and the RXAS writer should be able to override that choice
with `.jtable`.

## Goals

- Provide a coherent RXAS syntax for static value-to-label dispatch.
- Encode the dispatch table as a binary constant-pool block.
- Let the VM branch directly to the selected label address without allocating a
  string, binary, or temporary register value.
- Keep miss behavior simple: no match falls through to the next instruction.
- Keep all label targets in the same procedure as the jump-table instruction.
- Produce deterministic assembler diagnostics for duplicate keys, unresolved
  labels, malformed table declarations, inconsistent table use, and use outside
  a procedure.

## Non-Goals

- No Rexx-level surface in this first slice.
- No runtime-built dynamic table in this first slice.
- No cross-module or cross-procedure jump table.
- No public dependency on the ACPH repository.
- No removal of ordinary branch or compare instructions.

## Proposed RXAS Surface

A jump table is declared inside a procedure and populated by decorating target
labels with case values:

```rxas
main() .locals=4
    .jtable keyword_table auto

    load r0,"while"

    jumps r0,keyword_table
    br not_keyword

tok_if:     .jcase keyword_table "if"
    load r1,1
    br done

tok_then:   .jcase keyword_table "then"
    load r1,2
    br done

tok_while:  .jcase keyword_table "while"
    load r1,3
    br done

not_keyword:
    load r1,0

done:
    ret
```

The table declaration is explicit in Release 1. This keeps the parser and
assembler diagnostics deterministic and makes the algorithm policy visible:

```rxas
.jtable keyword_table
.jtable keyword_table acph
```

The explicit form must appear inside the same procedure as its cases and uses.
`.jtable` does not declare the key type. The RXAS writer, or the compiler
emitting RXAS, is expected to know which `jump*` instruction will be used. The
optional `.jtable` operand selects the table algorithm:

| Algorithm | Meaning |
| --- | --- |
| `auto` | Assembler chooses the strategy. This is the default. |
| `linear` | Compact linear table, useful for very small tables and bring-up. |
| `openhash` | Open-addressed hash table. |
| `acph` | ACPH-derived packed table. |

The `jump*` instruction chooses how to turn the source register into lookup
bytes:

| Instruction | Case literal | Key bytes |
| --- | --- | --- |
| `jumps rKey,table` | string constant | UTF-8 bytes, no NUL terminator |
| `jumpb rKey,table` | binary constant | whole logical binary bytes |
| `jumpbs rSource,rOffset,table` | binary constant | fixed-length binary slice bytes without copying |
| `jumpi rKey,table` | integer literal | signed `.int` canonical little-endian bytes |

`jumpbs` deliberately remains a three-operand instruction. The slice length is
determined from the table's packed `key_length` field. Therefore `jumpbs` is
valid only for fixed-length binary-key tables where `key_length` is non-zero.
Variable-length binary dispatch should use `jumpb` over a binary register whose
logical length is already the lookup key length.

If the algorithm is omitted, the assembler uses `auto`. In the first executable
slice, `auto` lowers to `linear`. `openhash` and `acph` are accepted as syntax
but rejected during table building until their packed bodies and VM lookup paths
are implemented. Later `jump*` uses of the same table must be consistent with
the canonicalized case literal bytes.

The label decoration form is:

```rxas
label: .jcase table literal
```

`.jcase` does not emit an instruction and does not move the current instruction
address. Consecutive decorated labels may point at the same code address:

```rxas
tok_less:      .jcase operator_table "<"
tok_less_eq:   .jcase operator_table "<="
    load r1,42
    br done
```

The branch instruction forms are:

```rxas
jumps rKey,table
jumpb rKey,table
jumpbs rSource,rOffset,table
jumpi rKey,table
```

On match, the VM jumps to the label address encoded in the table. On miss, the
instruction falls through. A default branch is therefore written explicitly after
the jump:

```rxas
    jumps rToken,keyword_table
    br identifier_path
```

## Assembler Semantics

Jump table names need forward/backpatch semantics. They cannot be ordinary
`.const` aliases at parse time because the binary payload depends on final label
addresses.

The assembler should treat a table name as a procedure-local symbolic table:

1. `.jtable` creates a procedure-local table descriptor and records the
   algorithm policy.
2. `.jcase` adds a key and target label to that descriptor.
3. `jump* ... table` emits an operand placeholder for the table constant.
4. After normal label optimisation, and before ordinary label metadata is freed,
   the assembler resolves every `.jcase` target label to an instruction address.
5. The assembler builds the packed table as a `BINARY_CONST` payload.
6. The assembler patches every `jump*` table operand to the constant-pool index.

This is closer to label backpatching than to existing `.const` alias handling.
The table name should not be visible outside the containing procedure, and the
same table name may be reused in different procedures.

There is no key-type field in `.jtable`. The assembler still canonicalizes case
literals into bytes and checks that the table is used consistently by `jumps`,
`jumpb`/`jumpbs`, or `jumpi`.

### Diagnostics

The assembler should report:

- duplicate `.jcase` key in one table;
- `.jcase` before any current procedure;
- `.jcase` without a label on the same line;
- unknown or invalid `.jtable` algorithm;
- `jump*` using an unknown table;
- inconsistent `jump*` use, such as `jumps` against a table whose cases are
  binary constants;
- target label missing or outside the table's procedure;
- table with no cases;
- table too large for the selected packed offset/address width;
- internal builder failure, including duplicate binary keys after canonicalizing
  the source literals.

Malformed jump-table payloads are RXBIN/bytecode corruption, not bad user
arguments. Add a dedicated runtime signal such as `RXBIN_CORRUPTION` before
shipping the executable instruction. Validation should be designed so hot-path
lookups do not repeatedly pay for expensive structural checks.

## Packed Constant Format

The table is stored in a `BINARY_CONST`. Its payload must be independent of C
host layout. Do not serialize ACPH's `HashNode`, `HashSlot`, `size_t`, or
pointer-derived offsets directly.

All numeric fields inside the payload should be little-endian fixed-width
integers. The payload should contain only fields needed by the VM lookup. A
magic marker is intentionally omitted because this is a hot instruction path and
the operand is already an assembler-created constant.

A proposed common Release 1 header:

```text
u8     algorithm       1=linear, 2=openhash, 3=acph
u8     flags
u16    header_size
u32    key_length      fixed byte length, or 0 for variable-length keys
u32    case_count
...    strategy-specific node/slot/key/target data
```

The `jump*` opcode supplies the key interpretation, so the packed table does not
need a key-kind field. The binary-constant logical length supplies the payload
length, so the table also does not need a separate payload-size field.

Leaf targets are instruction addresses in the current procedure's binary space,
the same address form used by ordinary branch operands after label backpatching.
The VM sets:

```c
next_pc = current_frame->procedure->binarySpace->binary + target;
```

and dispatches normally.

The Release 1 linear body is:

- entry: key byte offset, key length, target instruction address;
- key blob: exact key bytes for final compare.

An ACPH-derived layout can then be added behind the same header:

- node header: selected column, prime/multiplier, slot count, first slot offset;
- slot: selected byte/end-symbol, state, child offset or leaf offset;
- leaf: key length, key byte offset, target instruction address;
- key blob: exact key bytes for final compare.

The end-of-key symbol used by ACPH is an internal table-building concept. The
serialized table should store it explicitly as a value outside the byte range,
for example `u16 256`, rather than overloading a byte.

## VM Lookup Rules

`jumps` reads the string slot of `rKey` as UTF-8 bytes and uses `string_length`
as the byte length. It does not append or expect a terminator.

`jumpb` reads the binary slot of `rKey` and uses `binary_length`.

`jumpbs` reads a byte slice from the binary slot of `rSource`, using `rOffset`
and the table's fixed `key_length`. It must not allocate or copy. Negative
offsets, variable-length tables, zero-length fixed tables, or slices outside the
logical binary length raise `OUT_OF_RANGE`.

`jumpi` reads the integer slot of `rKey` and canonicalizes it to eight
little-endian bytes. This keeps `.int` dispatch stable with the Release 1
direction that `.int` is 64-bit.

The VM lookup should:

1. trust the assembler-created table in the normal hot path, with cheap bounds
   checks where needed to avoid unsafe memory access;
2. derive the key byte pointer and length from the instruction variant;
3. traverse the encoded table;
4. verify slot character/end-symbol matches at each node;
5. compare the full key at the leaf;
6. jump on match, otherwise fall through.

The table must be read-only. The instruction never mutates the key register or
the table constant.

## Implementation Slices

1. Add RXAS parser support for `.jtable`, `.jcase`, and table-name operands.
   Status: implemented for explicit `.jtable` declarations and same-line label
   decorations.
2. Add assembler table collection, label-resolution checks, duplicate-key
   checks, and deterministic packed `BINARY_CONST` emission. Status:
   implemented for linear tables.
3. Add the `RXBIN_CORRUPTION` signal for malformed generated jump-table
   payloads. Status: implemented in the signal tables and VM.
4. Add `jumps`, `jumpb`, `jumpbs`, and `jumpi` opcodes and VM handlers using a
   simple linear packed-table strategy first. Keep all forms within the existing
   three-operand RXAS/RXBIN policy. Status: implemented for the interpreter and
   bytecode interpreter.
5. Replace or augment the packed-table builder with ACPH-derived construction
   and lookup. Keep the same RXAS syntax and binary header version if possible;
   otherwise bump the payload version.
6. Teach `rxc` to lower suitable `select`/multi-`if` shapes to `jump*`.
7. Benchmark real parser and JSON workloads against nested branches, binary
   search, open hash, trie/switch code, and ACPH jump tables.

## Decisions And Remaining Questions

- Release 1 should include `jumpb`; a `select` can conceptually be over a binary
  value even if the Rexx source surface does not expose that immediately.
- Release 1 should include zero-copy binary-slice dispatch as three-operand
  `jumpbs rSource,rOffset,table`; the table must provide a fixed key length.
- The assembler should support `auto` strategy selection and allow `.jtable` to
  override it with `linear`, `openhash`, or `acph`.
- Malformed jump-table payloads should raise a new RXBIN corruption signal, not
  `INVALID_ARGUMENTS`.
- `rxdas` should reconstruct readable `.jtable` and `.jcase` lines by parsing
  the jump-table payload. It does not need to preserve the original source table
  name exactly; it may synthesize a stable table name. It should always emit a
  `.jtable` line showing the algorithm used.
- Remaining question: should the source spelling for the open-addressed strategy
  be `openhash` for RXAS parser simplicity, or should RXAS accept the human
  spelling `open-hash` here as a directive option?
