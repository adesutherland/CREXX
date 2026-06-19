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
structure that makes these lifetimes visible. Scoped allocation now returns
eligible named locals when real local scopes complete, including expression
`BLOCK_EXPR` scopes. The tree surgery therefore preserves semantics and avoids
call/link hazards while allowing the allocator to reuse the cloned local
register window.

For real source-level named locals, fixed register numbers remain the expected
model within their live scope for readability, tracing, and simple metadata. The
useful refinement is fixed registers within a scope, with registers returned
after a provably local scope is complete. The first safe target is real named
locals declared in nested statement scopes such as `DO`, `SIGNAL_BLOCK`,
explicit local `INSTRUCTIONS` scopes, and real expression `BLOCK_EXPR` scopes.
Synthetic blocks that deliberately inherit their parent scope/register behavior
are not reuse boundaries.

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
  symbols remain procedure-lifetime registers. Real `BLOCK_EXPR` scopes are now
  included in scoped reuse; inherited-register synthetic blocks are still not
  reuse boundaries.
- Keep the reuse/cleanup distinction explicit:

  | Pattern | Scoped register reuse | Lifetime / metadata cleanup |
  | --- | --- | --- |
  | procedure-level source locals | fixed for procedure | procedure cleanup |
  | eligible nested `SCOPE_LOCAL` locals | reusable after owning scope | `endlife` plus scoped metadata closeout |
  | `BLOCK_EXPR` locals | reusable after owning expression scope | `endlife` plus scoped metadata closeout |
  | inherited-register scopes | not a reuse boundary | parent scope owns cleanup |
  | exposed variables | fixed global/exposed slot | not local-owned |
  | arguments and `.ref` arguments | fixed ABI/alias slot | call/frame semantics own lifetime |
  | receiver / factory pseudo-locals | fixed ABI/object identity | call/frame semantics own lifetime |
  | reference-targeted locals | not reused while alias-visible | `endlife` remains required at scope exit |
  | generated `__inline*` helpers | tactical non-reuse until helper roles are explicit | inline scaffolding is not treated as source-owned storage |
  | generated `__rxtrace*` helpers | non-reuse; trace-only generated path | normal block cleanup may still close generated trace locals |

- Treat inline-helper classification as a priority item for the `BLOCK_EXPR`
  register-pressure pass. The inliner already knows whether a helper is a
  call-frame/alias capture, order-sensitive materialisation temp, or pure
  evaluation-order scratch when it creates the AST. Preserve that role on the
  generated symbol instead of teaching the allocator to infer semantics from
  `__inline*` names. Until that metadata exists, the blanket `__inline*`
  non-reuse rule remains the correct tactical fence.
- `BLOCK_EXPR` deep-dive, 2026-06-19:

  The real optimisation target is named locals in real expression `BLOCK_EXPR`
  scopes created by the inliner. It is not compiler-generated statement blocks
  that inherit the parent register scope, and it is not generated `__inline*`
  helper symbols until those helpers carry explicit lifetime roles.

  Existing safety machinery is stronger than the original blanket fence assumed:
  `LEAVE_WITH` copies the yielded value into the parent `BLOCK_EXPR` result
  register before branching to the common block-exit label; deferred cleanup for
  linked child helpers must therefore stay pending through `LEAVE_WITH` register
  allocation so the `BLOCK_EXPR` result cannot reuse a register that will still
  be unlinked by the emitted cleanup; the allocator assigns the `BLOCK_EXPR`
  result register before releasing the block's scoped named locals, so the
  result cannot reuse a local that is about to die; and block emission already
  performs dereference unlinks, `endlife`, and metadata closeout at that common
  exit. This means eligible cloned callee locals can plausibly be returned at
  block exit without changing value-return semantics.

  A controlled local experiment removed the `BLOCK_EXPR` exclusion from
  `scope_recycles_named_registers()` and then tightened deferred-register
  handling around linked helpers. Focused runtime tests for array/object
  `BLOCK_EXPR` returns, call-argument expression blocks, imported BIF expression
  lifetimes, array-attribute call-result lifetimes, the live-sibling regression,
  and `testRexxValue` all passed. Opt RXAS goldens changed only where expected
  because `.locals` shrank and registers were repacked. Measured examples from
  that experiment:

  | Example | Current opt locals | Experimental opt locals |
  | --- | ---: | ---: |
  | `inline_test_array_return_expr.main` | 14 | 12 |
  | `inline_test_array_multi_return_expr.main` | 20 | 13 |
  | `inline_test_object_return_expr.main` | 10 | 9 |
  | `inline_test_array_expr_arg.main` | 6 | 5 |
  | `inline_test_object_expr_arg.main` | 7 | 6 |
  | `testRexxValue.main` | 215 | 211 |
  | `RexxValue.add` / `subtract` / `multiply` / `divide` | 48 | 38 |
  | `RexxValue.copyFrom` | 43 | 39 |
  | `RexxValue.concat` | 30 | 28 |
  | `RexxValue.equals` | 26 | 24 |

  Hazards verified during the implementation:

  - generated RXAS for the affected opt goldens was inspected to confirm every
    repacked block still has the expected copy-to-result, `endlife`, and metadata
    closeout order;
  - keep generated `__inline*` helpers fixed until helper lifetime roles are
    explicit, because helper values such as call-frame captures and alias-visible
    references are not ordinary cloned source locals;
  - keep arguments, `.ref` arguments, exposed variables, receiver/factory
    pseudo-locals, and reference-targeted storage fixed for the same reasons as
    ordinary scoped reuse;
  - rerun the broader inline/reference/runtime slice, then the compiler suite,
    then full CTest before committing.

  Implementation step: removed the `BLOCK_EXPR` owner exclusion in
  `scope_recycles_named_registers()`, kept `LEAVE_WITH` child cleanup registers
  deferred until the owning expression has a result register, hardened duplicate
  deferred/free-list handling, updated the affected opt goldens after inspection,
  and left helper classification as the next standalone change.
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
