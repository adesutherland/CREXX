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

Status: completed for decimal `DIGITS` and `FUZZ`. `RexxValue` now exposes
`rexxvalue_numeric_digits()`, `rexxvalue_set_numeric_digits(digits)`,
`rexxvalue_numeric_fuzz()`, and `rexxvalue_set_numeric_fuzz(fuzz)`.
Digits defaults to 18, fuzz defaults to 0, and invalid updates leave the
current setting unchanged and return `-1`. Decimal materialization and decimal
arithmetic apply these settings with the VM `setnumdgts` and `setnumfuz`
instructions in the active method frame. Float mode deliberately ignores these
settings for now.

Further numeric context work remains for Level C, especially numeric form and
exact error-number mapping. Any later settings should integrate with this
RexxValue numeric context rather than adding separate ad hoc arithmetic paths.

## 4. Inlining And Register Pressure

The core materializers are acceptable, but expression-shaped RexxValue methods
show large register counts after inlining. Current optimized classlib RXAS
observed after the decimal digits/fuzz change:

- `asString`: around 12 locals
- `asInt`, `asFloat`: around 13 locals
- `asDecimal`: around 14 locals
- `asBinary`: around 17 locals
- `add`/`subtract`/`multiply`/`divide`: around 77 locals
- `equals`: around 50 locals
- `concat`: around 71 locals
- `copyFrom`: around 83 locals

Baseline inspection, 2026-06-18:

| Example | No-opt locals | Opt locals | Observation |
| --- | ---: | ---: | --- |
| `RexxValue.add` | 11 | 77 | `asFloat`, `asDecimal`, and factory bodies inline into both operands/branches. |
| `RexxValue.concat` | 8 | 71 | `asString` materializers dominate the inlined body. |
| `RexxValue.copyFrom` | 10 | 83 | Multiple materializers and setter paths inline together. |
| `RexxValue.equals` | 6 | 50 | Inlined `asString` calls dominate. |
| `testRexxValue.main` | 27 | 235 | Imported classlib inlining greatly reduces calls but raises local high-water. |
| `inline_test_composed_expr_contexts.main` | 7 | 42 | Golden compiler example showing the same call-removal/local-growth tradeoff. |

The current allocator does recycle expression temporaries through `get_reg()` and
`ret_reg()`, so register recycling is not simply absent. The immediate pressure
comes from aggressive inlining plus fixed registers for named locals from each
inlined body. AST-level inlining already creates `BLOCK_EXPR` / `SCOPE_LOCAL`
structure that should make these lifetimes visible, but the register pass does
not currently use it: `SCOPE_LOCAL` shares the parent register scope, and
`assign_registers_in_scope()` pre-assigns nested local-scope symbols at procedure
entry. The tree surgery therefore preserves semantics and avoids call/link
hazards, but does not yet deliver scoped register reuse.

For real source-level named locals, fixed register numbers remain the expected
model within their live scope for readability, tracing, and simple metadata. The
useful refinement is fixed registers within a scope, with registers returned
after a provably local scope is complete. The first safe target is real named
locals declared in nested statement scopes such as `DO`, `SIGNAL_BLOCK`, and
explicit local `INSTRUCTIONS` scopes. Expression `BLOCK_EXPR` scopes are not
recycled yet: an inlined `BLOCK_EXPR` can sit inside an attribute or array
assignment where linked target helper registers remain live across the
expression. Until the allocator models those live linked helpers, `BLOCK_EXPR`
locals must keep stable non-recycled registers. Synthetic blocks that
deliberately inherit their parent scope/register behavior are not reuse
boundaries.

The hand-tuned ideal for `RexxValue.add` would keep the real method locals
fixed, then reuse a compact scratch window across operand materialization,
branch-local work, and factory result creation. Current optimized output instead
clones named locals such as `temp`, `flags`, `text`, `numeric_digits`, and
`numeric_fuzz` for each inlined materializer instance, plus synthetic inline
leave registers. That makes the optimized shape correct but much larger than a
hand-tuned implementation.

Preferred next work:

- Exploit the existing AST surgery first: allocate named locals in eligible
  real statement `SCOPE_LOCAL` scopes from the reusable register pool and return
  them when that scope is complete.
- Keep procedure-level source locals fixed and permanent for now.
- Add inliner policy as a fallback/secondary control: a no-inline annotation or
  caller-side budget based on estimated local pressure, not only the existing
  per-callee node cutoff.
- Improve scratch/call-frame reuse and synthetic inline temporary handling.
- Preserve metadata correctness while reusing registers: scoped metadata must
  open when the block-local symbol becomes live and close before its register is
  returned/reused.
- Use the baseline examples above to judge improvements before changing
  RexxValue operators again.

## 5. Register Assignment

Add register assignment as a dedicated compiler TODO/work item, but split it
from the immediate inliner-pressure fix.

Design constraint: real named locals can keep fixed register numbers within the
scope where they are live. Procedure-level source locals remain fixed for the
whole procedure. Block-local and generated inline locals can be recycled after
scope exit because the AST already carries their scope boundaries.

Current findings:

- Anonymous expression temporaries are recycled today.
- Recycled VM stack frames do not zero locals in the normal build because
  `SAFE_RECYCLED_STACKFRAMES` is disabled. Scoped named-local reuse therefore
  relies on the compiler's symbol initiator path emitting `null rN` before a
  newly live local first uses a recycled register.
- Call argument frames require contiguous `rN` ranges and can raise the
  procedure `.locals` high-water even when returned immediately.
- Complex attribute and array access reserve extra helper registers; these are
  returned, but still contribute to high-water when the surrounding method has
  already accumulated many fixed locals.
- Inlined callee locals are ordinary named locals after cloning, and every
  repeated inline of `asString`, `asFloat`, or `asDecimal` currently adds
  another procedure-lifetime fixed local set even though the cloned inline scope
  is local.

Register-assignment improvements still wanted:

- Track and report register allocation categories in a compiler diagnostic
  mode: source locals, inlined locals, synthetic inline temporaries, expression
  temporaries, call frames, and complex attribute helpers.
- Implement scoped allocation/release for eligible `SCOPE_LOCAL` symbols.
  Storage-bearing block locals are now recyclable for known scalar values,
  `.binary`, object values, reference values, arrays, and `TP_UNKNOWN` compiler
  symbols. Eligibility is still conservative at the ownership boundary:
  exposed symbols, arguments, receivers/factories, variables whose storage is a
  reference target, generated `__inline*`, and trace-helper `__rxtrace*`
  symbols remain procedure-lifetime registers. `BLOCK_EXPR` scopes also remain
  non-recycled because inlined expression scopes can run while linked attribute
  or array target helpers are still live.
- Rework scoped metadata emission with the allocation change. Current variable
  metadata is symbol-keyed via `meta_emitted` and normally cleared at procedure
  end. If registers are reused between block locals, the emitter must clear a
  block-local symbol's metadata at block exit before returning its register,
  then emit metadata for the next symbol that reuses that register. Otherwise
  RXAS/debug consumers can see one register as two live names.
- Consider lifetime packing for synthetic inline temporaries.
- Keep inliner policy work as a complementary guard if scoped allocation does
  not sufficiently reduce the hot cases.
- Keep item 6 separate: keyhole link/copy cleanup can reduce emitted work, but
  it cannot by itself lower `.locals` unless a later RXAS renumbering pass is
  added.

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
