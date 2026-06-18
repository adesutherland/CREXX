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
emission must treat a complex attribute read as a copy boundary: link the
physical storage into a scratch register, copy the requested typed view into an
ordinary local register, then unlink the scratch before expression lowering,
type promotion, or operator code can manipulate the value. Writes and
materializers copy the updated representation back through the linked physical
slot. This prevents compiler-generated arithmetic, casts, and helper calls from
accidentally corrupting another view of the same underlying VM value.

RXAS already has a queue-based keyhole optimiser in `assembler/rxas_opt.c`.
That is the right hook for later cleanup of redundant special-attribute access,
for example repeated link/copy/unlink sequences over the same physical slot
where the hazard rules prove no intervening instruction can observe or mutate
the slot. The compiler must still emit the conservative copy boundary; RXAS
optimisation is an implementation improvement, not part of the semantic
contract.

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
- Flag access starts through RXAS flag instructions from Level B methods. A
  later `.flags` register-view suffix would require dedicated emitter
  semantics and should be treated as syntax sugar, not part of this proof.

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
