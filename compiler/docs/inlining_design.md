# cREXX Inlining Design

## Overview
Inlining in `rxc` is an AST rewrite performed during optimisation. The immediate goal is a buildable, staged implementation where the inliner only selects call sites whose rewrite strategy is already implemented.

That means selection is opportunity-based, not just callee-based. A procedure may be generally eligible for inlining, but only some uses of it may be rewritten in a given phase.

## Current Strategy

### Phase 1 target
The first supported scenario is:

```rexx
x = func(a)
```

More precisely:

- The RHS must be a `FUNCTION` call.
- The `FUNCTION` call must be the entire RHS of the enclosing `ASSIGN`.
- The callee must be a normal procedure, not a method or factory.
- The callee must satisfy the existing safety checks: fixed pass-by-value args only, single trailing `RETURN`, small body, no unsupported nested inlining.

The first implementation should rewrite the enclosing `ASSIGN` statement, not the `FUNCTION` node in isolation.

### Why selection must stay narrow
The inliner must not mark a procedure as globally "inline now" and then rewrite every call. That breaks the incremental rollout plan. Instead:

1. Identify procedures that are structurally inlineable.
2. Identify call sites whose context is supported by the currently implemented rewrite.
3. Rewrite only those call sites.

This preserves the ability to build and test the compiler while inlining support is still partial.

## `COMPILER_ADDED_BLOCK` vs `BLOCK_EXPR`

Both node types are needed.

### `COMPILER_ADDED_BLOCK`
Use this for statement-position rewrites that need:

- scope isolation
- a sequence of injected statements
- no expression value of their own

This is the right tool for phase 1, because `x = func(a)` should become a statement block that contains:

1. argument binding
2. cloned callee body
3. `RETURN expr` rewritten to `x = expr`

The original assignment statement is replaced as a whole by the block.

### `BLOCK_EXPR`
Use this only when the inline expansion itself must remain an expression and yield a value to its parent expression tree.

This is for later phases such as:

```rexx
x = func(a) + y
```

or other contexts where hoisting to surrounding statements would change semantics, especially around short-circuit evaluation.

`BLOCK_EXPR` already has the compiler machinery for:

- scoped expression blocks
- `LEAVE_WITH` result propagation
- emitter support for yielding a register result

So for expression-valued inlining, the target form should be a `BLOCK_EXPR`, not a raw `COMPILER_ADDED_BLOCK`.

### Decision
Do not drop `COMPILER_ADDED_BLOCK` in favour of `BLOCK_EXPR`.

Use:

- `COMPILER_ADDED_BLOCK` for statement-only inlining rewrites
- `BLOCK_EXPR` for expression-valued inlining rewrites

Also avoid wrapping a `BLOCK_EXPR` around a `COMPILER_ADDED_BLOCK` unless a specific implementation need appears. For expression contexts, prefer constructing the final `BLOCK_EXPR` shape directly.

## AST Rewrite Plan

### Phase 1 rewrite: `x = func(a)`
Given:

```rexx
x = func(a)
```

rewrite the enclosing assignment into:

1. a `COMPILER_ADDED_BLOCK`
2. containing an `INSTRUCTIONS` list
3. containing argument binding statements
4. containing the cloned callee statements
5. with the callee `RETURN expr` rewritten to `x = expr`

Important details:

- Formal parameter binding must target the formal variable node, not the `ARG` node.
- Actual arguments must be read from the real call shape used by `FUNCTION` nodes.
- The cloned body must use isolated local symbols and scopes.
- The result should be a valid statement tree without forcing `COMPILER_ADDED_BLOCK` into expression position.

## Procedure Eligibility
For the first slice, a procedure is eligible only if all of the following hold:

- standard procedure only
- not `main`
- not a method
- not a factory
- fixed arguments only
- no `.ref`, `.opt`, or varargs
- exactly one `RETURN`
- the `RETURN` is the final instruction
- body size is below the configured node threshold
- no unsupported nested inline cases

These checks may be tightened further if needed to keep the first implementation simple and correct.

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

## Later Phases

### Phase 2
Standalone call statements such as:

```rexx
call func(a)
```

This likely remains a `COMPILER_ADDED_BLOCK` rewrite in statement position.

### Phase 3
Embedded-expression inlining such as:

```rexx
x = func(a) + y
```

This should use `BLOCK_EXPR`, with the inlined block yielding its value through `LEAVE_WITH` or an equivalent direct block-expression result path.

## Verification
The design for phase 1 should be considered ready when:

- the compiler rewrites `inline_test1` using the narrow statement-position strategy
- the resulting AST is structurally valid under `-dp`
- optimised codegen succeeds
- the relevant compiler tests stop depending on `WILL_FAIL`

## Non-Goals For The First Step

- methods
- factories
- pass-by-reference
- optional args
- varargs
- general embedded-expression inlining
- aggressive pruning of fully inlined procedures
- claiming fully generic scope/symbol cloning before it is proven
