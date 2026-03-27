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
- For expression inlining, the `FUNCTION` call must fall into one of the currently supported expression-context capability buckets.
- The callee must be a normal procedure, not a method or factory.
- Arguments and return values must stay within the currently implemented safe slice: scalars are broadly supported, object values are supported across the local-procedure slice, and array values are supported only where attribute-copy semantics can be preserved.
- The callee must satisfy the existing safety checks: current scalar slice only, small body, and no unsupported nested inlining. Value-producing procedures still require a final `RETURN`; void statement-call sites may inline through bare-return and fallthrough shapes.
- `expose`/by-reference formals are supported when the actual argument is an aliasable variable-like target, including indexed and stem-style forms.
- For nontrivial by-reference actuals, the inline rewrite captures the locator expressions once into inline-scope temps so the callee still sees call-time binding semantics.
- Optional formals are supported in the current slice when the inline rewrite can materialise their default from the formal AST.

At present, the supported expression-context buckets are:

- eager value consumers:
  `SAY`, `RETURN`
- eager call arguments:
  direct arguments to `FUNCTION(...)`, `FACTORY_CALL(...)`, and `MEMBER_CALL(...)` excluding the receiver child of `MEMBER_CALL`
- eager operators:
  unary `+`/`-`, arithmetic operators, concatenation operators, and comparison operators

Short-circuit boolean parents such as `|` and `&` remain intentionally excluded.

Composed inline sites are now supported when an outer call consumes an already-inlined `BLOCK_EXPR` actual, provided the outer context still falls into a supported eager bucket. This relies on scope-aware subtree cloning so nested `BLOCK_EXPR` locals and `LEAVE_WITH` associations remain isolated after the outer rewrite.

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

or other contexts where hoisting to surrounding statements would change semantics, especially around short-circuit evaluation.

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
- The current implementation supports the eager value-consumer bucket (`SAY`, `RETURN`) and the eager-operator bucket (unary `+`/`-`, plus non-short-circuit arithmetic, concatenation, and comparison operators).
- Other direct statement consumers are still intentionally excluded.
- Short-circuit boolean operators are still intentionally excluded.
- Unsupported expression contexts should remain normal calls until their rewrite strategy exists.

## Procedure Eligibility
For the first slice, a procedure is eligible only if all of the following hold:

- standard procedure only
- not `main`
- not a method
- not a factory
- fixed arguments or a supported trailing by-value vararg
- built-in scalar arguments only
- built-in scalar return only
- value-producing procedures must still end in a final `RETURN`
- void statement-call sites may also inline through bare-return and implicit-fallthrough shapes
- body size is below the configured node threshold
- no unsupported nested inline cases

These checks may be tightened further if needed to keep the first implementation simple and correct.

## Milestones

The milestone names below describe user-visible capability goals. They are intentionally broader than a single implementation slice, so each milestone is expected to contain several staged iterations.

### Milestone 1: all local procedures work

This milestone should be treated as two internal stages:

- `M1a`: local plain procedures within the current safe scalar slice, including fixed arguments, optional arguments, by-reference formals, and by-value varargs, across the currently supported call-site capability buckets
- `M1b`: local plain procedures beyond the current leaf restrictions, removing the temporary exclusions around nested calls, nested scopes, and single-trailing-`RETURN` only shapes

Notes:

- Varargs are the next sensible step, but varargs alone do not complete milestone 1.
- The current implementation now supports by-value varargs, nested-call local procedures, and nested callee scopes through a bounded fixed-point pass with explicit cycle blocking for self-recursive and mutually recursive expansions.
- The current implementation still excludes methods/factories and class-scope procedures. Object by-value formals now follow the same split as normal calls: read-only formals alias the incoming binding, while writable formals materialise an isolated local copy. Array values are still supported only where the inline rewrite can preserve attribute-copy semantics.
- Selection should remain opportunity-based throughout milestone 1: a structurally inlineable procedure may still have uninlined call sites if their rewrite bucket is not yet implemented.

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
- by-value trailing varargs
- nested-call local procedures via repeated identify-and-inline passes until a bounded fixed point is reached
- nested callee-local scopes with duplicated scope and symbol remapping
- multi/early-return procedures, including branch returns and void fallthrough in statement-call sites
- object-typed returns and by-value object formals
- array-typed formals and assignment-site returns where the inline rewrite can preserve attribute-copy semantics
- explicit cycle blocking so self recursion and mutual recursion do not expand indefinitely

The implementation still excludes:

- methods and factories
- imported callees
- array-valued expression rewrites that would need aggregate `BLOCK_EXPR` result propagation

Each iteration in this area should continue to use a full build and the compiler regression suite as its validation gate:

- `cmake --build cmake-build-debug -j4`
- `ctest --test-dir cmake-build-debug/compiler/tests --output-on-failure`

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
The next meaningful extensions are the remaining argument-semantics and evaluation-order cases that are still intentionally excluded:

- broader by-reference actual shapes beyond direct aliasable symbol targets
- varargs
- short-circuit boolean parents and other contexts where evaluation ordering must be modelled explicitly

## Verification
The design for phase 1 should be considered ready when:

- the compiler rewrites `inline_test1`, `inline_test_call`, `inline_test_expr`, `inline_test_concat_expr`, `inline_test_say_expr`, `inline_test_return_expr`, `inline_test_unary_expr`, `inline_test_compare_expr`, `inline_test_call_arg_expr`, `inline_test_call_like_arg_expr`, `inline_test_nested_call_expr`, `inline_test_ref_opt`, `inline_test_ref_indexed`, and `inline_test_ref_stem` using the narrow supported strategies
- excluded cases such as large procedures, methods, multi-return procedures, and unsupported short-circuit contexts remain uninlined under optimisation
- unsupported expression contexts such as those in `inline_test_expr_negative`, `inline_test_say_expr_negative`, and `inline_test_bool_expr_negative` remain uninlined under optimisation
- unsupported by-reference/short-circuit combinations such as those in `inline_test_ref_negative` remain uninlined under optimisation
- the resulting AST is structurally valid under `-dp`
- optimised codegen succeeds
- positive and negative compiler tests lock down the selector behaviour

## Non-Goals For The First Step

- methods
- factories
- varargs
- broader pass-by-reference support beyond the current variable-like/indexed/stem slice
- general embedded-expression inlining beyond the current eager-bucket slice
- aggressive pruning of fully inlined procedures
- claiming fully generic scope/symbol cloning before it is proven
