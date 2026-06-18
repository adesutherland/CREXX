# Level C Rexx Runtime Values

Status: approved initial implementation slice for Level C/RexxScript runtime
foundation.

This note records the first shared runtime layer for Classic Rexx-compatible
execution. It is intentionally smaller than full Level C lowering. The goal is
to prove the value representation and Level B low-level register access before
the compiler depends on it for broad tree surgery.

## Scope

The approved first slice covers:

1. A design surface for scalar values, stems, and variable pools.
2. Level B class attribute register-view support for the VM value slots needed
   by scalar Rexx values.
3. An initial `RexxValue` class that uses one physical VM value slot with lazy
   cached representations and operator helper methods.

Stems, variable pools, BIF migration, and Level C lowering are the follow-on
slices once the one-slot scalar value proof is stable.

## Core Objects

### `RexxValue`

`RexxValue` represents one Classic Rexx scalar value. Classic Rexx is
string-first, but the VM value already has string, binary, int, float, and
decimal storage. `RexxValue` therefore maps multiple Level B attributes onto
one physical VM attribute slot:

```rexx
  _string  = .string  with register.0.string
  _binary  = .binary  with register.0.binary
  _int     = .int     with register.0.int
  _float   = .float   with register.0.float
  _decimal = .decimal with register.0.decimal
  _flags   = .int     with register.0.flags.library
```

The design rule is strict: these are views of one VM `value`, not separate
fields. Any implementation that introduces independent `_string`, `_int`, or
`_decimal` storage has missed the point and should be rejected.

For Level B class declarations, `register.0` is a Rexx-level convention for the
containing value itself. It is not RXAS attribute zero. The compiler lowers
reads and writes of a `register.0.<view>` attribute to a direct link to the
receiver/factory value, so a scalar `RexxValue` object does not need any child
attribute slots. `register.1` and above remain one-based child attribute slots.

Attributes mapped to `register.0`, and attributes sharing the same physical
`register.N` slot with another typed view, are complex attributes. Compiler
emission must treat a complex typed-view read as a local payload boundary: link
the physical storage into a scratch register, copy the requested typed view into
an ordinary local register, then unlink the scratch before expression lowering,
type promotion, or operator code can manipulate the value. Writes copy the
requested typed view back through the linked physical slot. The compiler does
not use `acopy` as hidden cache-flag maintenance for these views.
If a future compiler feature needs an explicit status-flag copy then `acopy`
is the correct RXAS primitive, but RexxValue typed payload access is not such a
case.
Binary views use the binary-only `bcopy` instruction rather than generic
`copy`, so `.binary with register.N.binary` access moves byte payload and byte
cursor state without moving public/compiler/library status flags.

The VM fields inside one value are independent storage. Cache coherency belongs
to `RexxValue`: a method that materializes or invalidates a string, binary,
integer, float, or decimal representation must update the library/user flags
it owns through the flag-view attribute. The compiler lowers a flag expression
such as `_flags = _flags + RV_FLAG_INT` as a direct masked status read, local
integer expression, and masked status write; it must not copy the object payload
or use `acopy` to maintain these flags. The compiler only protects
VM/compiler-reserved flag partitions by validating flag-view writes and lowering
writable source views to masked status replacement.

The generic compiler rule is intentionally conservative. Direct `register.N`
views are a system-programmer construct for runtime/library implementation, not
general application syntax and not a Level G feature. Runtime classes such as
`RexxValue` may still hand-optimize reviewed hot methods with inline assembler
when that avoids redundant copies, but those optimizations must preserve the
same link/copy/unlink safety boundary for naive source-level use.

RXAS has a queue-based keyhole optimiser in `assembler/rxas_opt.c`. It is the
right hook for cleanup of redundant special-attribute access, for example
repeated link/copy/unlink sequences over the same physical slot where the hazard
rules prove no intervening instruction can observe or mutate the slot. The
compiler must still emit locally correct access sequences; RXAS optimisation is
an implementation improvement, not part of the semantic contract.

The value uses the stable library/runtime status-flag band to record which
representations are known current:

- string current
- int current
- float current
- decimal current
- binary current
- byte contents are known valid text
- dropped/undefined state for later variable-pool use

VM-private bits remain VM-owned. Library code may read VM-private bits through
normal flag instructions, but must not depend on writing them.

Compiler call-ABI flags and runtime value-cache flags share the same status
word. Call setup must therefore update only the compiler flag band and leave
the library/runtime band intact. The general `settp` instruction keeps its
call-ABI-friendly partition semantics, while source-level flag-view assignment
uses the masked replacement instruction `settpmask` so a writable view can
replace its band with zero without clearing unrelated flags. The `setortp`
instruction remains the explicit OR operation for code that deliberately
accumulates flags.

Two compiler features are now part of the initial Level B support needed to
make this code maintainable enough for Level C:

1. First-class named constants. Level B source may declare compile-time
   constants, for example `constant RV_FLAG_STRING = 0x00010000`. The
   initializer must fold at compile time and the symbol is immutable. Constants
   may be used in ordinary expressions and as inline assembler operands; integer
   constants used as RXAS immediates are substituted by the compiler without
   allocating a runtime register. Runtime-value constants, including large
   `.binary` payloads, are emitted through the normal RXBIN constant-pool
   mechanism and exported/imported as typed constant metadata so a dependent
   module can use the same symbol names.
2. Register flag views. Level B class attributes may expose a masked view of a
   register status word, for example
   `cache_flags = .int with register.0.flags.library`. Flag views are direct
   status-word views, not typed payload views, so reads must not materialize the
   complex register payload or emit the link/copy/unlink sequence used by
   `.string`, `.int`, `.decimal`, `.binary`, and `.object` views.

The approved flag-view partitions are:

- `.flags.vm`: VM-private band, read-only.
- `.flags.compiler`: compiler call-ABI band, read-only from Level B source.
- `.flags.library`: stable runtime/library band, read/write.
- `.flags.user`: user/experimental band, read/write.
- `.flags.public`: library and user bands only, read/write. The compiler band
  is excluded because compiler call-ABI flags are owned by generated call
  setup/dispatch code.
- `.flags.readable`: all readable non-reserved flags, read-only.

Writes through writable flag views replace only the selected masked band:
`new_flags = (old_flags & ~view_mask) | (value & view_mask)`. They must not
clear unrelated public bands and must never write the VM-private or reserved
bits. The compiler lowers these writes to `settpmask target,value,mask`, with
`mask` restricted to the writable source-level bands. `.flags.compiler` is a
read-only view even though generated call setup still owns and updates the
compiler flag band internally.

Binary and text validity are deliberately separate claims. A value may have a
current binary byte representation without yet having a current `.string`
representation. When binary bytes are accepted from a host, BIF, or RexxScript
sandbox boundary, the value layer must be able to validate that byte sequence
as text, set the text-valid flag if it succeeds, and then lazily materialize
the `.string` view. If validation fails, the binary representation remains
current but the value must not claim that a valid text/string view exists.

The exact Classic Rexx policy for binary bytes that are not valid UTF-8 is
still open. The important first-slice rule is that the cache flags can
distinguish "bytes are current" from "bytes are valid text" so Level C and
RexxScript can share the same decision point later.

### `RexxStem`

`RexxStem` will represent one Classic Rexx stem binding. It is a string-keyed
map from tail text to `RexxValue`. Tails preserve Classic compound-variable
derivation semantics; the stem name is normalized by the variable pool.

This is a separate binding from the scalar with the same root:

- `A` maps to a scalar `RexxValue`.
- `A.` maps to a `RexxStem`.
- `A.B` resolves through the `A.` stem with derived tail `B`.

### `RexxVariablePool`

`RexxVariablePool` maps normalized variable names to bindings. Each binding
points to either a scalar value or a stem:

```text
RexxBinding
  kind: VALUE | STEM
  target: RexxValue | RexxStem
  ownership: owned | alias-to-parent
  state: defined | dropped
```

`PROCEDURE EXPOSE`, RexxScript `EXPOSE`, and host-variable anchors should alias
bindings where Classic semantics require shared storage. Copy-in/copy-out is a
host-adapter policy, not the core pool model.

## Design Constraints

- Level C compiled code and RexxScript should share this layer instead of
  growing parallel value/pool implementations.
- Level C lowering must never silently turn ordinary Classic variables into
  Level B typed locals unless the variable-pool semantics are preserved.
- Operators should be available as helper methods/functions that the compiler
  can lower to directly, for example `add(result, left, right)` or an
  equivalent method shape.
- Materializers and operators should prefer already-current typed views before
  parsing through `.string`. String remains the semantic default and fallback
  for Classic Rexx values, but it must not become the internal bus for every
  numeric promotion. For example, int-to-decimal promotion should use the
  current `.int` view directly and then set the decimal-current flag.
- Materialization of a cached representation is not a source-level assignment.
  It may mutate the VM value slot to fill another representation and set the
  matching cache bit.
- `.string` remains valid UTF-8 in normal Level B builds. Classic byte-text
  compatibility is a later explicit Level C policy and must not weaken Level B
  `.string`/`.binary`.
- A binary-to-string materializer must validate bytes before setting the string
  current/text-valid flags. A string-to-binary materializer may mark both the
  binary-current and text-valid flags because Level B strings are already valid
  UTF-8.
- Level B source can access register status partitions through
  `.flags.<partition>` class attributes. Reads lower to masked flag reads.
  Writes are allowed only for source-writable partitions and lower to masked
  replacement, not generic typed register copies.

## Initial Operator Surface

The first `RexxValue` proof should include:

- setters and materializers for string, int, float, decimal, and binary
- cache flag inspection
- `copyFrom`
- `concat`
- `add`, `subtract`, `multiply`, and `divide`
- loose comparison helpers for equality/order

The initial arithmetic implementation may prefer decimal semantics where both
operands can materialize as decimals. Exact Classic numeric policy, including
`NUMERIC DIGITS`, `FORM`, `FUZZ`, `%`, `//`, and error-number mapping, belongs
to the Level C lowering/BIF migration slice.

## QA Expectations

The first proof is not accepted until:

- compiler validation accepts `.decimal` and `.binary` register views;
- source and binary import stubs preserve those explicit register views;
- a runtime test proves the same object slot can be read through string, int,
  float, decimal, and binary views;
- a runtime test proves `RexxValue` materializers and first operators work;
- the focused compiler/library tests and then the full relevant CTest suite run
  cleanly before checkpoint commits.
