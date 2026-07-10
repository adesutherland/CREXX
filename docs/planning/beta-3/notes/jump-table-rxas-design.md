# RXAS Jump Table Working Design

Status: Release 1 RXAS surface and all three packed lookup algorithms implemented;
`rxc` lowering is in progress.

This note defines the RXAS surface for a fast static multi-branch instruction.
The intended use is compiler-emitted dispatch for `select`, parser/token
classification, and other cases where a fixed set of literal values maps to
labels in one procedure.

The ACPH algorithm was reviewed from <https://github.com/adesutherland/acph> at
commit `c8b99aa`. The cREXX implementation is an internal packed-table builder
and VM reader adapted to RXBIN serialization rules; it does not link the
external project or serialize its C structures.

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
open addressing for every table. The assembler therefore chooses a measured
strategy with `auto`, and the RXAS writer can override that choice with
`.jtable`.

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

If the algorithm is omitted, the assembler uses `auto`. Explicit `linear`,
`openhash`, and `acph` tables are available for repeatable measurement and
tuning. Later `jump*` uses of the same table must be consistent with the
canonicalized case literal bytes.

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

The common Release 1 header is:

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

The open-hash body adds a `u32` power-of-two slot count after the common header.
Each immutable 16-byte slot stores the FNV-1a hash, key offset, key length, and
target. `UINT32_MAX` in the key-offset field marks an empty slot. Tables are
built at no more than 50% load, and lookup uses linear probing followed by an
exact key comparison.

The ACPH-derived body uses:

- table header extension: root-node offset;
- node header: selected column, prime/multiplier, and slot count;
- slot: selected byte/end-symbol, state, and child or leaf offset;
- leaf: key byte offset, key length, and target instruction address;
- key blob: exact key bytes for final compare.

The end-of-key symbol is stored explicitly as `u16 256`, outside the byte
range. All offsets are absolute payload offsets, and the final leaf always does
an exact length and byte comparison.

## Auto Selection Profile

The policy was profiled on 2026-07-10 using the Release `rxas` and `rxvm` on
Darwin ARM64. Generated real RXAS tables measured the resulting `jumpb`
instruction for hits, misses, and balanced hit/miss loops. The matrix covered
randomish and common-prefix keys, one to 2048 cases, and key lengths from one to
32 bytes. Reported comparisons used the median of three to five runs around the
decision boundaries.

The resulting Release 1 rule is:

| Table/key shape | `auto` choice |
| --- | --- |
| One case | `linear` |
| Two or more cases, average key length 0-2 bytes | `openhash` |
| 2-255 cases, average key length above 2 bytes | `acph` |
| 256 or more cases, average key length 3-4 bytes | `openhash` |
| 256 or more cases, average key length above 4 bytes | `acph` |

Linear won consistently for a one-case hit and lost decisively as the table
grew. Open hashing was the safer choice for one- and two-byte keys, especially
for successful lookups. ACPH usually won from three-byte keys upward because it
can reject on one selected symbol and avoids hashing every byte before the full
leaf comparison. At 256 cases and above, three- and four-byte keys were close,
but open hashing was the more stable hit-oriented choice. Assembler construction
times were similar after process-startup noise and did not justify another
policy input. The policy deliberately uses only data available to the assembler
and can be retuned without changing RXAS source.

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
   implemented for linear, open-hash, and ACPH-derived tables.
3. Add the `RXBIN_CORRUPTION` signal for malformed generated jump-table
   payloads. Status: implemented in the signal tables and VM.
4. Add `jumps`, `jumpb`, `jumpbs`, and `jumpi` opcodes and VM handlers using a
   simple linear packed-table strategy first. Keep all forms within the existing
   three-operand RXAS/RXBIN policy. Status: implemented for the interpreter and
   bytecode interpreter.
5. Add open-hash and ACPH-derived construction and lookup, then profile and
   implement `auto`. Status: implemented and profiled with the common Release 1
   header.
6. Teach `rxc` to lower suitable `select`/multi-`if` shapes to `jump*`.
   Status: implementation is proceeding through the compiler stages below.
7. Benchmark real parser and JSON workloads against nested branches, binary
   search, open hash, trie/switch code, and ACPH jump tables. Status: the
   generated key-size/table-size jump-table matrix is complete; compiler
   lowering and real workload comparisons follow slice 6.

## Decisions And Remaining Questions

- Release 1 should include `jumpb`; a `select` can conceptually be over a binary
  value even if the Rexx source surface does not expose that immediately.
- Release 1 should include zero-copy binary-slice dispatch as three-operand
  `jumpbs rSource,rOffset,table`; the table must provide a fixed key length.
- The assembler supports measured `auto` strategy selection and allows
  `.jtable` to override it with `linear`, `openhash`, or `acph`.
- Malformed jump-table payloads should raise a new RXBIN corruption signal, not
  `INVALID_ARGUMENTS`.
- `rxdas` should reconstruct readable `.jtable` and `.jcase` lines by parsing
  the jump-table payload. It does not need to preserve the original source table
  name exactly; it may synthesize a stable table name. It should always emit a
  `.jtable` line showing the algorithm used.
- The canonical RXAS spelling is `openhash`.

## `rxc` Integration Design

Jump-table selection is primarily an `rxc` optimization. At this stage the
compiler still knows the source construct, resolved symbols, types, folded
constants, and evaluation rules. RXAS-level recognition of arbitrary branch
ladders is deferred until RXAS has a control-flow graph plus reaching-definition
and liveness analysis; the current keyhole window is not a safe substitute for
those proofs.

Both source SELECT forms retain their language semantics:

- classic `select` evaluates independent Boolean `when` expressions;
- C-style `select expression` evaluates the selector once and compares it to
  each `when` expression using `=` semantics.

The existing early control-flow rewrite remains in place. It creates a private
typed selector temporary for C-style SELECT, then a canonical nested equality
ladder. The generated block is marked with explicit SELECT provenance. After
validation, common lowering recognizes that compiler-owned shape without making
all validation and symbol passes understand a new source control-flow node.
Optimized builds may first expose additional constants through inlining and
folding, but explicit eligible C-style SELECT is lowered in no-opt builds too.

Eligible ladders are rewritten to a dedicated internal dispatch AST. The
optimizer describes dispatch; it does not emit RXAS text. The normal emitter
owns `.jtable`, `jump*`, `.jcase`, fallback, and join-label generation. This
same internal representation is the future target for safe classic-SELECT and
general IF-ladder recognition.

### Compiler Stages

1. **Integer C-style SELECT.** In both optimized and no-opt builds, lower an
   explicit C-style SELECT when its selector is scalar `.int`, every case is a
   compile-time `.int` value, and the ladder has enough cases to justify a
   table. Emit `jumpi` and `.jtable ... auto`. Keep the original IF ladder for
   every rejected candidate. No-opt lowering recognizes typed source literals
   and explicit language constants without enabling general constant-folding
   optimization. The initial profitability threshold is conservative and
   remains subject to branch-ladder benchmarking.
2. **Mixed C-style SELECT.** Partition the ordered ladder into maximal
   consecutive eligible runs. Emit a jump table for profitable constant runs
   and retain ordinary IF comparisons for dynamic or unsupported cases. Never
   combine cases across an intervening expression whose evaluation order could
   be observed. Status: implemented for integer selectors in optimized and
   no-opt builds. Runs are lowered from the tail so every table miss retains
   the original dynamic comparison, side effects, and subsequent fallback.
3. **General integer IF recognition.** Reuse the dispatch builder for canonical
   nested IF ladders, including classic SELECT after its normal rewrite. The
   initial proof requires equality tests over the same resolved scalar variable,
   constant right operands, and no mutation, alias, call, getter, or other
   side-effect risk. Status: implemented for optimized builds. Loose and strict
   integer equality are accepted for direct scalar variable reads. Exposed,
   global, reference-target, reference-argument, class-attribute, indexed,
   dereferenced, property, call, non-integer, and undersized shapes remain
   ordinary comparisons. Safe constant runs may be lowered on either side of
   an unsupported condition because each run reads the selector at its original
   entry point. No-opt builds preserve general IF and classic SELECT source
   flow; their dispatch lowering remains limited to explicit C-style SELECT.
4. **Exact strings and binary.** Strict string comparisons can use `jumps`
   because `==` is exact after compiler string conversion and normalization.
   Exact binary dispatch can use `jumpb` once the Rexx binary equality path is
   validated end to end.
5. **Loose nonnumeric strings.** Add a distinct Rexx/padded-string jump form.
   The VM reduces the source's effective length over trailing ASCII spaces
   without copying, while assembler case keys are stored with the same trailing
   spaces removed. Leading spaces remain significant. Compiler-provided Unicode
   normalization means equal text has the same normalized UTF-8 representation
   before this operation. This optimization is valid when every case key is
   provably nonnumeric: loose comparison must then use padded text comparison
   regardless of the selector's runtime contents.
6. **Loose numeric strings.** After the numeric comparison contract is
   centralized, parse the selector once and dispatch using the same canonical
   numeric representation used for case constants. Equivalent spellings map to
   one key and retain the first source case. Numeric context, signed zero,
   non-finite values, and parser consistency must be settled before this form is
   enabled.
7. **RXAS flow optimization.** Once CFG/dataflow support exists, consider the
   same transformation for hand-written RXAS and compiler shapes exposed only
   after emission. Correctness must not depend on source metadata.

### Ordering, Duplicates, And Fallback

SELECT remains first-match-wins. A complete table preserves source case order
only through its target mapping; duplicate canonical keys must map to the first
case. During initial compiler bring-up, duplicate or otherwise ambiguous input
may conservatively retain the IF ladder rather than reaching the assembler,
which correctly rejects duplicate `.jcase` keys in hand-written RXAS.

Mixed lowering preserves observable order by grouping only consecutive eligible
runs. A jump miss falls through to the next residual comparison or to the
SELECT `otherwise` path. A match executes its original body and then branches
to the common SELECT join.

### Validation And Performance

Compiler coverage must include jump-table RXAS in optimized and no-opt builds,
runtime first, middle, and last hits, misses, `otherwise`, missing `otherwise`,
nested SELECTs, duplicate
fallback, mixed unsupported cases, and source/trace metadata. Negative safety
tests must prove that side-effecting or type-incompatible ladders are not
rewritten.

The assembler's `auto` profile chooses the packed table algorithm; it does not
choose between a table and an IF ladder. `rxc` therefore needs a separate
benchmark matrix comparing fused branches with `jumpi`/string dispatch over
case counts, hit positions, misses, and representative key distributions.

### Compiler Defects Found During Integration

Compiler defects exposed by this work are recorded here and receive immediate
regression coverage; they must not be hidden by weakening a jump-table test.

- **Repeated call-argument register corruption (found 2026-07-10).** A no-opt
  call such as `nested(i, i)` emitted two destructive `swap` operations from
  the same source register. The first swap changed the source before the second
  argument was staged, so the callee received different values and restoring
  the swaps corrupted the caller's loop variable. The required invariant is
  that every logical argument value is captured before argument placement can
  mutate any source register. Status: resolved in this integration slice. The
  emitter now copies each non-primary repeated occurrence into its final call
  slot before swaps begin, then applies per-argument flags to the staged slot.
  `repro_duplicate_call_argument` permanently covers repeated integer and
  string arguments plus preservation of the caller's source value in optimized
  and no-opt builds.
- **Mixed-dispatch fallback detachment (found 2026-07-10).** The initial mixed
  rewrite detached case bodies before looking up the final case's positional
  fallback child. Detachment changed the child index and could discard every
  residual comparison after a table miss. Status: resolved before the mixed
  slice baseline. The fallback is captured before any AST mutation, and
  `select_dispatch_mixed` covers dynamic miss paths in optimized and no-opt
  builds.
- **Unknown eager-operator capture type after inlining (found 2026-07-10).** An
  optimized eager comparison whose right operand was inlined could capture a
  symbol-backed left operand into an `__inline_lhs` temporary using the AST
  node's stale `.unknown` type instead of the resolved symbol type. `rxc` then
  emitted the nonexistent generic `eq` mnemonic and `rxas` rejected the output.
  Status: resolved in the remap temporary-symbol builder, which now uses the
  resolved symbol shape when the node shape is unknown. The optimized
  `select_dispatch_mixed` compiler and runtime tests permanently cover this
  path.
