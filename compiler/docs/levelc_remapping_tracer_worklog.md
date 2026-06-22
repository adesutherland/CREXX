# Level C Remapping Tracer Worklog

Status: active implementation log for the remapping/inlining tracer bullet.

This file records the steps, issues, and resolutions needed to replay the
tracer from clean code if the branch is rolled back. Keep entries factual and
ordered by implementation step.

## 2026-06-21: Inlining-First Tracer Start

### Goal

Implement the first remapping tracer by converting the existing inliner bucket
selection into stable remapping rule descriptors, while preserving current
inliner behaviour and tests.

The first pass deliberately avoids:

- a new external rule syntax;
- Level C compile enablement;
- semantic changes to inline clone/bind/return rewriting.

### Replay Steps

1. Add this worklog.
2. Keep `compiler/docs/levelc_remapping_target.md` as the design target.
3. In `compiler/rxcp_inline.c`, add internal inline remap rule descriptors for
   the existing call-site buckets:
   - `inline.rhs-eager-operator.capture-left`
   - `inline.expression.block`
   - `inline.assignment.whole-rhs`
   - `inline.call.statement`
4. Route `inline_procedure_walker()` through the descriptor dispatcher instead
   of open-coded bucket selection.
5. Keep the existing `ast_inline_*` builders as the semantic implementation.
6. Add debug-visible rule identities around rewrite attempts so existing
   fail-closed messages can be associated with the active remap rule.
7. Run focused inline tests and record results below.

### Issues And Resolutions

1. `16_classes_parse` golden output changed after active remap rule ids were
   added to fail-closed inline debug messages.
   - Symptom: generated output included
     `rule=inline.expression.block` on the existing `DEBUG_INLINE_FAILCLOSED`
     line.
   - Resolution: keep the rule id because tracer visibility is intentional,
     and update the single affected parsing golden line.
   - Replay note: rerun the focused parsing probe after updating
     `compiler/tests/golden/parsing/16_classes.txt`.

### Verification

Green stop for implementation stage 1:

- `cmake --build cmake-build-release --target rxc --parallel 4`
  - result: passed, no work needed after the final source state
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: 3/3 passed
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: 254/254 passed
- `git diff --check`
  - result: passed
- Direct trailing-whitespace check over touched source/doc/golden files
  - result: passed

### Stage 1 Result

`inline_procedure_walker()` now dispatches through internal remapping rule
descriptors for the four current top-level inline call-site buckets. Existing
`ast_inline_*` builders still own the semantic rewrites, so clone/bind/return
behaviour is unchanged.

Converted bucket rules:

- `inline.rhs-eager-operator.capture-left`
- `inline.expression.block`
- `inline.assignment.whole-rhs`
- `inline.call.statement`

Remaining tracer work:

- Lift structural eligibility into a rule/catalog form.
- Decide whether clone/bind/return services should become named remap
  subrules or remain services used by top-level rules.

## 2026-06-21: Structural Eligibility Rule Identity

### Goal

Lift the structural inlinability decision into the same remapping vocabulary as
the call-site buckets, without changing eligibility semantics or the existing
`DEBUG_INLINE` acceptance/rejection text.

### Replay Steps

1. Add the internal rule descriptor `inline.eligibility.structural`.
2. Keep `inline_analyse_callable_eligibility()` as the semantic gate.
3. Emit `DEBUG_INLINE_REMAP` accept/reject lines at `-d2` and higher using the
   new structural rule descriptor.
4. Keep normal parser goldens stable by not adding structural rule ids to the
   existing `DEBUG_INLINE` lines.

### Issues And Resolutions

No Stage 2 implementation issues recorded.

### Verification

Green stop for implementation stage 2:

- `cmake --build cmake-build-release --target rxc --parallel 4`
  - result: passed
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: 3/3 passed
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: 254/254 passed
- `cmake-build-release/bin/rxc -i cmake-build-release/bin -d2 -o /tmp/inline_remap_debug compiler/tests/rexx_src/inline_test_expr.crexx`
  - result: passed
  - observed `DEBUG_INLINE_REMAP` lines for
    `inline.eligibility.structural` and `inline.expression.block`
- `git diff --check`
  - result: passed
- Direct trailing-whitespace check over touched source files
  - result: passed

### Stage 2 Result

Structural eligibility is now named in the remapping tracer as
`inline.eligibility.structural`. The current eligibility code remains the
source of truth; the remap layer records the decision instead of reimplementing
it.

Remaining tracer work:

- Decide whether clone/bind/return services should become named remap
  subrules or remain services used by top-level rules.

## 2026-06-21: Bind Actuals Service Boundary

### Goal

Expose the shared actual-binding service as a named remap boundary while
leaving the existing binding implementation intact. This makes the tracer cover
the call-site selector, structural eligibility, and the first shared expansion
service without converting clone/bind/return internals into a new engine.

### Decision

`inline.bind.actuals` is represented as a remap service boundary. The bind
implementation remains the existing `inline_bind_call_arguments()` function.
This keeps by-value, optional/default, by-reference, vararg, method receiver,
and factory binding semantics unchanged.

Return rewriting, body cloning, and receiver copyback remain implementation
services for now. They should become separate remap subrules only when a later
change needs rule-specific guards, tests, or debug reasons inside those
services.

### Replay Steps

1. Add the internal rule descriptor `inline.bind.actuals`.
2. Use the existing `INLINE_BIND_RETURN()` macro in
   `inline_bind_call_arguments()` to emit `DEBUG_INLINE_REMAP` applied/rejected
   results at the service boundary.
3. Do not change any binding branches or return values.
4. Keep existing focused inline tests green.

### Issues And Resolutions

No Stage 3 implementation issues recorded.

### Verification

Green stop for implementation stage 3:

- `cmake --build cmake-build-release --target rxc --parallel 4`
  - result: passed
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: 3/3 passed
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: 254/254 passed
- `cmake-build-release/bin/rxc -i cmake-build-release/bin -d2 -o /tmp/inline_remap_debug compiler/tests/rexx_src/inline_test_expr.crexx`
  - result: passed
  - observed `DEBUG_INLINE_REMAP` lines for
    `inline.eligibility.structural`, `inline.bind.actuals`, and
    `inline.expression.block`
- `git diff --check`
  - result: passed
- Direct trailing-whitespace check over touched source files
  - result: passed

### Stage 3 Result

The current tracer now names:

- structural inlinability: `inline.eligibility.structural`
- call-site bucket rewrites:
  - `inline.rhs-eager-operator.capture-left`
  - `inline.expression.block`
  - `inline.assignment.whole-rhs`
  - `inline.call.statement`
- shared actual binding: `inline.bind.actuals`

That covers the current inliner rule set at the safe tracer boundary. Deeper
clone/return/copyback subrules remain deliberately unconverted services until
there is a concrete need to split their internal guard logic.

## 2026-06-22: Inline Rule Master Table

### Goal

Before adding a selector-step implementation, capture the current inlining
surface as a rule table. This is the review checklist for whether static
selectors plus named C bind/guard/build helpers can express the existing
inliner without hiding semantics inside a single callback.

### Rule Table

| Rule | Selector | Captures | Binds | Guards | Rewrite/Treatment | Current C owner | Focused tests |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `inline.eligibility.structural` | `callable:[PROCEDURE|METHOD|FACTORY]` | `callable` | `sym`, `eligibility` | not main, usable scope, class owner shape, no procedure-level `EXPOSE`, body/args/return shape, node cutoff, recursion/unsupported reference/assembler/varg checks | mark symbol inlineable and attach AST template, or reject fail-closed | `identify_inlinable_walker`, `inline_analyse_callable_eligibility` | full `inline|Inline` slice, negative inline fixtures |
| `inline.rhs-eager-operator.capture-left` | `op:eager_operator(left:_, rhs:[FUNCTION|MEMBER_CALL|FACTORY_CALL])` | `op`, `left`, `rhs` | `proc_sym` from `rhs` | `rhs` is inlineable, no sibling after `rhs`, left capture needed, parent scope exists | replace `op` with `BLOCK_EXPR` that captures `left`, then `LEAVE_WITH temp <op> clone(rhs)` for the next fixed-point inline pass | `inline_remap_apply_rhs_eager_operator`, `ast_inline_rhs_eager_operator` | composed expression/block expr inline fixtures |
| `inline.expression.block` | `call:[FUNCTION|MEMBER_CALL|FACTORY_CALL]` | `call`, parent by AST link | `proc_sym`, expression context | inlineable call, not RHS-left-capture case, supported direct value consumer, return shape, receiver copyback/imported receiver restrictions | replace call with `BLOCK_EXPR`; bind actuals, clone body, rewrite returns to `LEAVE_WITH` | `inline_remap_apply_expression_block`, `ast_inline_expression`, `inline_build_block_expr` | expression, call-argument, condition, operator, type operator fixtures |
| `inline.assignment.whole-rhs` | `assign:ASSIGN(lhs:_, rhs:[FUNCTION|MEMBER_CALL|FACTORY_CALL])` | `assign`, `lhs`, `rhs` | `proc_sym`, return shape | inlineable RHS call, plain/fallback LHS rules, final value return or `BLOCK_EXPR` fallback, receiver copyback restrictions | replace whole assignment with compiler block, or replace RHS with `BLOCK_EXPR` fallback for complex LHS/aggregate cases | `inline_remap_apply_assignment_whole_rhs`, `ast_inline_assignment`, `ast_inline_statement`, `inline_build_block_expr` | assignment, complex LHS, array/object/binary return fixtures |
| `inline.call.statement` | `stmt:CALL(call:[FUNCTION|MEMBER_CALL|FACTORY_CALL])` | `stmt`, `call` | `proc_sym`, return shape | inlineable call, void/value return sink rules, receiver copyback/imported receiver restrictions | replace statement with compiler block; ignored return values are evaluated into a sink when needed | `inline_remap_apply_call_statement`, `ast_inline_call`, `ast_inline_statement`, `inline_build_block_expr` | standalone call, void fallthrough, multi-return call fixtures |
| `inline.bind.actuals` | service boundary over callee args and call actuals | `proc_def`, `call_node`, formals/actuals by traversal | receiver capture, scoped actual captures, vararg symbols, ref maps | arity, scoped capture success, factory init, method receiver bind, optional defaults, ref actual support, vararg support | emit binding/capture prologue into inline instruction list | `inline_bind_call_arguments` | ref, vararg, optional/default, object/array actual fixtures |
| `inline.return.rewrite` | service boundary over cloned callee body | `RETURN` nodes, enclosing block/statement plan | return target or sink, receiver copyback state | final return requirements already proved; dummy return allowed only in void block-expression cases | rewrite to assignment, sink assignment, or `LEAVE_WITH`; may wrap receiver copyback before `LEAVE_WITH` | `inline_rewrite_return_nodes`, `ast_inline_statement`, `inline_create_receiver_copyback_leave_wrapper` | multi-return, void, expression block, receiver copyback fixtures |
| `inline.clone.body` | service boundary over callee instruction subtree | callee body nodes | scope map, symbol map, node association map | clone scope/symbol allocation succeeds, unsupported associations fail closed elsewhere | clone body into inline scope with remapped symbols/scopes and source anchors | `inline_clone_subtree`, `inline_build_symbol_map`, `inline_clone_scope` | nested scope, imported body, source metadata fixtures |
| `inline.receiver.copyback` | service boundary over mutating method receiver | receiver, cloned `§this` | source receiver symbol, cloned receiver symbol | mutating method writes class attribute, direct non-attribute receiver copyback target | append copyback after statement body or wrap before `LEAVE_WITH` | `inline_bind_method_receiver`, `inline_append_method_receiver_copyback` | local class method mutation fixtures |
| `inline.imported-template` | metadata payload attached to imported callable symbol | payload, callable template | restored source/scope/symbol metadata | payload version supported, dependencies/signature compatible, writer/reader gates pass | attach read-only imported AST template and feed through normal local inline rules | `rxcp_inline_attach_imported_body`, `inline_meta_*` | cross-file inline and RXDAS preserve fixtures |

### Selector Rule Direction

The first real static selector-step specimen should be
`inline.rhs-eager-operator.capture-left`. It has a small structural shape,
captures three nodes, binds one semantic value, and builds one replacement. It
is also intentionally staged: the replacement leaves the RHS call in place so
the next fixed-point pass can inline it through `inline.expression.block`.

## 2026-06-22: Static Selector-Step Specimen

### Goal

Implement one real selector-backed remap rule using a static selector-step
array, without introducing an external DSL parser. This proves the shape:

1. selector steps fill capture cells;
2. a named bind helper derives semantic values;
3. a named guard helper proves the rewrite site;
4. a named rewrite helper delegates to the existing tree surgery builder.

### Implemented Rule

`inline.rhs-eager-operator.capture-left` now uses this static selector shape:

```text
op:eager_operator(
  left:_,
  rhs:[FUNCTION|MEMBER_CALL|FACTORY_CALL]
)
assert rhs has no next sibling
```

Captured cells:

- `op`
- `left`
- `rhs`

Named C blocks:

- bind: `inline_bind_rhs_eager_operator_proc_symbol`
- guard: `inline_guard_rhs_eager_operator_capture_safe`
- rewrite: `inline_rewrite_rhs_eager_operator_capture`

The other top-level inline call-site rules remain callback-backed for now.
This keeps the stage narrow and leaves a readable specimen for review.

### Replay Steps

1. Add `InlineRemapSelectorStep`, selector op enums, node-class/type-set enums,
   and selector convenience macros in `compiler/rxcp_inline.c`.
2. Extend `InlineRemapRule` with optional `selector`, `bind`, `guard`,
   `rewrite`, and `debug_site_capture` fields.
3. Add `inline_remap_run_selector()` and capture lookup helpers.
4. Add the static selector array `inline_rhs_eager_operator_selector`.
5. Change only `inline.rhs-eager-operator.capture-left` from callback-backed to
   selector-backed.
6. Keep the existing `ast_inline_rhs_eager_operator()` builder as the actual
   replacement implementation.

### Issues And Resolutions

No Stage 4 implementation issues recorded.

### Verification

Green stop for implementation stage 4:

- `cmake --build cmake-build-release --target rxc --parallel 4`
  - result: passed
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: 3/3 passed
- `cmake-build-release/bin/rxc -i cmake-build-release/bin -d2 -o /tmp/inline_selector_debug_rhs compiler/tests/rexx_src/inline_test_composed_expr_contexts.crexx`
  - result: passed
  - observed `DEBUG_INLINE_REMAP ... rule=inline.rhs-eager-operator.capture-left`
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: 254/254 passed
- `git diff --check`
  - result: passed
- Direct trailing-whitespace check over touched files
  - result: passed

### Stage 4 Result

There is now a concrete rule defined as selector plus bind/guard/rewrite
building blocks. This is the first inspectable mapping-rule specimen and should
be used as the pattern for deciding whether to convert the remaining inline
rules to selector-backed form.
