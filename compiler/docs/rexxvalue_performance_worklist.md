# RexxValue Performance Worklist

Temporary worklist for RexxValue/compiler performance follow-up. Delete this
file when the items below have either been implemented or deliberately rejected.

Reviewed: 2026-06-18.

## 1. Binary Register-View Copy

RexxValue `.binary with register.0.binary` currently emits the generic `copy`
instruction for binary view reads and writes. Unlike `scopy`, `icopy`, `fcopy`,
and `dcopy`, generic `copy` copies the whole VM value payload and status state.
That is functionally safe in the current RexxValue code because cache flags are
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

## 2. Flag Test Syntax And Codegen

RexxValue currently spells flag checks as `(flags % RV_FLAG_X) // 2` because
Level B has no direct integer bit-test surface. The emitted code uses integer
divide plus modulo plus compare for each test.

Add a Level B integer bitwise/test feature and lower it directly to cheap
integer operations or a status-mask predicate. Then rewrite RexxValue flag tests
and flag set/clear code for readability and performance.

## 3. Numeric Operator Mode

RexxValue arithmetic now avoids string conversion, but all operators currently
promote through decimal. Add an explicit arithmetic mode, probably a process or
runtime global initially, to choose decimal or float numeric execution. Avoid a
three-way int/decimal/string decision tree in ordinary operators; use int fast
paths only where they are clearly worth the added complexity.

Expected first shape:

- decimal mode: current decimal promotion/fallback
- float mode: float promotion/fallback
- optional later int/int fast path if measured worthwhile

## 4. Inlining And Register Pressure

The core materializers are acceptable, but expression-shaped RexxValue methods
show large register counts after inlining. Recent review observed:

- `asString`: around 10 locals
- `asInt`, `asFloat`, `asDecimal`: around 12 locals
- `asBinary`: around 15 locals
- `add`/`subtract`/`multiply`/`divide`: around 35 locals
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
