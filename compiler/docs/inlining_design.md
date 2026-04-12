# cREXX Inlining Design

## Overview

Inlining in `rxc` is an AST rewrite performed during optimisation. The immediate goal is a buildable, staged implementation where the inliner only selects call sites whose rewrite strategy is already implemented.

That means selection is opportunity-based, not just callee-based. A procedure may be generally eligible for inlining, but only some uses of it may be rewritten in a given phase.

The selector is now best understood in terms of capability buckets rather than one-off scenarios:

- statement rewrites where the enclosing statement can be replaced directly
- eager value consumers such as `SAY` and `RETURN`
- eager call arguments to call-like parents
- eager expression parents such as arithmetic, concatenation, and comparison operators

Contexts outside those buckets stay uninlined until their rewrite strategy exists.

## Current Strategy

### Supported scenarios

The currently supported scenarios are:

```rexx
x = func(a)
```

and

```rexx
call func(a)
```

and

```rexx
say keepValue(func(a))
```

and

```rexx
say func(a)
```

and

```rexx
return func(a)
```

and

```rexx
if func(a) = 3 then ...
```

and

```rexx
x = 10 + func(a)
```

More precisely:

- The call must be a plain procedure `FUNCTION`, not a method or factory call.
- For assignment inlining, the `FUNCTION` call must be the entire RHS of the enclosing `ASSIGN`.
- For standalone call inlining, the enclosing statement must be `CALL func(...)`.
- For expression inlining, the `FUNCTION` call must sit in an expression parent that can consume a `BLOCK_EXPR` result directly. In practice this is now the default for local plain procedures; dedicated statement rewrites still own standalone `CALL` statements and whole-RHS `ASSIGN` sites.
- The callee must be a normal procedure, not a method or factory.
- Arguments and return values must stay within the currently implemented safe slice: scalars, object values, binary values, optional/default formals, and array-shaped formals/returns are now supported across the local plain-procedure slice.
- The callee must satisfy the existing safety checks: plain local procedure only, small body, no recursion cycle, and no unsupported vararg indexing. Value-producing procedures still require a final `RETURN`; void statement-call sites may inline through bare-return and fallthrough shapes.
- `expose`/by-reference formals are supported when the actual argument is an aliasable variable-like target, including indexed and stem-style forms.
- For nontrivial by-reference actuals, the inline rewrite captures the locator expressions once into inline-scope temps so the callee still sees call-time binding semantics.
- Optional formals now inline through the same rewrite path as other supported local plain-procedure calls, with omitted-actual/default-formal semantics preserved during binding.

At present, expression rewriting is open for local plain procedures in all value and condition parents that can consume a `BLOCK_EXPR` directly. The discriminator now explicitly keeps only the dedicated statement-rewrite sites out of the expression path:

- whole-RHS `ASSIGN` sites, which are rewritten as assignment statements rather than expression children
- standalone `CALL func(...)` statements, which use the statement-position rewrite

Short-circuit boolean parents (`|`, `&`, `\`), direct condition consumers (`IF`, `WHILE`, `UNTIL`), loop-bound expressions (`FOR`, `TO`, `BY`), call arguments, receiver-position `MEMBER_CALL` expressions, and ordinary value consumers/operators are all admitted.

Composed inline sites are now supported when an outer call consumes an already-inlined `BLOCK_EXPR` actual, provided the outer context can itself consume a `BLOCK_EXPR`. This relies on scope-aware subtree cloning so nested `BLOCK_EXPR` locals and `LEAVE_WITH` associations remain isolated after the outer rewrite.

The statement-position cases rewrite the enclosing statement. The expression-position case rewrites the `FUNCTION` node itself to `BLOCK_EXPR`.

### Why selection must stay narrow
The inliner must not mark a procedure as globally "inline now" and then rewrite every call. That breaks the incremental rollout plan. Instead:

1. Identify procedures that are structurally inlineable.
2. Classify each call site by context capability bucket.
3. Rewrite only call sites whose bucket is already supported.

This preserves the ability to build and test the compiler while inlining support is still partial.

## Compiler-Added Statement Blocks vs `BLOCK_EXPR`

Both forms are still needed, but they no longer need separate node types.

### Compiler-added statement block

Use a compiler-added `INSTRUCTIONS` node for statement-position rewrites that need:

- scope isolation
- a sequence of injected statements
- no expression value of their own

This is the right tool for the current statement-position phases, because `x = func(a)` should become a statement block that contains:

1. argument binding
2. cloned callee body
3. `RETURN expr` rewritten to `x = expr`

The original assignment statement is replaced as a whole by the block.

### `BLOCK_EXPR`

Use this only when the inline expansion itself must remain an expression and yield a value to its parent expression tree.

This is now used for the first expression-position slice, and remains the path for broader later phases such as:

```rexx
x = func(a) + y
```

or other contexts where hoisting to surrounding statements would change semantics.

`BLOCK_EXPR` already has the compiler machinery for:

- scoped expression blocks
- `LEAVE_WITH` result propagation
- emitter support for yielding a register result

So for expression-valued inlining, the target form should be a `BLOCK_EXPR`, not a raw statement block.

### Decision

Do not collapse statement rewrites into `BLOCK_EXPR`.

Use:

- compiler-added `INSTRUCTIONS` for statement-only inlining rewrites
- `BLOCK_EXPR` for expression-valued inlining rewrites

Also avoid wrapping a `BLOCK_EXPR` around a compiler-added statement block unless a specific implementation need appears. For expression contexts, prefer constructing the final `BLOCK_EXPR` shape directly.

## AST Rewrite Plan

### Phase 1 rewrite: `x = func(a)`

Given:

```rexx
x = func(a)
```

rewrite the enclosing assignment into:

1. a compiler-added `INSTRUCTIONS` block
2. containing argument binding statements
3. containing the cloned callee statements
4. with the callee `RETURN expr` rewritten to `x = expr`

Important details:

- Formal parameter binding must target the formal variable node, not the `ARG` node.
- Actual arguments must be read from the real call shape used by `FUNCTION` nodes.
- The cloned body must use isolated local symbols and scopes.
- The result should be a valid statement tree without forcing a statement-only block into expression position.

### Phase 2 rewrite: `call func(a)`

Given:

```rexx
call func(a)
```

rewrite the enclosing call statement into:

1. a compiler-added `INSTRUCTIONS` block
2. containing argument binding statements
3. containing the cloned callee statements
4. with the callee `RETURN expr` rewritten so the expression is still evaluated even though the caller ignores the result

Important details:

- A bare `RETURN` with no expression can simply disappear in the inlined form.
- A `RETURN expr` must not be dropped, because the expression may still have side effects.
- The current implementation handles this by sinking the ignored return value inside the compiler-added block.

### Phase 3 rewrite: direct operand expression inlining

Given:

```rexx
x = 10 + func(a)
```

rewrite the `FUNCTION` node into:

1. a `BLOCK_EXPR`
2. containing argument binding statements
3. containing the cloned callee statements
4. with the callee `RETURN expr` rewritten to `LEAVE_WITH expr`

Important details:

- The `BLOCK_EXPR` must preserve the original call node's value and target typing.
- The current implementation now admits local plain-procedure calls in ordinary value consumers/operators, short-circuit boolean operators, direct condition consumers, loop-bound expressions, call arguments, and receiver-position member expressions.
- Whole-RHS `ASSIGN` and standalone `CALL` sites still use dedicated statement-position rewrites instead of the expression path.

## Procedure Eligibility

For the current local-plain-procedure slice, a procedure is structurally eligible only if all of the following hold:

- standard procedure only
- not `main`
- not a method
- not a factory
- local body available in the current compilation unit
- argument and return shapes stay within the implemented local-procedure slice, including aggregates, binary values, by-reference formals, by-value varargs, and the current constant-index `.ref` vararg support
- value-producing procedures still satisfy the current return-shape proof used by the inliner
- body size is below the configured node threshold
- no recursion cycle is introduced by the inline ancestry chain
- no unsupported dynamic `.ref` vararg access is required

Call-site selection remains separate from structural eligibility: an eligible procedure may still have uninlined call sites if the parent rewrite bucket belongs to milestone 2 or 3.

## Milestones

The milestone names below describe user-visible capability goals. They are intentionally broader than a single implementation slice, so each milestone is expected to contain several staged iterations.

### Milestone 1: all local procedures work

This milestone was delivered in two internal stages:

- `M1a`: local plain procedures within the current safe scalar slice, including fixed arguments, optional arguments, by-reference formals, and by-value varargs, across the currently supported call-site capability buckets
- `M1b`: local plain procedures beyond the current leaf restrictions, removing the temporary exclusions around nested calls, nested scopes, and single-trailing-`RETURN` only shapes

Notes:

- The current implementation now supports by-value varargs, nested-call local procedures, and nested callee scopes through a bounded fixed-point pass with explicit cycle blocking for self-recursive and mutually recursive expansions.
- The current implementation still excludes methods/factories and class-scope procedures. Object by-value formals now follow the same split as normal calls: read-only formals alias the incoming binding, while writable formals materialise an isolated local copy. Array-shaped formals and returns are now handled by the same inline rewrite machinery as other supported local plain-procedure calls.
- Selection should remain opportunity-based throughout milestone 1: a structurally inlineable procedure may still have uninlined call sites if their rewrite bucket is not yet implemented.

### Milestone 1 review

Milestone 1 is now complete for local plain procedures.

That does not mean every syntactic use of a local procedure is now inlined. It means the remaining non-inlined cases are no longer caused by missing local-procedure body semantics such as varargs, nested calls, nested scopes, multi-return shapes, or aggregate value binding. Those local-procedure capability gaps are now closed.

The remaining discriminator behaviour is intentional and milestone-driven:

- methods and factories are still out because they belong to milestone 2
- imported callees are still out because they belong to milestone 3

So the milestone 1 closure is: local plain procedures are structurally and semantically covered; the remaining exclusions are later-milestone boundaries, not plain-procedure semantic gaps.

Decision taken at milestone-1 closure:

- keep the implementation on top of existing RXAS primitives rather than adding new VM or assembler instructions just for inlining
- use compiler-side capture and rewrite helpers to preserve evaluation-once and alias semantics
- support `.ref` / `expose` varargs for the common inlineable cases: `arg[]`, constant `arg[n]`, and constant `arg(n, "E")`
- keep forwarded constant `.ref` vararg elements semantically correct by allowing them to flow into inner `expose` calls while leaving those inner calls as normal calls instead of inventing a locator-table model mid-milestone
- defer dynamic `.ref` vararg indexing (`arg[ix]`, `arg(ix, "E")`) to a later design iteration; this is now treated as a post-milestone enhancement, not a milestone-1 blocker

### Milestone 2: all local class method inlining works

This milestone should start with local methods before it attempts factories.

Required work includes:

- selecting `MEMBER_CALL` callees, not just `FUNCTION` callees
- binding `§this` correctly
- preserving receiver evaluation-once semantics
- preserving class-scope and attribute resolution rules inside the cloned body

Factories may follow as part of milestone 2, but they should be tracked separately from ordinary methods because their call shape and object-construction semantics are different enough to justify an incremental rollout.

### Milestone 3: imported procedures and methods

Milestone 3 requires an explicit design phase before implementation.

The key open question is how inlineable callee bodies are made available across module boundaries. Current import handling is sufficient for signature-driven resolution, but not yet for general cross-module body cloning.

The main design options currently in scope are:

- metadata carries a reusable AST/symbol representation for inlineable imported bodies
- imported REXX source is used as the body source for inlining

Those options should be evaluated during the milestone 3 design phase. Additional approaches may emerge, but milestone 3 should not be treated as just another selector extension until body transport and ownership are understood.

Also note that imported sources are not all equivalent:

- source imports may plausibly provide full bodies
- metadata-only imports such as `.rxbin` and plugin-driven imports may only provide signatures or stubs

So milestone 3 should likely begin with the narrower target of source-imported procedures and methods whose bodies are definitely available.

## Current Status

The implementation now covers:

- local plain procedures in statement and expression buckets across the current scalar slice
- optional/default formals with omitted-actual binding preserved in the inline body
- a production inline body cutoff of 200 AST nodes for local plain procedures
- by-value trailing varargs
- dynamic vararg indexing and existence checks via inline-scope captured vararg arrays
- nested-call local procedures via repeated identify-and-inline passes until a bounded fixed point is reached
- nested callee-local scopes with duplicated scope and symbol remapping
- multi/early-return procedures, including branch returns and void fallthrough in statement-call sites
- object-typed returns and by-value object formals
- array-typed formals and array-valued returns, including assignment, expression, and temp-materialised return sites
- non-symbol aggregate actuals and non-symbol aggregate return expressions via inline temp materialisation
- broader aliasable by-reference actuals, including computed indexed/stem locator children
- `.ref` / `expose` varargs for `arg[]`, constant `arg[n]`, and constant `arg(n, "E")`, using existing RXAS argument/link primitives plus compiler-side capture helpers
- assignment-site inlining when the LHS itself has child selectors, by falling back to the RHS `BLOCK_EXPR` path
- binary-typed local plain procedures across the current statement and expression rewrite machinery
- preserved default-init requirements for duplicated inline locals and inline-created aggregate temporaries
- explicit cycle blocking so self recursion and mutual recursion do not expand indefinitely

The implementation still excludes:

- methods and factories
- imported callees
- dynamic-index `.ref` / `expose` vararg access

## Post-Hardening Status

The temporary hardening phase for local plain procedures is complete.

The work that was previously fail-closed during broad-cutoff exploration is now part of the normal supported slice:

- scope/symbol duplication preserves default-init requirements for cloned locals and inline-created aggregate temporaries
- optional/default formals inline with omitted-actual/default-formal semantics preserved
- array-typed formals and array-valued returns inline in assignment, expression, and temp-materialised contexts
- whole-variable aggregate assignment emission preserves copy semantics for array and binary values in inline-expanded paths

The current production stance is:

- keep the 200-node body cutoff for local plain procedures
- treat methods/factories, imported callees, and dynamic-index `.ref` / `expose` vararg access as milestone-boundary exclusions, not temporary hardening gaps
- keep debug-visible inline rejection and rewrite diagnostics available for future tuning and milestone work

Validation for the current cutoff and supported slice:

- debug top-level `ctest` matrix passed
- release compiler suite passed
- debug and release `performance` label runs passed
- additional compiler gold drift exposed by the broader cutoff was refreshed only after the matching runtime tests stayed green

Future tuning in this area should prefer better profitability modeling over reintroducing structural fail-closed exclusions for already-supported local plain procedures. In particular, any later experimentation with larger limits should bias toward call-site-sensitive policy, such as hot/cold or loop-sensitive thresholds, rather than undoing the milestone-1 semantic coverage.

## Discriminator Review

The inline discriminator is now best understood as a two-stage filter.

Stage 1: structural procedure eligibility

- `identify_inlinable_walker()` answers whether a procedure body is inline-capable in principle
- it rejects non-plain procedures (`main`, class-scope procedures, methods, factories), unsupported vararg shapes, invalid value-return shapes, oversized bodies, and malformed vararg access
- it does not decide that every call must inline; it only marks the callee as structurally available

Stage 2: call-site capability selection

- `inline_validate_call_site()` answers whether a particular call is safe for the current rewrite machinery
- it applies recursion blocking, arity/varg checks, and the concrete binding rules for actual arguments
- statement rewrites then decide whether the enclosing node shape is one of the implemented buckets
- expression rewrites additionally pass through `inline_classify_expr_context()`, which now defaults local plain-procedure expression sites to the `BLOCK_EXPR` path and only diverts dedicated statement buckets such as whole-RHS `ASSIGN` and standalone `CALL`

This split is the right shape for later milestones:

- new procedure semantics should extend stage 1 only when the cloned body model grows
- new parent-expression or method/factory contexts should extend stage 2 without weakening structural safety checks

That separation is what keeps the inliner opportunity-based instead of turning `is_inlinable` into an over-broad “inline everywhere” flag.

Each later iteration in this area should continue to use a full build, the compiler regression suite, the top-level `ctest` matrix, and the focused `performance` label as its validation gate:

- `cmake --build cmake-build-debug -j4`
- `ctest --test-dir cmake-build-debug/compiler/tests --output-on-failure`
- `ctest --test-dir cmake-build-debug --output-on-failure`
- `ctest --test-dir cmake-build-debug -L performance --output-on-failure`

## Symbol and Scope Handling

Inlining requires duplicating callee-local AST, symbols, and scopes into a new isolated scope under the caller.

The immediate goal is not a fully general subtree cloning framework. For phase 1, the duplication logic only needs to be correct for the supported procedure shape used by the first rewrite.

Requirements:

- caller-visible symbols must continue to resolve to the caller
- callee locals must be duplicated as fresh symbols
- duplicated nodes must link to duplicated symbols, not the original callee symbols
- nested local scopes inside the cloned body must remain isolated

If the existing generic duplication helpers are not yet sound for that, the phase 1 implementation should use a narrower remapping path rather than pretending the generic case is solved.

## Optimisation Pipeline

Inlining still happens during optimisation, after the main validation/fixup pipeline.

However, the implementation should not assume that any arbitrary post-validation AST mutation is automatically safe. The rewrite must produce a tree compatible with the downstream emitter and optimisation passes for the supported phase.

That is another reason phase 1 should replace the enclosing assignment statement with a statement block, rather than replacing an expression node with a statement-only node.

## Argument Semantics During Inlining

Inlining must preserve the same argument semantics as a normal call:

- `.ref` / `expose` formals must continue to alias the caller target selected at the call site.
- By-value formals that are written in the callee must behave as isolated locals, even if the VM's native call mechanism is by reference.
- Read-only by-value formals may reuse the incoming value when that is already allowed in the non-inlined path.
- Non-symbol temporaries may reuse storage when no caller-visible binding exists to preserve, but that remains an implementation choice, not a semantic change.

For validation purposes, any inline rewrite that changes whether a caller variable, array, or object binding can be observed after the call is wrong even if the emitted code is smaller or faster.

## Later Phases

### Next Phase
The next meaningful extensions are now milestone-driven rather than milestone-1 cleanup work:

- methods and factories
- imported callees
- any later reconsideration of dynamic `.ref` / `expose` vararg indexing

Outside milestone 2 and 3, the remaining exclusions now split cleanly into:

- post-milestone enhancements that may or may not be worth the complexity, such as dynamic `.ref` vararg indexing
- safety and cost controls that should remain in some form: recursion-cycle blocking and body-size gating
- non-goals or structural policy exclusions: `main`
- language-validity constraints that are not extra inline restrictions: arity mismatches and invalid non-void fallthrough/value-return shapes

## Verification

The local-plain-procedure design should be considered established when:

- the compiler rewrites `inline_test1`, `inline_test_call`, `inline_test_expr`, `inline_test_concat_expr`, `inline_test_say_expr`, `inline_test_return_expr`, `inline_test_unary_expr`, `inline_test_compare_expr`, `inline_test_call_arg_expr`, `inline_test_call_like_arg_expr`, `inline_test_nested_call_expr`, `inline_test_ref_opt`, `inline_test_ref_indexed`, and `inline_test_ref_stem` using the narrow supported strategies
- excluded cases such as large procedures, methods, and imported callees remain uninlined under optimisation
- unsupported by-reference/short-circuit combinations such as those in `inline_test_ref_negative` remain uninlined under optimisation
- the resulting AST is structurally valid under `-dp`
- optimised codegen succeeds
- positive and negative compiler tests lock down the selector behaviour

## Non-Goals For The Current Design Boundary

- methods
- factories
- methods/factories and imported callees in expression position
- aggressive pruning of fully inlined procedures
- claiming fully generic scope/symbol cloning before it is proven
