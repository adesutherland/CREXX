# RexxValue Performance Worklist

Temporary worklist for RexxValue/compiler performance follow-up. Delete this
file when the items below have either been implemented or deliberately rejected.

Reviewed: 2026-06-18.

## 1. Flag Test Syntax And Codegen

Status: completed. Level B named operators now provide direct flag/bit syntax
such as `<has>`, `<set>`, `<clear>`, `<and>`, `<or>`, `<xor>`, and `<not>`.
`RexxValue` has been rewritten to use that surface.

RexxValue previously spelled flag checks as `(flags % RV_FLAG_X) // 2` because
Level B had no direct integer bit-test surface. The emitted code used integer
divide plus modulo plus compare for each test.

The VM/RXAS layer already has integer bitwise instructions: `iand`, `ior`, and
`ixor`, including immediate forms. This item is therefore primarily compiler
surface/codegen work, not VM instruction work.

Add a Level B integer bitwise/test feature and lower it directly to the existing
integer bitwise instructions or a status-mask predicate. Then rewrite RexxValue
flag tests and flag set/clear code for readability and performance.

## 2. Binary Register-View Copy

Status: completed. RXAS/VM now has `bcopy`, a binary-payload copy instruction.
The compiler maps `.binary` typed register-view access to `bcopy` through
`type_to_prefix()`, and the existing duplicate linked-read peephole rules cover
`bcopy` as a typed copy family.

RexxValue `.binary with register.0.binary` previously emitted the generic `copy`
instruction for binary view reads and writes. Unlike `scopy`, `icopy`, `fcopy`,
and `dcopy`, generic `copy` copies the whole VM value payload and status state.
That was functionally safe in the current RexxValue code because cache flags are
then managed explicitly via `.flags.library`, but it is less precise than the
other typed views and can move status attributes through temporary registers.

Investigate whether RXAS/VM should grow a binary-payload copy instruction, for
example `bcopy`, and update `type_to_prefix()` / compiler emission so `.binary`
complex register-view access uses it. The expected shape should match the other
views:

```text
link rTmp,rObject
bcopy rDest,rTmp
unlink rTmp
```

This is not about missing binary BIFs or byte operators. The language already
has binary literals, string/binary conversion instructions, and binary BIFs.
The gap is a typed binary payload copy for register views.

## 3. Numeric Operator Mode

Status: completed. `RexxValue` now has module-level numeric-mode procedures:
`rexxvalue_numeric_mode()` reads the current mode and
`rexxvalue_set_numeric_mode(mode)` selects decimal mode (`0`) or float mode
(`1`). Decimal remains the default and invalid setter values normalize back to
decimal. Arithmetic operators branch once on the mode and then use either
decimal materializers/results or float materializers/results.

This is only the execution-representation switch. Full classic Rexx numeric
semantics will also need a numeric context for settings such as `DIGITS` and
`FUZZ`; do not treat the mode switch alone as the complete Level C numeric
contract. The optimized classlib RXAS now shows the four arithmetic methods at
about 63 locals each because both decimal and float materializer paths are
present in the method body; item 4 owns that register-pressure follow-up.

RexxValue arithmetic now avoids string conversion, but the first implementation
promoted all operators through decimal. The explicit arithmetic mode lets
runtime users choose decimal or float numeric execution. Avoid a three-way
int/decimal/string decision tree in ordinary operators; use int fast paths only
where they are clearly worth the added complexity.

Expected first shape:

- decimal mode: current decimal promotion/fallback
- float mode: float promotion/fallback
- optional later int/int fast path if measured worthwhile

## 3a. Classic Rexx Numeric Settings

Status: pending. Add the classic Rexx numeric context needed by Level C,
starting with `DIGITS` and `FUZZ` and later any related settings such as
numeric form if required. This should integrate with the `RexxValue` arithmetic
mode rather than adding separate ad hoc arithmetic paths.

## 4. Inlining And Register Pressure

The core materializers are acceptable, but expression-shaped RexxValue methods
show large register counts after inlining. Current optimized classlib RXAS
observed after the numeric-mode change:

- `asString`: around 10 locals
- `asInt`, `asFloat`, `asDecimal`: around 12 locals
- `asBinary`: around 15 locals
- `add`/`subtract`/`multiply`/`divide`: around 63 locals
- `equals`: around 44 locals
- `concat`: around 63 locals
- `copyFrom`: around 75 locals

This appears to be part of the broader compiler register-pressure problem.
Investigate inliner budget controls, no-inline annotations, or
register-pressure-aware inlining before adding more RexxValue operator surface.

## 5. Register Assignment

Add register assignment as a dedicated compiler TODO/work item. The current
register numbering can become large in inlined expression code and makes RXAS
harder to inspect. Better assignment should also make keyhole optimisation more
effective.

## 6. Link/Unlink Keyhole Follow-Up

The duplicate linked-read optimizer is present, but RexxValue still has many
`link`/typed-copy/`unlink` sequences after assembly. Review real emitted shapes
and add narrowly safe peephole rules where repeated reads or copy chains remain
after inlining.

## 7. Minor RexxValue Cleanup

After the above, sweep RexxValue comments and constants. Current known minor
items:

- `RV_FLAGS_BINARY_TEXT` is defined but not currently used.
- `RV_FLAGS_ALL_CACHE` is defined but not currently used.
- Arithmetic flag idioms should be replaced or commented once the compiler
  feature decision is made.
