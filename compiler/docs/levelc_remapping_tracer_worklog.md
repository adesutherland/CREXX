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
  - result: passed, 3/3 tests
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: passed, 254/254 tests
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

## 2026-06-22: Readable Rule Shape

### Goal

Make the selector-backed rule easier to read and extend. The previous rule
table entry worked, but it was still too positional: a maintainer had to know
which struct field was selector, bind, guard, rewrite, or callback. This stage
keeps the same behaviour but turns the rule into arrays of named building
blocks plus rule declaration macros.

### Implemented Shape

The model rule now reads as four pieces:

1. `inline_rhs_eager_operator_selector`
2. `inline_rhs_eager_operator_binds`
3. `inline_rhs_eager_operator_guards`
4. `inline_rhs_eager_operator_rewrites`

The table entry uses `INLINE_REMAP_RULE_SELECTOR(...)` to connect those pieces.
Callback-backed legacy entries use `INLINE_REMAP_RULE_CALLBACK(...)`, and
service/debug-only descriptors use `INLINE_REMAP_RULE_SERVICE(...)`.

This deliberately supports multiple bind, guard, and rewrite steps per rule,
even though the first specimen uses one of each.

### Replay Steps

1. Add `InlineRemapBindStep`, `InlineRemapGuardStep`, and
   `InlineRemapRewriteStep`.
2. Add `INLINE_BIND_STEP`, `INLINE_GUARD_STEP`, and `INLINE_REWRITE_STEP`
   array macros plus terminators.
3. Extend `InlineRemapRule` to point to arrays of bind, guard, and rewrite
   steps.
4. Add readable declaration macros:
   - `INLINE_REMAP_RULE_SELECTOR`
   - `INLINE_REMAP_RULE_CALLBACK`
   - `INLINE_REMAP_RULE_SERVICE`
5. Update `inline_remap_apply_selector_rule()` to run all bind steps, then all
   guard steps, then all rewrite steps.
6. Keep the same selector, same semantic helpers, and same tree surgery
   builder for `inline.rhs-eager-operator.capture-left`.

### Issues And Resolutions

- The first temp-log debug wrapper used `status` as a shell variable, which is
  read-only under zsh. Reran the same smoke with `rc`; no code change was
  needed.

### Verification

Green stop for implementation stage 5:

- `cmake --build cmake-build-release --target rxc --parallel 4`
  - result: passed
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: 3/3 passed
- Temp-log wrapped `cmake-build-release/bin/rxc -i cmake-build-release/bin -d2 -o /tmp/inline_rule_shape_debug compiler/tests/rexx_src/inline_test_composed_expr_contexts.crexx`
  - result: passed
  - observed `DEBUG_INLINE_REMAP ... rule=inline.rhs-eager-operator.capture-left`
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: 254/254 passed
- `git diff --check`
  - result: passed
- Direct trailing-whitespace check over touched files
  - result: passed

### Stage 5 Result

The model rule is now maintainable enough to read as a rule instead of a bundle
of callbacks. The selector remains the structural part; binds, guards, and
rewrites are named C building blocks that can be reused or split as later rules
need.

## 2026-06-22: Inline Rule Catalog Split

### Goal

Refactor the inlining remap tracer into a source layout that can grow into the
Level C remapping substrate without trapping generic mechanics behind
`inline_` names. Also convert the current call-site inline buckets to
selector-backed rule entries so the compiler can print a readable catalog of
the active inline rules.

### Implemented Shape

The shared remap substrate now lives in:

- `compiler/rxcp_remap.h`
- `compiler/rxcp_remap.c`

It owns generic rule descriptors, selector steps, captures, bind/guard/rewrite
step arrays, rule declaration macros, and selector execution. It is intentionally
not inline-specific.

The inline private contract now lives in:

- `compiler/rxcp_inline_internal.h`

The reader-facing inline rule catalog now lives in:

- `compiler/rxcp_inline_rules.c`

`compiler/rxcp_inline.c` keeps the heavy existing inline mechanics: eligibility
analysis, actual binding, clone/scope/symbol mapping, return rewriting,
copyback, and inline payload import/export. Generic tree-build helpers were not
split out yet; they should move only when Level C or another remap family needs
the same helper outside inline semantics.

### Rule Catalog

The current call-site rules are now all selector-backed:

1. `inline.rhs-eager-operator.capture-left`
2. `inline.expression.block`
3. `inline.assignment.whole-rhs`
4. `inline.call.statement`

The service/debug boundaries remain represented as service rules:

1. `inline.eligibility.structural`
2. `inline.bind.actuals`

The reusable selector vocabulary gained exact node-type matching via
`RXCP_REMAP_SEL_TYPE(...)`, used by the assignment and call-statement rules.

### Human-Readable Summary

Set `RXCP_INLINE_RULE_SUMMARY=1` when running `rxc` to print the current inline
rule catalog. The output is deliberately simple text:

- rule id, phase, root shape, priority, and kind
- selector steps and capture names
- bind, guard, and rewrite step ids

This gives us a cheap review tool and a regression signal: if a rule is hard to
read in this summary, the rule definition is probably not yet in the right
shape.

### Replay Steps

1. Add `rxcp_remap.h/.c` with generic `RxcpRemap*` descriptors, selector
   execution, capture lookup, and summary support helpers.
2. Add `rxcp_inline_internal.h` for private cross-file inliner types and helper
   declarations.
3. Move the inline rule catalog and selector-backed model rule into
   `rxcp_inline_rules.c`.
4. Promote only the helpers needed by the rule catalog out of `static`:
   inlineable-call lookup, RHS eager capture check, the existing `ast_inline_*`
   rewrite builders, and remap debug rule-id hooks.
5. Convert the three remaining callback-backed call-site buckets to
   selector/bind/guard/rewrite arrays.
6. Add the explicit `RXCP_INLINE_RULE_SUMMARY=1` summary hook.
7. Add the new files to `compiler/CMakeLists.txt`.

### Issues And Resolutions

- The summary initially labelled service rules as `selector: <callback>`.
  Changed the summary wording to print `<service>` for service descriptors.
- `inline.expression.block` needs to reject the RHS-left-capture case at the
  call node so the parent eager-operator rule can handle it later in the same
  walker pass. This preserves the previous callback behaviour and is visible in
  `-d2` traces as a rejected expression rule followed by an applied parent
  capture rule.

### Verification

Green stop for implementation stage 6:

- `cmake --build cmake-build-release --target rxc --parallel 4`
  - result: passed
- `RXCP_INLINE_RULE_SUMMARY=1 cmake-build-release/bin/rxc -i cmake-build-release/bin -o /tmp/inline_rule_summary_probe compiler/tests/rexx_src/inline_test_composed_expr_contexts.crexx`
  - result: passed
  - observed selector/bind/guard/rewrite summaries for all four call-site rules
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: 3/3 passed
- Temp-log wrapped `cmake-build-release/bin/rxc -i cmake-build-release/bin -d2 -o /tmp/inline_rule_shape_debug compiler/tests/rexx_src/inline_test_composed_expr_contexts.crexx`
  - result: passed
  - observed selector-backed `inline.expression.block` and
    `inline.rhs-eager-operator.capture-left` call-site rule traces
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: 254/254 passed

### Stage 6 Result

The inline remap implementation now has a reusable remap substrate and a
separate inline rule catalog. The active call-site inlining rules can be read
directly from `rxcp_inline_rules.c` or printed with
`RXCP_INLINE_RULE_SUMMARY=1`, which is the first useful maintainer-facing view
of the remapping approach.

## 2026-06-22: Shared Service Runner

### Goal

Move inline service boundaries onto shared remap machinery. The previous rule
catalog showed the service descriptors, but each service still owned its own
ad-hoc trace and result handling. This stage adds one remap service runner and
uses it for both selector rewrites and inline expansion services.

### Implemented Shape

`compiler/rxcp_remap.h/.c` now provides:

- `RxcpRemapServiceFn`
- `RxcpRemapHooks`
- `rxcp_remap_run_service(...)`

The runner owns the common service lifecycle:

1. optionally enter an active rule id
2. call the service body
3. optionally leave the active rule id
4. trace `applied` or `rejected`
5. return `RXCP_REMAP_APPLIED` or `RXCP_REMAP_REJECTED`

Inline provides two hook sets:

- `rxcp_inline_remap_hooks()`: enter/leave plus trace, used where active
  rule-id context already existed for rewrites.
- `rxcp_inline_remap_trace_hooks()`: trace only, used for service boundaries
  where wrapping regular `DEBUG_INLINE` text would change existing diagnostics.

### Service Boundaries

The printed inline summary now includes these service rules:

1. `inline.eligibility.structural`
2. `inline.bind.actuals`
3. `inline.clone.body`
4. `inline.return.rewrite`
5. `inline.receiver.copyback`

The first two were already descriptors; the latter three are now represented
and exercised through the shared runner at the safe top-level service calls.
General subtree cloning remains a utility because it is used for many purposes;
only callee instruction subtree cloning is labelled `inline.clone.body`.

### Replay Steps

1. Add `RxcpRemapHooks`, `RxcpRemapServiceFn`, and
   `rxcp_remap_run_service(...)`.
2. Add inline hook adapters for active-rule enter/leave and remap tracing.
3. Route selector rewrite execution through `rxcp_remap_run_service(...)`.
4. Split `inline_bind_call_arguments()` into a service wrapper and unchanged
   implementation body.
5. Split structural eligibility into a service body and runner call.
6. Add and wire service descriptors for body clone, return rewrite, and
   receiver copyback.
7. Add those service descriptors to `RXCP_INLINE_RULE_SUMMARY=1`.

### Issues And Resolutions

- Full enter/leave hooks around structural eligibility would have changed
  existing `DEBUG_INLINE` golden output by adding active rule ids. Used
  trace-only service hooks for expansion/eligibility services and kept full
  hooks for selector rewrites, preserving existing diagnostics.
- Body cloning is intentionally scoped to callee instruction subtree cloning.
  Other `inline_clone_subtree(...)` calls remain ordinary mechanics until a
  second remap family proves they are generic materialisation services.

### Verification

Green stop for implementation stage 7:

- `cmake --build cmake-build-release --target rxc --parallel 4`
  - result: passed
- `RXCP_INLINE_RULE_SUMMARY=1 cmake-build-release/bin/rxc -i cmake-build-release/bin -o /tmp/inline_service_summary_probe compiler/tests/rexx_src/inline_test_composed_expr_contexts.crexx`
  - result: passed
  - observed service boundaries for eligibility, actual binding, body clone,
    return rewrite, and receiver copyback
- Temp-log wrapped `cmake-build-release/bin/rxc -i cmake-build-release/bin -d2 -o /tmp/inline_service_debug compiler/tests/rexx_src/inline_test_composed_expr_contexts.crexx`
  - result: passed
  - observed `inline.bind.actuals`, `inline.clone.body`, and
    `inline.return.rewrite` service traces
- Temp-log wrapped `cmake-build-release/bin/rxc -i cmake-build-release/bin -d2 -o /tmp/inline_copyback_debug compiler/tests/rexx_src/inline_test_class_methods.crexx`
  - result: passed
  - observed `inline.receiver.copyback` service traces
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: 3/3 passed
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: 254/254 passed

### Stage 7 Result

The remap layer now has shared service execution, and the inliner uses it for
the current rule rewrites plus the main expansion services. This gives Level C
a concrete place to plug in future lowering services without depending on
inline-only trace mechanics.

## 2026-06-22: Inline Implementation Source Split

### Goal

Move the heavy inline internals out of the `rxcp_inline.c` monolith so the rule
catalog and service boundaries can be reviewed without wading through all clone,
bind, rewrite, analysis, and payload mechanics. This stage is deliberately a
mechanical file split, not a semantic rewrite.

### Implemented Shape

`compiler/rxcp_inline.c` is now the inline shell for includes, forward
declarations, debug hooks, remap hooks, and the private implementation fragment
order.

The heavy inline internals are split into private implementation fragments:

- `compiler/rxcp_inline_bind.c`
- `compiler/rxcp_inline_clone.c`
- `compiler/rxcp_inline_rewrite.c`
- `compiler/rxcp_inline_analysis.c`
- `compiler/rxcp_inline_payload.c`

These fragments are intentionally included by `rxcp_inline.c` and marked
`HEADER_FILE_ONLY` in CMake. They are not independently compiled yet because
the inline internals still share many static helpers. That keeps this stage
low-risk and preserves the exact static dependency graph while making each
area reviewable.

### File Responsibilities

- `rxcp_inline_bind.c`: actual/formal binding, ref/vararg capture, receiver
  binding, factory setup, receiver copyback service, and related materializers.
- `rxcp_inline_clone.c`: clone maps, symbol/scope duplication, inline body
  clone service, clone cleanup, and clone-side helpers.
- `rxcp_inline_rewrite.c`: statement/expression/block rewrites, return rewrite
  service, copyback leave wrapper, recursion/call-site validation helpers, and
  the `ast_inline_*` builders.
- `rxcp_inline_analysis.c`: inline eligibility debug/reporting, structural
  eligibility service, inlinable walker, and inline pass entry point.
- `rxcp_inline_payload.c`: prune support plus inline metadata import/export.

### Replay Steps

1. Split `rxcp_inline.c` at stable function-boundary markers.
2. Add private-fragment comments to each new file.
3. Include the fragments from `rxcp_inline.c` in the original order.
4. Add a comment in `rxcp_inline.c` explaining why the fragments remain in one
   translation unit.
5. List the fragments in `compiler/CMakeLists.txt` with `HEADER_FILE_ONLY`
   so project tooling sees them but does not compile them twice.

### Issues And Resolutions

- A true separate-compilation split would require exposing or moving a large
  number of static helper dependencies at once. Chose private fragments first
  to reduce review risk and avoid churn while preserving a clear next step.

### Verification

Green stop for implementation stage 8:

- `cmake --build cmake-build-release --target rxc --parallel 4`
  - result: passed
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: passed, 3/3 tests
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: passed, 254/254 tests

### Stage 8 Result

The first two inline refactor gaps are addressed at the source-layout level:
heavy mechanics are no longer buried in one file, and the mechanics are grouped
by bind, clone, rewrite, analysis, and payload responsibility. The next step is
to thin dependencies inside these fragments enough to promote selected ones to
independently compiled sources or to extract Level C-neutral builders.

## 2026-06-22: Shared Remap Builder Materialisation Layer

### Goal

Extract the reusable generated-tree mechanics from the inliner into a neutral
remap builder layer. This is deliberately broader than a narrow helper move:
Level C lowering, future Rexx front ends, and optimiser rewrites all need the
same reliable ways to create compiler-owned scopes, temps, anchors,
assignments, block expressions, and capture-once rewrites.

### Implemented Shape

Added:

- `compiler/rxcp_remap_build.h`
- `compiler/rxcp_remap_build.c`

The layer now owns reusable mechanics for:

- synthetic source anchors and generated-block marking;
- numeric-context copying;
- generated scopes, including named cloned scopes and local rewrite scopes;
- generated local/temp symbols shaped from AST nodes;
- symbol refs/targets linked back to `Symbol` usage records;
- integer constants;
- ordinary assignments and assignment-to-symbol materialisation;
- sink targets for intentionally discarded values;
- `LEAVE WITH` construction;
- `BLOCK_EXPR` scaffolds with child instruction lists;
- capture-once rewrites using a caller-provided expression materializer.

The inliner remains responsible for inline-specific policy: eligibility,
call-site guards, actual/formal binding choices, subtree cloning, receiver
copyback decisions, assembler copy instructions, and the exact rewrite rule
catalog.

### Replay Steps

1. Add `rxcp_remap_build.[ch]` beside the existing remap selector/service
   substrate and include it in `rxclib`.
2. Route inline-generated scope creation through `rxcp_remap_create_scope()` or
   `rxcp_remap_create_local_scope()`.
3. Replace inline-local temp, symbol-node, integer-constant, sink-target,
   source-anchor, and generated-block helper bodies with the shared builder
   functions.
4. Replace broad generated block-expression scaffolds with
   `rxcp_remap_create_block_expr()`.
5. Replace straightforward generated assignments and `LEAVE WITH` nodes with
   shared assignment/leave builders.
6. Keep inline-specific copyback, register-copy, vararg policy, and clone
   decisions in the inline fragments.

### Issues And Resolutions

- The shared `capture-once` helper cannot know how to materialise a source
  expression. It takes a materializer callback; inline passes a clone callback,
  while Level C can later pass a lowering/materialisation callback.
- Generated instruction blocks need to distinguish the node that supplies the
  printed/token shape from the node that supplies the diagnostic source anchor.
  The builder now accepts both so it can preserve existing parse output while
  still centralising source-anchor mechanics.
- Inline golden output is sensitive to AST allocation order because branch
  labels include node numbers. Added `rxcp_remap_create_assignment_node()` so
  shared assignment materialisation can allocate the assignment shell before
  targets/RHS nodes, preserving the old tree numbering.
- Some remaining direct inline constructors are intentionally not moved yet:
  receiver-copyback wrappers reuse an existing inline scope, and return
  assignments may target caller scope or inline scope depending on rewrite
  context. The shared helpers are still used inside those flows where the
  construction is neutral.

### Verification

Green stop for implementation stage 9:

- `cmake --build cmake-build-release --target rxc --parallel 4`
  - result: passed
- `RXCP_INLINE_RULE_SUMMARY=1 cmake-build-release/bin/rxc -i cmake-build-release/bin -o /tmp/remap_build_summary_probe compiler/tests/rexx_src/inline_test_composed_expr_contexts.crexx`
  - result: passed
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: passed, 3/3 tests
- `ctest --test-dir cmake-build-release -R 'inline_test_expr_negative_opt|inline_test_nested_call_expr_opt|inline_test_call_arg_expr_opt|inline_test_call_like_arg_expr_opt|inline_test_composed_expr_contexts_opt|inline_test_byvalue_arg_reuse_opt|inline_test_array_return_expr_opt|inline_test_array_multi_return_expr_opt|inline_test_array_multi_return_assign_opt|inline_test_object_writable_arg_opt|inline_test_array_expr_arg_opt|inline_test_binary_arg_return_opt' --output-on-failure`
  - result: passed, 12/12 targeted drift-repair tests
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: passed, 254/254 tests
- `git diff --check`
  - result: passed
- `rg -n '[[:blank:]]$' <touched-files>`
  - result: no trailing whitespace found

### Stage 9 Result

The remap substrate now has a first neutral builder/materialisation layer. The
inliner is still the proving client, but generated tree surgery is no longer
encoded only as inline-specific helper names. This is the bridge needed before
Level C lowering can share the same compiler-owned AST construction mechanics.

## Stage 10 - Captured-Locator Receiver Copyback

### Intent

Use the new remap/builder machinery for a class-priority extension that is
small enough to prove: local mutating method inlining when a simple whole-RHS
assignment calls through a computed variable-like receiver, for example:

```rexx
saved = items[pickIndex()].setAndReport("changed")
```

The required behaviour is:

1. evaluate the receiver locator children exactly once;
2. bind the selected receiver object into the callee's inline `§this`;
3. run the cloned mutating method body;
4. deliver the return value to the assignment target; and
5. copy the mutated receiver object back through the same captured locator.

### Framework Lessons

This was a useful framework test because the selector alone was not the hard
part. The reusable unit is a proof/materialisation pattern:

- target shape: a variable-like receiver with locator children;
- guard: local mutating method, simple statement-owned rewrite, supported
  receiver target, and no imported non-direct method receiver;
- capture: evaluate locator children into generated temps before argument
  binding;
- materialise: build `§this` from the captured locator;
- writeback: assign the mutated receiver object back through the same captured
  locator;
- boundary: leave general expression-position and receiver-producing copyback
  closed until each parent bucket has an explicit liveness/copyback proof.

For Level C remapping, this proves that the shared framework needs a small
library of named semantic obligations, not only tree selectors:

- `capture-locator-once`
- `materialise-selected-value`
- `writeback-through-captured-locator`
- `statement-owned-value-delivery`
- `expression-owned-leave-with-delivery`

Those names are more reusable than inline-specific names such as "method
receiver copyback", and they describe exactly what a reader needs to audit.

### Implementation Notes

- Extended `InlineCloneState` with one embedded receiver-copyback locator entry.
- Reused the existing by-reference actual locator capture shape for receiver
  locator children.
- Captured receiver locator children before method argument binding so later
  argument expressions cannot change which receiver slot gets written back.
- Reused the captured locator when a scoped-argument receiver capture is needed,
  avoiding double evaluation of receiver children.
- Kept general expression-position mutating method copyback closed; the new
  path is a statement-owned rewrite path.

### Named Pattern Extraction

The captured-locator proof is now a neutral remap-builder concept rather than
an inline-private helper. `RxcpRemapCapturedLocator` records the selected
locator node and the generated symbols that hold each captured child
expression. The shared builder API owns three named operations:

- `rxcp_remap_capture_locator_once()` evaluates each locator child into a
  generated temp using a caller-provided materializer callback.
- `rxcp_remap_materialise_selected_value()` rebuilds a read or write view of
  the original locator from those captured child temps.
- `rxcp_remap_writeback_through_captured_locator()` appends an assignment that
  writes a supplied value back through the captured locator.

The inliner now uses this pattern for computed mutating-method receiver
copyback and for existing nontrivial by-reference actual rematerialisation.
Inline-specific policy still decides which shapes are legal; the shared remap
layer owns the reusable proof/materialisation mechanics.

### Replay Steps

1. Add a fixture that assigns the result of a mutating method called through a
   computed receiver and verifies the stored receiver changed.
2. Add an optimized-output assertion that the mutating method call is gone.
3. Extend the inline clone state with an embedded captured-locator entry for
   receiver copyback.
4. Capture computed receiver locator children before argument binding.
5. Bind `§this` from the captured locator and append copyback through the same
   captured locator.
6. Open the assignment/statement call-site guard from "direct receiver" to
   "supported receiver" while keeping expression-position copyback direct-only.

### Verification

Green stop for implementation stage 10:

- `cmake --build cmake-build-release --target rxc rxas rxvm --parallel 4`
  - result: passed
- direct optimized fixture probe for
  `inline_test_computed_receiver_copyback.crexx`
  - result: `setAndReport()` call removed from main RXAS; runtime output
    matched `changed`, `one`, `changed`
- direct function-index receiver probe using
  `items[pickIndex()].setAndReport("changed")`
  - result: `setAndReport()` call removed; runtime output matched
- `ctest --test-dir cmake-build-release -R 'inline_test_computed_receiver_copyback' --output-on-failure`
  - result: passed, 3/3 tests
- `ctest --test-dir cmake-build-release -R 'inline_test_class_methods|inline_local_member_scalar|inline_test_member_receiver_expr|inline_test_object_writable_arg|inline_test_object_expr_arg' --output-on-failure`
  - result: passed, 17/17 tests
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: passed, 257/257 tests
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: passed, 3/3 tests
- `RXCP_INLINE_RULE_SUMMARY=1 cmake-build-release/bin/rxc -i cmake-build-release/bin -o /tmp/computed_receiver_summary_probe compiler/tests/rexx_src/inline_test_computed_receiver_copyback.crexx`
  - result: passed; summary still prints service boundaries and selector rules
- `git diff --check`
  - result: passed
- `rg -n '[[:blank:]]$' <touched-files>`
  - result: reported pre-existing whitespace in `compiler/tests/CMakeLists.txt`;
    `git diff --check` confirmed no new whitespace errors

## Stage 11 - Named Captured-Locator Patterns

### Goal

Make the captured-locator proof/materialisation pattern explicit in the shared
remap builder layer. Selectors identify candidate tree shapes, but this layer
names the semantic obligations that make a rewrite safe: evaluate locator
children once, rematerialise the selected read/write value from those captures,
and optionally write a changed value back through the same locator.

### Replay Steps

1. Add `RxcpRemapCapturedLocator` to `rxcp_remap_build.h`.
2. Add `rxcp_remap_capture_locator_once()` beside the scalar
   `rxcp_remap_capture_once()` helper.
3. Move the inline-private captured-locator rematerialisation into
   `rxcp_remap_materialise_selected_value()`.
4. Add `rxcp_remap_writeback_through_captured_locator()` for assignment
   copyback through a captured locator.
5. Change computed method receiver copyback to use the shared captured-locator
   API instead of an inline-private entry.
6. Change nontrivial by-reference actual entries to embed
   `RxcpRemapCapturedLocator`, so existing ref indexed/stem/computed/vararg
   rematerialisation also proves the shared pattern.
7. Update the Level C remapping and inlining docs with the named operations.

### Issues And Resolutions

- By-reference actual entries still need inline-specific state, notably the
  formal symbol they replace. Kept `InlineRefActualEntry` as an inline policy
  record, but replaced its private child-capture fields with
  `RxcpRemapCapturedLocator`.
- Receiver copyback needs writeback, while by-reference actuals only need
  rematerialisation. The shared API therefore separates capture,
  materialisation, and writeback instead of baking one receiver-specific flow.
- The materialiser callback remains caller-owned. Inline passes subtree cloning;
  Level C lowering can later pass a source-to-target materialiser without
  depending on inline clone state.

### Verification

Green stop for implementation stage 11:

- `cmake --build cmake-build-release --target rxc rxas rxvm --parallel 4`
  - result: passed
- `ctest --test-dir cmake-build-release -R 'inline_test_computed_receiver_copyback' --output-on-failure`
  - result: passed, 3/3 tests
- `ctest --test-dir cmake-build-release -R 'inline_test_class_methods|inline_local_member_scalar|inline_test_member_receiver_expr|inline_test_object_writable_arg|inline_test_object_expr_arg' --output-on-failure`
  - result: passed, 17/17 tests
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: passed, 257/257 tests
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: passed, 3/3 tests
- `RXCP_INLINE_RULE_SUMMARY=1 cmake-build-release/bin/rxc -i cmake-build-release/bin -o /tmp/captured_locator_summary_probe compiler/tests/rexx_src/inline_test_computed_receiver_copyback.crexx`
  - result: passed; summary still prints service boundaries and selector rules
- `git diff --check`
  - result: passed

## Stage 12 - Command-Shaped Remap Builder Split

### Goal

Make the remap builder easier to read as a future command vocabulary. The
selector table is already DSL-like enough for now; this pass splits reusable C
construction routines so later rule descriptions can talk in verbs such as
`shape-from`, `append-assignment`, `append-leave-with`, `replace-node`,
`capture-assignment`, `assembler-instr`, and `register-copy` without embedding
inline-specific helper names.

### Implemented Shape

The shared remap builder now owns:

- node semantic shape copying via `rxcp_remap_copy_node_semantics()`;
- symbol-disconnect cleanup and replacement through
  `rxcp_remap_disconnect_subtree_symbols()` and `rxcp_remap_replace_node()`;
- string constants, generic assembler instructions, and register-copy
  instructions;
- assignment completion/append and `LEAVE_WITH` append operations;
- split capture helpers:
  `rxcp_remap_build_capture_assignment()` and
  `rxcp_remap_create_captured_value_ref()`, used by scalar and locator
  capture-once commands.

Inline clients now use those shared helpers for semantic copying, assembler
and copy instruction creation, assignment/leave appends, and normal AST
replacement cleanup. Inline policy remains inline-owned: argument-binding
classification, receiver legality, return-shape decisions, and clone-map
policy are not moved in this pass.

### Remaining Candidates

- A higher-level `deliver-to-target` / `deliver-to-sink` API would make return
  handling read even more like a rule command, but it should preserve current
  assignment allocation order.
- Clone-map extraction remains deliberately deferred. It is likely useful for
  Level C and Classic Rexx, but it touches scope/symbol ownership more deeply
  than this command-split pass.
- Scoped method/factory argument capture still has inline policy interleaved
  with construction. It now uses shared append helpers, but the proof logic
  should stay with the inliner until another frontend needs the same rule.

### Replay Steps

1. Promote node semantic shape copying from inline-private/static helper to the
   remap builder API.
2. Move ordinary replacement cleanup into `rxcp_remap_replace_node()`.
3. Move assembler, string-constant, and register-copy builders into
   `rxcp_remap_build.[ch]`.
4. Add append helpers for existing assignment nodes and `LEAVE_WITH` nodes.
5. Split capture-once internals into capture-assignment and captured-value-ref
   builders, keeping the existing scalar and locator convenience APIs.
6. Refactor inline clients to call the shared helpers without changing rewrite
   policy.

### Verification

Green stop for implementation stage 12:

- `cmake --build cmake-build-release --target rxc rxas rxvm --parallel 4`
  - result: passed
- `ctest --test-dir cmake-build-release -R 'inline|Inline' --output-on-failure`
  - result: 257/257 passed
- `ctest --test-dir cmake-build-release -R '16_classes|address_inline_then|address_exit_extended' --output-on-failure`
  - result: 3/3 passed
- `RXCP_INLINE_RULE_SUMMARY=1 cmake-build-release/bin/rxc -i cmake-build-release/bin -o /tmp/remap_command_summary_probe compiler/tests/rexx_src/inline_test_computed_receiver_copyback.crexx`
  - result: passed and printed selector/service rule summaries
- Direct helper-shape searches over inline/remap source
  - result: passed; old inline-private builder names removed, direct
    replacement is limited to `rxcp_remap_replace_node()` and the intentional
    return-wrapper move case
- `git diff --check`
  - result: passed
